#define INITGUID

#include <ntddk.h>
#include <wdf.h>

#include "device.h"
// TODO
//#include "queue.h"

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_UNLOAD SnifferDriverUnload;
