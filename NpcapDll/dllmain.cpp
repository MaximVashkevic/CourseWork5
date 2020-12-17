#include <Windows.h>
#include <objbase.h>
#include "../SnifferLib/SnifferLib.h"

#define MAX_STRING_SIZE 1024
#define SNAP_LENGTH 2000
#define TIMEOUT 1000
#define PCAP_NEXT_EX_OK 1
#define PCAP_NEXT_EX_TIMEOUT 0

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}

extern "C" HANDLE StartSniffing();

extern "C" void StopSniffing(HANDLE handle);

//int GetFrame(pcap_t* handle, struct pcap_pkthdr* pkt_header,
//	u_char** pkt_data)
//{
extern "C" int GetPacketManaged(HANDLE captureHandle, UCHAR** ppPacketData)
{
	int receivedCount = -1;

	*ppPacketData = (UCHAR*)CoTaskMemAlloc(SNAP_LENGTH);
	if (ppPacketData != NULL)
	{
		receivedCount = GetPacket(captureHandle, *ppPacketData, SNAP_LENGTH);
	}

	return receivedCount;

	/*const u_char* data;
	int result;
	do
	{
		result = pcap_next_ex(handle, &header, &data);
	} while (result == PCAP_NEXT_EX_TIMEOUT);
	if (result == PCAP_NEXT_EX_OK)
	{
		memcpy(pkt_header, header, sizeof(pcap_pkthdr));

		*pkt_data = (u_char*)CoTaskMemAlloc(header->caplen);
		if (*pkt_data != NULL)
		{
			memcpy(*pkt_data, data, header->caplen);
		}
	}*/

	//return result;
}