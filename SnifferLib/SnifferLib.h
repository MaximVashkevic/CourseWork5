#pragma once

#include <Windows.h>
#include <guiddef.h>
#include "../SnifferDriver/common.h"

// {EB8AFC40-1915-42CD-80CF-7F12E7559FD2}
static const GUID PROVIDER_GUID =
{ 0xeb8afc40, 0x1915, 0x42cd, { 0x80, 0xcf, 0x7f, 0x12, 0xe7, 0x55, 0x9f, 0xd2 } };


// {F1104EF0-AFAF-4FE0-9814-75DFCB63EF91}
static const GUID SUBLAYER_GUID =
{ 0xf1104ef0, 0xafaf, 0x4fe0, { 0x98, 0x14, 0x75, 0xdf, 0xcb, 0x63, 0xef, 0x91 } };

// {9E9E900D-419E-4450-A315-5DBF6F78D08A}
static const GUID MAC_OUT_FILTER_GUID =
{ 0x9e9e900d, 0x419e, 0x4450, { 0xa3, 0x15, 0x5d, 0xbf, 0x6f, 0x78, 0xd0, 0x8a } };

// {5E476338-CCA8-4FD2-94B3-6C152D0D99F6}
static const GUID MAC_IN_FILTER_GUID =
{ 0x5e476338, 0xcca8, 0x4fd2, { 0x94, 0xb3, 0x6c, 0x15, 0x2d, 0xd, 0x99, 0xf6 } };

// {8D9F6604-12C8-40DE-B624-4F5957838A60}
static const GUID IP_FILTER_GUID =
{ 0x8d9f6604, 0x12c8, 0x40de, { 0xb6, 0x24, 0x4f, 0x59, 0x57, 0x83, 0x8a, 0x60 } };


#define SNIFFER_DEVICE_NAME L"Sniffer"
#define DEVICE_NAME  L"\\\\.\\"  SNIFFER_DEVICE_NAME

extern "C" HANDLE StartSniffing();

extern "C" ULONG GetPacket(HANDLE handle, PUCHAR buffer, ULONG bufferLength);

extern "C" void StopSniffing(HANDLE handle);

