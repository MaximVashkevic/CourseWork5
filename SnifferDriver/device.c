#include "driver.h"

// specify code sections for functions

//#ifdef ALLOC_PRAGMA
//#pragma alloc_text (PAGE, SnifferDeviceCreate)
//#endif
//
//NTSTATUS SnifferDeviceCreate(PWDFDEVICE_INIT DeviceInit)
//{
//    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
//    P_FILE_OBJECT_CONTEXT fileObjectContext;
//    WDF_PNPPOWER_EVENT_CALLBACKS    pnpPowerCallbacks;
//    WDFDEVICE device;
//    NTSTATUS status;
//
//    PAGED_CODE();
//}

void SnifferFree(PVOID pointer);


void SnifferEvtWdfDeviceFileCreate(
	WDFDEVICE Device,
	WDFREQUEST Request,
	WDFFILEOBJECT FileObject
)
{
	NTSTATUS status = STATUS_SUCCESS;
	P_FILE_OBJECT_CONTEXT fileObjectContext;
	WDF_IO_QUEUE_CONFIG             queueConfig;

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: opening file\n"));

	fileObjectContext = GetFileObjectContext(FileObject);

	KeInitializeSpinLock(&(fileObjectContext->lock));

	WDF_IO_QUEUE_CONFIG_INIT(
		&queueConfig,
		WdfIoQueueDispatchManual
	);
	status = WdfIoQueueCreate(
		Device,
		&queueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&fileObjectContext->ReadQueue
	);
	if (!NT_SUCCESS(status))
	{
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't create read queue device\n"));
		goto Exit;
	}

	InitializeListHead(&fileObjectContext->RecvNetBufListQueue);

	// TODO: reference?
	GlobalFileObject = FileObject;

	// 
Exit:
	WdfRequestComplete(Request, status);
}

