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




    // TODO start classify
Exit:
    WdfRequestComplete(Request, status);
}

void SnifferEvtWdfFileClose(
    WDFFILEOBJECT FileObject
)
{
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: closing device\n"));

}

void SnifferEvtWdfFileCleanup(
    WDFFILEOBJECT FileObject
)
{
    P_FILE_OBJECT_CONTEXT fileObjectContext;

    // TODO stop classify, cleanup
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: file cleanup\n"));

    WdfIoQueuePurgeSynchronously(fileObjectContext->ReadQueue);

    WdfObjectDelete(fileObjectContext->ReadQueue);

    FreeReceiveQueue(fileObjectContext);
}

void FreeReceiveQueue(P_FILE_OBJECT_CONTEXT objectContext)
{
    // TODO free queue;
}

// TODO
void ProcessReadRequest(P_FILE_OBJECT_CONTEXT objectContext)
{
    WDFREQUEST          request;
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PMDL                pMdl;
    PUCHAR              pDst;
    PLIST_ENTRY         pReceiveNetBufferListEntry;
    PNET_BUFFER_LIST    pNBL;

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: processing read\n"));

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

        status = WdfRequestRetrieveInputWdmMdl(request, &pMdl);
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
        (objectContext->RecvNetBufListCount)--;

        //pNBL = 
    }
}

// remove?
void SnifferEvtWdfObjectContextDestroy(
    WDFOBJECT Object
)
{
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: object destroy\n"));
}

void SnifferEvtWdfIoQueueIoRead(
    WDFQUEUE Queue,
    WDFREQUEST Request,
    size_t Length
)
{
    // forward request to deviceObject queue
    P_FILE_OBJECT_CONTEXT fileObjectContext;
    WDFFILEOBJECT fileObject;
    NTSTATUS status;


    fileObject = WdfRequestGetFileObject(Request);
    fileObjectContext = GetFileObjectContext(fileObject);

    status = WdfRequestForwardToIoQueue(Request, fileObjectContext->ReadQueue);

    if (!NT_SUCCESS(status))
    {
        WdfRequestCompleteWithInformation(Request, status, 0);
    }

    return;
}

void SnifferEvtWdfIoQueueIoDeviceControl(
    WDFQUEUE Queue,
    WDFREQUEST Request,
    size_t OutputBufferLength,
    size_t InputBufferLength,
    ULONG IoControlCode
)
{


    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: device control\n"));
}

void SnifferEvtWdfIoInCallerContext(
    WDFDEVICE Device,
    WDFREQUEST Request
)
{
    PCHAR inbuf;
    size_t inbuflen;
    WDF_REQUEST_PARAMETERS params;
    WDFMEMORY memobj;
    WDF_OBJECT_ATTRIBUTES attributes;
    NTSTATUS status;

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Type != WdfRequestTypeDeviceControl)
    {
        status = WdfDeviceEnqueueRequest(Device, Request);
        goto Cleanup;
    }

    status = WdfRequestRetrieveInputBuffer(Request, 0, &inbuf, &inbuflen);
    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

    if (inbuflen < sizeof(RECEIVE_IOCTL))
    {
        goto Cleanup;
    }

    status = WdfDeviceEnqueueRequest(Device, Request);

 Cleanup:

    if (!NT_SUCCESS(status))
    {
        WdfRequestComplete(Request, status);
    }
}