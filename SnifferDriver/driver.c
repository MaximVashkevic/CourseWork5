#include <ntddk.h>
#include <wdf.h>

#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

UINT32 CalloutId;

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
    WDF_IO_TYPE_CONFIG ioConfig;
    DECLARE_CONST_UNICODE_STRING(deviceName,
    L"\\Device\\" SNIFFER_DEVICE_NAME);
    DECLARE_CONST_UNICODE_STRING(dosDeviceName,
    L"\\DosDevices\\" SNIFFER_DEVICE_NAME);
    WDF_OBJECT_ATTRIBUTES  attributes;
    WDF_FILEOBJECT_CONFIG fileObjectConfig;
    WDF_OBJECT_ATTRIBUTES objectAttributes;
    WDFDEVICE device;
    WDF_IO_QUEUE_CONFIG queueConfig;
    WDFQUEUE queue;

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: DriverEntry\n"));

    // configure for nonPnP
    WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
    config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
    config.EvtDriverUnload = EvtWdfDriverUnload;

    status = WdfDriverCreate(DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES,
        &config,
        &driver);

    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't create driver\n"));
        goto Cleanup;
    }

    // Create control device
    deviceInit = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_SYS_ALL_ADM_ALL);
    if (deviceInit == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    // Add NonPnpShutdown??? (1221) // нет в ndisprot

    status = CreateDevice(driver, deviceInit);

    status = InitializeWFP();

Cleanup:

    return status;
}

NTSTATUS
InitializeWFP(PDEVICE_OBJECT deviceObject)
{
    FWPS_CALLOUT   calloutInitData;
    NTSTATUS status = FwpsCalloutRegister0(deviceObject, &calloutInitData )
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
    WDF_OBJECT_ATTRIBUTES  attributes;
    WDF_FILEOBJECT_CONFIG fileObjectConfig;
    WDF_OBJECT_ATTRIBUTES objectAttributes;
    WDFDEVICE device;
    WDF_IO_QUEUE_CONFIG queueConfig;
    WDFQUEUE queue;
    PDEVICE_OBJECT deviceObject;
    FWPS_CALLOUT0 callout;

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
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't create symlink\n"));
        goto Cleanup;
    }
    WdfControlFinishInitializing(device);

    // create callout
    deviceObject = WdfDeviceWdmGetDeviceObject(device);
    RtlZeroMemory(&callout, sizeof(callout));
    callout.calloutKey = CALLOUT_GUID;
    callout.classifyFn = ClassifyFn;
    callout.notifyFn = NotifyFn;
    callout.flowDeleteFn = NULL;

    status = FwpsCalloutRegister0(
        deviceObject,
        &callout,
        &CalloutId);

    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't create symlink\n"));
        goto Cleanup;
    }


Cleanup:
    if (DeviceInit != NULL)
    {
        WdfDeviceInitFree(DeviceInit);
    }

    return status;
}

void EvtWdfDriverUnload(
    WDFDRIVER Driver
)
{
}

VOID ClassifyFn(IN const FWPS_INCOMING_VALUES0* inFixedValues, IN const FWPS_INCOMING_METADATA_VALUES0* inMetaValues, IN OUT VOID* layerData, IN const FWPS_FILTER0* filter, IN UINT64 flowContext, IN OUT FWPS_CLASSIFY_OUT0* classifyOut)
{
    return VOID();
}

NTSTATUS NotifyFn(IN FWPS_CALLOUT_NOTIFY_TYPE notifyType, IN const GUID* filterKey, IN const FWPS_FILTER0* filter)
{
    return STATUS_SUCCESS;
}
