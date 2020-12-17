#include "driver.h"

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

	GlobalFileObject = FileObject;

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
	GlobalFileObject = NULL;

	fileObjectContext = GetFileObjectContext(FileObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: file cleanup\n"));

	WdfIoQueuePurgeSynchronously(fileObjectContext->ReadQueue); // не блокировать

	WdfObjectDelete(fileObjectContext->ReadQueue);

	FreeReceiveQueue(fileObjectContext);
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
}

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

	P_FILE_OBJECT_CONTEXT fileObjectContext;
	WDFFILEOBJECT fileObject;
	NTSTATUS status = STATUS_SUCCESS;

	fileObject = WdfRequestGetFileObject(Request);
	fileObjectContext = GetFileObjectContext(fileObject);

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

				if (size < MAX_PACKET_LENGTH)
				{
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
					}
				}
			}
		}
	}

	KeReleaseInStackQueuedSpinLock(&lockHandle);

	if (captured)
	{
		ProcessReadRequest(fileObjectContext);
	}
}