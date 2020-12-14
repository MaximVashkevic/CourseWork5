
#include <ntddk.h>
#include <wdf.h>
#define NDIS_WDM
#define NDIS60
#define NDIS_SUPPORT_NDIS6
#include <fwpsk.h>
#include <fwpmk.h>

#define INITGUID
#include "device.h"
// TODO
//#include "queue.h"

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_UNLOAD SnifferDriverUnload;
