
#include <ntddk.h>
#include <wdf.h>
#define NDIS_WDM
#define NDIS60
#define NDIS_SUPPORT_NDIS6
#include <fwpsk.h>
#include <fwpmk.h>

#include "device.h"
// TODO
//#include "queue.h"

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_UNLOAD SnifferDriverUnload;

// callout functions' prototypes
VOID NTAPI
ClassifyFn(
    IN const FWPS_INCOMING_VALUES0* inFixedValues,
    IN const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    IN OUT VOID* layerData,
    IN const FWPS_FILTER0* filter,
    IN UINT64  flowContext,
    IN OUT FWPS_CLASSIFY_OUT0* classifyOut
);

NTSTATUS NTAPI
NotifyFn(
    IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    IN const GUID* filterKey,
    IN const FWPS_FILTER0* filter
);