void SnifferEvtWdfFileClose(
	WDFFILEOBJECT FileObject
)
{
	UNREFERENCED_PARAMETER(FileObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: closing device\n"));

}

void SnifferEvtWdfFileCleanup(
	WDFFILEOBJECT FileObject
)
{
	P_FILE_OBJECT_CONTEXT fileObjectContext;
	KLOCK_QUEUE_HANDLE lockHandle;
	GlobalFileObject = NULL;
	GUID emptyGuid = { 0 };

	fileObjectContext = GetFileObjectContext(FileObject);
	// TODO stop classify, cleanup
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: file cleanup\n"));

	WdfIoQueuePurgeSynchronously(fileObjectContext->ReadQueue); // не блокировать

	WdfObjectDelete(fileObjectContext->ReadQueue);

	// TODO: блокировать?
	FreeReceiveQueue(fileObjectContext);

	KeAcquireInStackQueuedSpinLock(&fileObjectContext->lock, &lockHandle);

	HANDLE engineHandle = NULL;
	DWORD result = 0;
	result = FwpmEngineOpen0(
		NULL,
		RPC_C_AUTHN_WINNT,
		NULL,
		NULL,
		&engineHandle
	);
	if (result == STATUS_SUCCESS)
	{
		for (int i = 0; i < MAX_FILTER_COUNT; ++i)
		{
			if (!IsEqualGUID(&fileObjectContext->filters[i], &emptyGuid))
			{
				result = FwpmFilterDeleteByKey(engineHandle, &fileObjectContext->filters[i]);
			}
		}
	}

	FwpmEngineClose(engineHandle);
	KeReleaseInStackQueuedSpinLock(&lockHandle);
}

void FreeReceiveQueue(P_FILE_OBJECT_CONTEXT objectContext)
{
	KLOCK_QUEUE_HANDLE lockHandle;
	PLIST_ENTRY entry;

	KeAcquireInStackQueuedSpinLock(&objectContext->lock, &lockHandle);

	while (!IsListEmpty(&objectContext->RecvNetBufListQueue))
	{
		entry = RemoveHeadList(&objectContext->RecvNetBufListQueue);

		PVOID pPacket = entry;

		SnifferFree(pPacket);

		objectContext->RecvNetBufListCount--;
	}

	KeReleaseInStackQueuedSpinLock(&lockHandle);

}

// TODO
void ProcessReadRequest(P_FILE_OBJECT_CONTEXT objectContext)
{
	WDFREQUEST          request;
	NTSTATUS            status = STATUS_UNSUCCESSFUL;
	KLOCK_QUEUE_HANDLE	lockHandle;
	PMDL                pMdl;
	PUCHAR              pDst;
	PLIST_ENTRY         pReceiveNetBufferListEntry;
	ULONG				bufferLength;
	ULONG				copiedLength = 0;
	PPACKET				pPacket;
	PVOID				pPacketData;

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: processing read\n"));

	KeAcquireInStackQueuedSpinLock(&(objectContext->lock), &lockHandle);

	while (IsListEmpty(&objectContext->RecvNetBufListQueue) == FALSE)
	{
		status = WdfIoQueueRetrieveNextRequest(
			objectContext->ReadQueue,
			&request
		);
		if (!NT_SUCCESS(status))
		{
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Sniffer: processing read: no entries in queue\n"));
			break;
		}

		/*WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
		return;*/

		status = WdfRequestRetrieveOutputWdmMdl(request, &pMdl);
		if (!NT_SUCCESS(status))
		{
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Sniffer: processing read: can't get mdl\n"));
			break;
		}

		pDst = MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority | MdlMappingNoExecute);
		if (pDst == NULL)
		{
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Sniffer: processing read: can't map mdl\n"));
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		bufferLength = MmGetMdlByteCount(pMdl);

		pReceiveNetBufferListEntry = RemoveHeadList(&objectContext->RecvNetBufListQueue);
		//RemoveEntryList(pReceiveNetBufferListEntry);
		(objectContext->RecvNetBufListCount)--;

		pPacket = (PPACKET)pReceiveNetBufferListEntry;
		pPacketData = &(pPacket->data);

		if (bufferLength < pPacket->length)
		{
			status = STATUS_BUFFER_TOO_SMALL;
			InsertHeadList(&objectContext->RecvNetBufListQueue, pReceiveNetBufferListEntry);
			(objectContext->RecvNetBufListCount)++;
		}
		else
		{
			RtlCopyMemory(pDst, pPacketData, pPacket->length);
			copiedLength = pPacket->length;
			status = STATUS_SUCCESS;
			SnifferFree(pReceiveNetBufferListEntry);
		}


		WdfRequestCompleteWithInformation(request, status, copiedLength);

	}

	KeReleaseInStackQueuedSpinLock(&lockHandle);

	// TODO: replace

}

// remove?
void SnifferEvtWdfObjectContextDestroy(
	WDFOBJECT Object
)
{
	UNREFERENCED_PARAMETER(Object);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: object destroy\n"));
}

void SnifferEvtWdfIoQueueIoRead(
	WDFQUEUE Queue,
	WDFREQUEST Request,
	size_t Length
)
{
	UNREFERENCED_PARAMETER(Length);
	UNREFERENCED_PARAMETER(Queue);

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: ioQueueIoRead (forwarding to readQueue)\n"));

	// forward request to deviceObject queue
	P_FILE_OBJECT_CONTEXT fileObjectContext;
	WDFFILEOBJECT fileObject;
	NTSTATUS status = STATUS_SUCCESS;


	fileObject = WdfRequestGetFileObject(Request);
	fileObjectContext = GetFileObjectContext(fileObject);


	//TODO!!!!!
	// TODO: не блокируют?
	status = WdfRequestForwardToIoQueue(Request, fileObjectContext->ReadQueue);

	if (!NT_SUCCESS(status))
	{
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Sniffer: can't forward read request\n"));
		WdfRequestCompleteWithInformation(Request, status, 0);
	}

	ProcessReadRequest(fileObjectContext);

	return;
}

PVOID SnifferNonPagedMalloc(SIZE_T size)
{
	if (size == 0)
	{
		return NULL;
	}
	return ExAllocatePoolWithTag(NonPagedPool, size, SNIFFER_TAG);
}

void SnifferFree(PVOID pointer)
{
	if (pointer != NULL)
	{
		ExFreePoolWithTag(pointer, SNIFFER_TAG);
	}
}

BOOL SnifferCopyBuffer(PNET_BUFFER pNB, PVOID data, UINT size)
{
	PVOID ptr;

	ptr = NdisGetDataBuffer(pNB, size, NULL, 1, 0);
	if (ptr != NULL) // data is contiguous
	{
		RtlCopyMemory(data, ptr, size);
	}
	else
	{
		ptr = NdisGetDataBuffer(pNB, size, data, 1, 0);
		if (ptr == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

VOID ClassifyFn(IN const FWPS_INCOMING_VALUES0* inFixedValues, IN const FWPS_INCOMING_METADATA_VALUES0* inMetaValues, IN OUT VOID* layerData, IN const FWPS_FILTER0* filter, IN UINT64 flowContext, IN OUT FWPS_CLASSIFY_OUT0* classifyOut)
{
	P_FILE_OBJECT_CONTEXT fileObjectContext;
	KLOCK_QUEUE_HANDLE	lockHandle;
	PNET_BUFFER_LIST rawData;
	BOOL captured = FALSE;

	UCHAR buffer[14];
	UCHAR* header;

	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(inMetaValues);
	UNREFERENCED_PARAMETER(inFixedValues);

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: classifyFn\n"));

	classifyOut->actionType = FWP_ACTION_CONTINUE;

	if (GlobalFileObject == NULL)
	{
		return;
	}

	fileObjectContext = GetFileObjectContext(GlobalFileObject);

	KeAcquireInStackQueuedSpinLock(&(fileObjectContext->lock), &lockHandle);

	if (fileObjectContext->RecvNetBufListCount <= MAX_PACKET_QUEUE_LENGTH)
	{
		rawData = (PNET_BUFFER_LIST)layerData;

		for (PNET_BUFFER_LIST pNBL = rawData; pNBL; pNBL = NET_BUFFER_LIST_NEXT_NBL(pNBL))
		{
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "-----------NBL:\n"));

			for (PNET_BUFFER pNB = NET_BUFFER_LIST_FIRST_NB(pNBL); pNB; pNB = NET_BUFFER_NEXT_NB(pNB))
			{

				ULONG size = NET_BUFFER_DATA_LENGTH(pNB);
				header = NdisGetDataBuffer(pNB, sizeof(buffer), buffer, 1, 0);
				if (!header)
				{
					continue;
				}

				KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL,
					"Size: %10i dest MAC address: %02x-%02x-%02x-%02x-%02x-%02x\n src MAC address: %02x-%02x-%02x-%02x-%02x-%02x\n",
					size,
					header[0], header[1], header[2], header[3], header[4], header[5], header[6], header[7], header[8], header[9], header[10], header[11]));

				if (size < MAX_PACKET_LENGTH)
				{
					// TODO: check - from size
					PVOID pPacket = SnifferNonPagedMalloc(sizeof(PACKET) - sizeof(SIZE_T) + size);

					((PPACKET)pPacket)->length = size;
					PVOID pPacketData = &(((PPACKET)pPacket)->data);
					if (SnifferCopyBuffer(pNB, pPacketData, size))
					{
						PLIST_ENTRY pEntry = &(((PPACKET)pPacket)->entry);
						InsertTailList(
							&(fileObjectContext->RecvNetBufListQueue),
							pEntry);
						(fileObjectContext->RecvNetBufListCount)++;
						captured = TRUE;
						// enqueue
					}
				}
			}
		}
	}

	// TODO: replace?
	KeReleaseInStackQueuedSpinLock(&lockHandle);

	if (captured)
	{
		// TODO: где вызывать?
		ProcessReadRequest(fileObjectContext);
	}

}

