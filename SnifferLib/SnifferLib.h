#pragma once

#ifndef __SNIFFERLIB_H
#define __SNIFFERLIB_H

#define SNIFFERLIBAPI __declspec(dllimport)

#else
#define SNIFFERLIBAPI __declspec(dllexport)

#endif

#include <Windows.h>
#include <guiddef.h>

// {EB8AFC40-1915-42CD-80CF-7F12E7559FD2}
static const GUID PROVIDER_GUID =
{ 0xeb8afc40, 0x1915, 0x42cd, { 0x80, 0xcf, 0x7f, 0x12, 0xe7, 0x55, 0x9f, 0xd2 } };


// {F1104EF0-AFAF-4FE0-9814-75DFCB63EF91}
static const GUID SUBLAYER_GUID =
{ 0xf1104ef0, 0xafaf, 0x4fe0, { 0x98, 0x14, 0x75, 0xdf, 0xcb, 0x63, 0xef, 0x91 } };

// {0071EA65-87A2-4AC9-9518-F5C215A2383E}
static const GUID CALLOUT_GUID =
{ 0x71ea65, 0x87a2, 0x4ac9, { 0x95, 0x18, 0xf5, 0xc2, 0x15, 0xa2, 0x38, 0x3e } };

// {9E9E900D-419E-4450-A315-5DBF6F78D08A}
static const GUID FILTER_GUID =
{ 0x9e9e900d, 0x419e, 0x4450, { 0xa3, 0x15, 0x5d, 0xbf, 0x6f, 0x78, 0xd0, 0x8a } };


extern "C" DWORD StartSniffing();
