#include "common.h"



typedef struct _DEVICE_CONTEXT
{
	ULONG PrivateDeviceData;  // TODO

} DEVICE_CONTEXT, * PDEVICE_CONTEXT;

typedef struct
{
	UINT64 address;       // buffer address
	UINT64 length;        // buffer length
} RECEIVE_IOCTL, *PRECEIVE_IOCTL;

typedef struct
{
	UINT64* addr_len_ptr;                     // Pointer to address length.
	UINT64 addr_len;
};

// generate function for access to device context
WDF_DECLARE_CONTEXT_TYPE(DEVICE_CONTEXT)

//NTSTATUS
//SnifferDeviceCreate(
//    PWDFDEVICE_INIT DeviceInit
//);

EVT_WDF_DEVICE_FILE_CREATE SnifferEvtWdfDeviceFileCreate;
EVT_WDF_FILE_CLOSE SnifferEvtWdfFileClose;
EVT_WDF_FILE_CLEANUP SnifferEvtWdfFileCleanup;
EVT_WDF_OBJECT_CONTEXT_DESTROY SnifferEvtWdfObjectContextDestroy;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL SnifferEvtWdfIoQueueIoDeviceControl;
EVT_WDF_IO_IN_CALLER_CONTEXT SnifferEvtWdfIoInCallerContext;

#define IOCTL_SNIFFER_RECV                                                \
    CTL_CODE(FILE_DEVICE_NETWORK, 0x900, METHOD_OUT_DIRECT, FILE_READ_DATA)
