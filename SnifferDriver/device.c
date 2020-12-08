#include "driver.h"

// specify code sections for functions

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, SnifferDeviceCreate)
#endif

NTSTATUS SnifferDeviceCreate(PWDFDEVICE_INIT DeviceInit)
{
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    PDEVICE_CONTEXT deviceContext;
    WDF_PNPPOWER_EVENT_CALLBACKS    pnpPowerCallbacks;
    WDFDEVICE device;
    NTSTATUS status;

    PAGED_CODE();
}

void SnifferEvtWdfDeviceFileCreate(
    WDFDEVICE Device,
    WDFREQUEST Request,
    WDFFILEOBJECT FileObject
)
{
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: creating device\n"));
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