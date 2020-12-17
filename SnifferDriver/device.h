#include "common.h"

#define MAX_PACKET_LENGTH (40 + 0xFFFF)
#define MAX_PACKET_QUEUE_LENGTH 10
#define MAX_FILTER_COUNT 2

#define SNIFFER_TAG 'finS'
#define FILTER_CONTEXT_TAG 'tliF'

typedef struct FILTER_CONTEXT_ {
	int guidId;
} FILTER_CONTEXT, * PFILTER_CONTEXT;

typedef struct _FILE_OBJECT_CONTEXT
{
	WDFQUEUE                ReadQueue;
	ULONG                   PendedReadCount;
	LIST_ENTRY              RecvNetBufListQueue;
	ULONG                   RecvNetBufListCount;
	KSPIN_LOCK              lock;

	GUID					filters[2];
	int						filterCount;
} FILE_OBJECT_CONTEXT, * P_FILE_OBJECT_CONTEXT;


// generate function for access to device context
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FILE_OBJECT_CONTEXT, GetFileObjectContext)

// TODO: remove?
typedef struct
{
	UINT64 address;       // buffer address
	UINT64 length;        // buffer length
} RECEIVE_IOCTL, * PRECEIVE_IOCTL;

typedef struct _PACKET
{
	LIST_ENTRY entry;
	UINT length;
	SIZE_T data;

} PACKET, *PPACKET;

//NTSTATUS
//SnifferDeviceCreate(
//    PWDFDEVICE_INIT DeviceInit
//);

EVT_WDF_DEVICE_FILE_CREATE SnifferEvtWdfDeviceFileCreate;
EVT_WDF_FILE_CLOSE SnifferEvtWdfFileClose;
EVT_WDF_FILE_CLEANUP SnifferEvtWdfFileCleanup;
EVT_WDF_OBJECT_CONTEXT_DESTROY SnifferEvtWdfObjectContextDestroy;
EVT_WDF_IO_QUEUE_IO_READ SnifferEvtWdfIoQueueIoRead;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL SnifferEvtWdfIoQueueIoDeviceControl;
EVT_WDF_IO_IN_CALLER_CONTEXT SnifferEvtWdfIoInCallerContext;

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
	IN FWPS_CALLOUT_NOTIFY_TYPE  notifyType,
	IN const GUID* filterKey,
	IN const FWPS_FILTER0* filter
);

void FreeReceiveQueue(P_FILE_OBJECT_CONTEXT objectContext);

extern WDFFILEOBJECT GlobalFileObject;

// TODO: remove?
#define IOCTL_SNIFFER_RECV                                                \
    CTL_CODE(FILE_DEVICE_NETWORK, 0x900, METHOD_OUT_DIRECT, FILE_READ_DATA)