//void SnifferEvtWdfIoQueueIoDeviceControl(
//	WDFQUEUE Queue,
//	WDFREQUEST Request,
//	size_t OutputBufferLength,
//	size_t InputBufferLength,
//	ULONG IoControlCode
//)
//{
//	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: device control\n"));
//}

// TODO: remove?
//void SnifferEvtWdfIoInCallerContext(
//	WDFDEVICE Device,
//	WDFREQUEST Request
//)
//{
//	PCHAR inbuf;
//	size_t inbuflen;
//	WDF_REQUEST_PARAMETERS params;
//	WDFMEMORY memobj;
//	WDF_OBJECT_ATTRIBUTES attributes;
//	NTSTATUS status;
//
//	WDF_REQUEST_PARAMETERS_INIT(&params);
//	WdfRequestGetParameters(Request, &params);
//
//	if (params.Type != WdfRequestTypeDeviceControl)
//	{
//		status = WdfDeviceEnqueueRequest(Device, Request);
//		goto Cleanup;
//	}
//
//	status = WdfRequestRetrieveInputBuffer(Request, 0, &inbuf, &inbuflen);
//	if (!NT_SUCCESS(status))
//	{
//		goto Cleanup;
//	}
//
//	if (inbuflen < sizeof(RECEIVE_IOCTL))
//	{
//		goto Cleanup;
//	}
//
//	status = WdfDeviceEnqueueRequest(Device, Request);
//
//Cleanup:
//
//	if (!NT_SUCCESS(status))
//	{
//		WdfRequestComplete(Request, status);
//	}
//}