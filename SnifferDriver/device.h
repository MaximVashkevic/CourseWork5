#include "common.h"



typedef struct _DEVICE_CONTEXT
{
    ULONG PrivateDeviceData;  // TODO

} DEVICE_CONTEXT, * PDEVICE_CONTEXT;

// generate function for access to device context
WDF_DECLARE_CONTEXT_TYPE(DEVICE_CONTEXT)

NTSTATUS
SnifferDeviceCreate(
    PWDFDEVICE_INIT DeviceInit
);

EVT_WDF_DEVICE_FILE_CREATE SnifferEvtWdfDeviceFileCreate;
EVT_WDF_FILE_CLOSE SnifferEvtWdfFileClose;
EVT_WDF_FILE_CLEANUP SnifferEvtWdfFileCleanup;
EVT_WDF_OBJECT_CONTEXT_DESTROY SnifferEvtWdfObjectContextDestroy;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL SnifferEvtWdfIoQueueIoDeviceControl;
