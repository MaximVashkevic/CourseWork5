#include "driver.h"

// specify code sections for functions

//#ifdef ALLOC_PRAGMA
//#pragma alloc_text (PAGE, SnifferDeviceCreate)
//#endif
//
//NTSTATUS SnifferDeviceCreate(PWDFDEVICE_INIT DeviceInit)
//{
//    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
//    PDEVICE_CONTEXT deviceContext;
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

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: creating device\n"));

    // TODO start classify

    WdfRequestComplete(Request, status);
}

void SnifferEvtWdfFileClose(
    WDFFILEOBJECT FileObject
)
{
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: closing device\n"));
    // TODO stop classify, cleanup

}

void SnifferEvtWdfFileCleanup(
    WDFFILEOBJECT FileObject
)
{
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: file cleanup\n"));
}

void SnifferEvtWdfObjectContextDestroy(
    WDFOBJECT Object
)
{
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: object destroy\n"));
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