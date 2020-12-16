#include <ntddk.h>
#include <wdf.h>

#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

UINT32 CalloutId = 0;
WDFDEVICE device = NULL;
WDFFILEOBJECT GlobalFileObject = NULL;

NTSTATUS AddCallout(GUID guid)
{
    // create callout
    PDEVICE_OBJECT deviceObject;
    FWPS_CALLOUT0 callout;
    deviceObject = WdfDeviceWdmGetDeviceObject(device);
    RtlZeroMemory(&callout, sizeof(callout));
    callout.calloutKey = guid;
    callout.classifyFn = ClassifyFn;
    callout.notifyFn = NotifyFn;
    callout.flowDeleteFn = NULL;

    NTSTATUS result = FwpsCalloutRegister0(
        deviceObject,
        &callout,
        NULL);

    return result;
}

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT     DriverObject,
    _In_ PUNICODE_STRING    RegistryPath
)
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG config;
    WDFDRIVER driver;
    PWDFDEVICE_INIT deviceInit;

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: DriverEntry\n"));

    // configure for nonPnP
    WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
    config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
    config.EvtDriverUnload = SnifferDriverUnload;

    status = WdfDriverCreate(DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES,
        &config,
        &driver);

    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't create driver\n"));
        goto Exit;
    }

    // Create control device
    deviceInit = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_SYS_ALL_ADM_ALL);
    if (deviceInit == NULL) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Sniffer: can't allocate init structure\n"));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }
    // Add NonPnpShutdown??? (1221) // нет в ndisprot

    status = CreateDevice(driver, deviceInit);

Exit:

    return status;
}

NTSTATUS
CreateDevice(
    IN WDFDRIVER Driver,
    IN PWDFDEVICE_INIT DeviceInit
)
{
    NTSTATUS status = STATUS_SUCCESS;

    WDF_IO_TYPE_CONFIG ioConfig;
    DECLARE_CONST_UNICODE_STRING(deviceName,
    L"\\Device\\" SNIFFER_DEVICE_NAME);
    DECLARE_CONST_UNICODE_STRING(dosDeviceName,
    L"\\DosDevices\\" SNIFFER_DEVICE_NAME);
    //WDF_OBJECT_ATTRIBUTES  attributes;
    WDF_FILEOBJECT_CONFIG fileObjectConfig;
    WDF_OBJECT_ATTRIBUTES objectAttributes;
    WDF_IO_QUEUE_CONFIG queueConfig;
    WDFQUEUE queue;

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: create device\n"));

    UNREFERENCED_PARAMETER(Driver);

    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_NETWORK);
    WdfDeviceInitSetExclusive(DeviceInit, TRUE);
    WDF_IO_TYPE_CONFIG_INIT(&ioConfig);
    ioConfig.ReadWriteIoType = WdfDeviceIoDirect;
    // TODO: dont need it?
    //ioConfig.DeviceControlIoType = WdfDeviceIoDirect;
    WdfDeviceInitSetIoTypeEx(DeviceInit, &ioConfig);

    status = WdfDeviceInitAssignName(DeviceInit, &deviceName);
    if (!NT_SUCCESS(status)) {

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't assign device name\n"));
        WdfDeviceInitFree(DeviceInit);
        goto Cleanup;
    }

    // Initialize driver's framework file objects configuration
    WDF_FILEOBJECT_CONFIG_INIT(
        &fileObjectConfig, 
        SnifferEvtWdfDeviceFileCreate, 
        SnifferEvtWdfFileClose, 
        SnifferEvtWdfFileCleanup);
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&objectAttributes, FILE_OBJECT_CONTEXT);
    //objectAttributes.ExecutionLevel = WdfExecutionLevelPassive;
    //objectAttributes.SynchronizationScope = WdfSynchronizationScopeNone;
    //objectAttributes.EvtDestroyCallback = SnifferEvtWdfObjectContextDestroy;
    WdfDeviceInitSetFileObjectConfig(DeviceInit, &fileObjectConfig, &objectAttributes);
    //WdfDeviceInitSetIoInCallerContextCallback(DeviceInit,
    //    SnifferEvtWdfIoInCallerContext);
    WDF_OBJECT_ATTRIBUTES_INIT(&objectAttributes);
    status = WdfDeviceCreate(&DeviceInit, &objectAttributes, &device);
    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't create device\n"));
        WdfDeviceInitFree(DeviceInit);
        goto Cleanup;
    }

    // Create io queue
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig,
        WdfIoQueueDispatchParallel);
    queueConfig.EvtIoRead = SnifferEvtWdfIoQueueIoRead;
    queueConfig.EvtIoWrite = NULL;
    queueConfig.EvtIoDeviceControl = NULL;
    //queueConfig.EvtIoDeviceControl = SnifferEvtWdfIoQueueIoDeviceControl;
    //WDF_OBJECT_ATTRIBUTES_INIT(&objectAttributes);
    //objectAttributes.ExecutionLevel = WdfExecutionLevelPassive;
    //objectAttributes.SynchronizationScope = WdfSynchronizationScopeNone;
    //status = WdfIoQueueCreate(device, &queueConfig, &objectAttributes, &queue);
    status = WdfIoQueueCreate(device, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, &queue);
    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't create queue\n"));
        goto Cleanup;
    }

    // Create symlink
    status = WdfDeviceCreateSymbolicLink(device, &dosDeviceName);
    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Sniffer: can't create symlink\n"));
        goto Cleanup;
    }
    WdfControlFinishInitializing(device);

    status = AddCallout(MAC_OUT_CALLOUT_GUID);
    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Sniffer: can't create symlink\n"));
        goto Cleanup;
    }
    status = AddCallout(MAC_OUT_CALLOUT_GUID);
    status = AddCallout(MAC_IN_CALLOUT_GUID);
    //AddCallout(MAC_OUT_CALLOUT_GUID);
    //AddCallout(MAC_OUT_CALLOUT_GUID);

    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Sniffer: can't register Callout\n"));
        goto Cleanup;
    }

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: finished create device (+queue, +callout)\n"));



Cleanup:
    if (DeviceInit != NULL)
    {
        WdfDeviceInitFree(DeviceInit);
    }

    return status;
}



void SnifferDriverUnload(
    WDFDRIVER Driver
)
{
    UNREFERENCED_PARAMETER(Driver);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: unload driver\n"));

    FwpsCalloutUnregisterById0(
        CalloutId
    );

    if (device != NULL)
    {
        WdfObjectDelete(device);
    }
}


NTSTATUS NotifyFn(IN FWPS_CALLOUT_NOTIFY_TYPE notifyType, IN const GUID* filterKey, IN const FWPS_FILTER0* filter)
{
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: notify\n"));

    UNREFERENCED_PARAMETER(notifyType);
    UNREFERENCED_PARAMETER(filterKey);
    UNREFERENCED_PARAMETER(filter);
    return STATUS_SUCCESS;
}
