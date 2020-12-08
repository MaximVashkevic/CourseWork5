#include <ntddk.h>
#include <wdf.h>

#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, EchoEvtDeviceAdd)
#endif

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
    L"\\??\\" SNIFFER_DEVICE_NAME);
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
    //config.EvtDriverUnload = unload;

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

    WdfDeviceInitSetDeviceType(deviceInit, FILE_DEVICE_NETWORK);

    WDF_IO_TYPE_CONFIG_INIT(&ioConfig);
    ioConfig.ReadWriteIoType = WdfDeviceIoDirect;
    // TODO: dont need it?
    //ioConfig.DeviceControlIoType = WdfDeviceIoDirect;
    WdfDeviceInitSetIoTypeEx(deviceInit, &ioConfig);

    status = WdfDeviceInitAssignName(deviceInit, &deviceName);
    if (!NT_SUCCESS(status)) {

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't assign device name\n"));
        WdfDeviceInitFree(deviceInit);
        goto Cleanup;
    }

    // Initialize driver's framework file objects configuration
    WDF_FILEOBJECT_CONFIG_INIT(&fileObjectConfig, SnifferEvtWdfDeviceFileCreate, SnifferEvtWdfFileClose, SnifferEvtWdfFileCleanup);
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&objectAttributes, DEVICE_CONTEXT);
    objectAttributes.ExecutionLevel = WdfExecutionLevelPassive;
    objectAttributes.SynchronizationScope = WdfSynchronizationScopeNone;
    objectAttributes.EvtDestroyCallback = SnifferEvtWdfObjectContextDestroy;
    WdfDeviceInitSetFileObjectConfig(deviceInit, &fileObjectConfig, &objectAttributes);
    // TODO 1042

    WDF_OBJECT_ATTRIBUTES_INIT(&objectAttributes);
    status = WdfDeviceCreate(&deviceInit, &objectAttributes, &device);
    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Sniffer: can't create device\n"));
        WdfDeviceInitFree(deviceInit);
        goto Cleanup;
    }

    // Create io queue
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig,
        WdfIoQueueDispatchParallel);
    queueConfig.EvtIoRead = NULL;
    queueConfig.EvtIoWrite = NULL;
    queueConfig.EvtIoDeviceControl = SnifferEvtWdfIoQueueIoDeviceControl;
    WDF_OBJECT_ATTRIBUTES_INIT(&objectAttributes);
    objectAttributes.ExecutionLevel = WdfExecutionLevelPassive;
    objectAttributes.SynchronizationScope = WdfSynchronizationScopeNone;
    status = WdfIoQueueCreate(device, &queueConfig, &objectAttributes, &queue);
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

Cleanup:
    if (!NT_SUCCESS(status))
    {
        // TODO
    }

    return status;
}

