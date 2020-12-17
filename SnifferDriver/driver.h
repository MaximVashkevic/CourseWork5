#include <ntddk.h>
#include <wdf.h>
#define NDIS630

#include <fwpsk.h>
#include <fwpmk.h>

#pragma comment(lib, "fwpkclnt.lib")

#include "device.h"

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_UNLOAD SnifferDriverUnload;
void SnifferDriverUnload();
NTSTATUS
CreateDevice(
    IN WDFDRIVER Driver,
    IN PWDFDEVICE_INIT DeviceInit
);

NTSTATUS AddCallout();
