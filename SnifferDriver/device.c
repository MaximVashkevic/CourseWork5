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

void SnifferEvtWdfDeviceFileCreate(
	WDFDEVICE Device,
	WDFREQUEST Request,
	WDFFILEOBJECT FileObject
)
{
	NTSTATUS status = STATUS_SUCCESS;
	P_FILE_OBJECT_CONTEXT fileObjectContext;
	WDF_IO_QUEUE_CONFIG             queueConfig;

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: creating device\n"));

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

	fileObjectContext = GetFileObjectContext(FileObject);
	// TODO stop classify, cleanup
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: file cleanup\n"));

	WdfIoQueuePurgeSynchronously(fileObjectContext->ReadQueue); // не блокировать

	WdfObjectDelete(fileObjectContext->ReadQueue);

	// TODO: блокировать?
	FreeReceiveQueue(fileObjectContext);
}

void FreeReceiveQueue(P_FILE_OBJECT_CONTEXT objectContext)
{
	UNREFERENCED_PARAMETER(objectContext);
	// TODO free queue;
}

// TODO
void ProcessReadRequest(P_FILE_OBJECT_CONTEXT objectContext)
{
	WDFREQUEST          request;
	NTSTATUS            status = STATUS_UNSUCCESSFUL;
	KLOCK_QUEUE_HANDLE	lockHandle;
	//PMDL                pMdl;
	//PUCHAR              pDst;
	//PLIST_ENTRY         pReceiveNetBufferListEntry;
	//PNET_BUFFER_LIST    pNBL;

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

		WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
		return;

		/*status = WdfRequestRetrieveInputWdmMdl(request, &pMdl);
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

		pReceiveNetBufferListEntry = objectContext->RecvNetBufListQueue.Flink;
		RemoveEntryList(pReceiveNetBufferListEntry);
		(objectContext->RecvNetBufListCount)--;*/

		//pNBL = 
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

	/******
	* 
	* TODO
	* 
	*/
		WdfRequestCompleteWithInformation(Request, status, 0);
		return;


	/*
	* 
	* 
	* */

		//TODO!!!!!
	//// TODO: не блокируют?
	//status = WdfRequestForwardToIoQueue(Request, fileObjectContext->ReadQueue);

	//if (!NT_SUCCESS(status))
	//{
	//	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Sniffer: can't forward read request\n"));
	//	WdfRequestCompleteWithInformation(Request, status, 0);
	//}

	//ProcessReadRequest(fileObjectContext);

	//return;
}

VOID ClassifyFn(IN const FWPS_INCOMING_VALUES0* inFixedValues, IN const FWPS_INCOMING_METADATA_VALUES0* inMetaValues, IN OUT VOID* layerData, IN const FWPS_FILTER0* filter, IN UINT64 flowContext, IN OUT FWPS_CLASSIFY_OUT0* classifyOut)
{
	P_FILE_OBJECT_CONTEXT fileObjectContext;
	KLOCK_QUEUE_HANDLE	lockHandle;


	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(inMetaValues);
	UNREFERENCED_PARAMETER(inFixedValues);

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: classifyFn\n"));

	PNET_BUFFER_LIST rawData;

	rawData = (PNET_BUFFER_LIST)layerData;

	fileObjectContext = GetFileObjectContext(GlobalFileObject);

	KeAcquireInStackQueuedSpinLock(&(fileObjectContext->lock), &lockHandle);



	KeReleaseInStackQueuedSpinLock(&lockHandle);

	// TODO: где вызывать?
	//ProcessReadRequest(fileObjectContext);

	classifyOut->actionType = FWP_ACTION_CONTINUE;
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