// SnifferLib.cpp : Defines the functions for the static library.
//

#include "framework.h"
#include "SnifferLib.h"
#include <fwptypes.h>
#include <fwpmu.h>

#pragma comment(lib, "fwpuclnt.lib")

const WCHAR DRIVER_NAME[] = L"SnifferDriver.sys";

bool AddCallout(HANDLE engineHandle, GUID guid, GUID layer);

bool GetDriverPath(LPWSTR driverPath)
{
	size_t length;
	size_t driverNameLength = sizeof(DRIVER_NAME) / sizeof(WCHAR);

	length = GetModuleFileName(nullptr, driverPath, MAX_PATH);

	if (length == 0)
	{
		return false;
	}

	// find \ in path
	for (; length > 0 && driverPath[length] != L'\\'; --length);

	if (driverPath[length] != L'\\' || length + driverNameLength + 1 >= MAX_PATH)
	{
		return false;
	}

	for (int i = 0; i < driverNameLength; ++i)
	{
		length++;
		driverPath[length] = DRIVER_NAME[i];
	}

	return true;
}

bool InstallDriver()
{
	SC_HANDLE managerHandle = nullptr;
	SC_HANDLE serviceHandle = nullptr;
	WCHAR driverPath[MAX_PATH + 1];

	bool result = false;

	do
	{
		managerHandle = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
		if (managerHandle == nullptr)
		{
			break;
		}

		serviceHandle = OpenService(managerHandle, SNIFFER_DEVICE_NAME, SERVICE_ALL_ACCESS);
		if (serviceHandle != nullptr)
		{
			// driver is already installed
			break;
		}

		if (!GetDriverPath(driverPath))
		{
			break;
		}

		serviceHandle = CreateService(
			managerHandle,              // SCM database 
			SNIFFER_DEVICE_NAME,                   // name of service 
			SNIFFER_DEVICE_NAME,                   // service name to display 
			SERVICE_ALL_ACCESS,        // desired access 
			SERVICE_KERNEL_DRIVER, // service type 
			SERVICE_DEMAND_START,      // start type 
			SERVICE_ERROR_NORMAL,      // error control type 
			driverPath,                    // path to service's binary 
			nullptr,                      // no load ordering group 
			nullptr,                      // no tag identifier 
			nullptr,                      // no dependencies 
			nullptr,                      // LocalSystem account 
			nullptr);                     // no password 
		if (serviceHandle == nullptr && GetLastError() == ERROR_SERVICE_EXISTS)
		{
			serviceHandle = OpenService(managerHandle, SNIFFER_DEVICE_NAME, SERVICE_ALL_ACCESS);
		}
	} while (false);

	if (serviceHandle != nullptr)
	{
		result = StartService(serviceHandle, 0, nullptr);
		if (!result)
		{
			result = (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING);
		}
		else
		{
			DeleteService(serviceHandle);
		}
	}

	if (serviceHandle != nullptr)
	{
		CloseServiceHandle(serviceHandle);
	}

	if (managerHandle != nullptr)
	{
		CloseServiceHandle(managerHandle);
	}

	return result;
}

DWORD AddFilter(HANDLE engineHandle, GUID layerGuid, GUID calloutGuid, GUID filterGuid, GUID sublayerKey)
{
	FWPM_FILTER0 filter;
	ZeroMemory(&filter, sizeof(filter));

	filter.layerKey = layerGuid;
	filter.filterKey = filterGuid;
	filter.subLayerKey = sublayerKey;
	filter.action.type = FWP_ACTION_CALLOUT_INSPECTION;
	filter.action.calloutKey = calloutGuid;
	//filter.providerKey = &(provider.providerKey);
	filter.providerKey = nullptr;
	filter.weight.type = FWP_EMPTY;
	filter.displayData.name = const_cast<PWCHAR>(L"Filter");

	filter.numFilterConditions = 0;
	return FwpmFilterAdd0(engineHandle, &filter, nullptr, nullptr);
}

HANDLE StartSniffing()
{
	HANDLE engineHandle = nullptr;
	HANDLE fileHandle = nullptr;
	DWORD result = ERROR_SUCCESS;
	FWPM_SESSION0 session;
	WCHAR PROVIDER_NAME[] = L"SnifferProvider";
	WCHAR FILTER_NAME[] = L"SnifferFIlter";
	WCHAR SUBLAYER_NAME[] = L"Sniffersublayer";
	FWPM_FILTER_CONDITION filterConditions[1];

	InstallDriver();

	fileHandle = CreateFile(DEVICE_NAME,
		GENERIC_READ, 0, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, // | FILE_FLAG_OVERLAPPED (async IO?)
		INVALID_HANDLE_VALUE); // nullptr

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	do
	{
		ZeroMemory(&session, sizeof(session));
		session.txnWaitTimeoutInMSec = INFINITE;

		result = FwpmEngineOpen0(
			nullptr,
			RPC_C_AUTHN_WINNT,
			nullptr,
			&session,
			&engineHandle
		);

		if (result != ERROR_SUCCESS)
		{
			break;
		}


		result = FwpmTransactionBegin0(engineHandle, 0);
		if (result != ERROR_SUCCESS)
		{
			break;
		}

		FWPM_PROVIDER0 provider;

		ZeroMemory(&provider, sizeof(provider));
		provider.providerKey = PROVIDER_GUID;
		provider.flags = FWPM_PROVIDER_FLAG_PERSISTENT;
		provider.displayData.name = PROVIDER_NAME;

		result = FwpmProviderAdd0(engineHandle, &provider, nullptr);

		if (!(result == FWP_E_ALREADY_EXISTS || result == ERROR_SUCCESS))
		{
			break;
		}

		FWPM_SUBLAYER0 sublayer;
		ZeroMemory(&sublayer, sizeof(sublayer));
		sublayer.subLayerKey = SUBLAYER_GUID;
		sublayer.flags = FWPM_SUBLAYER_FLAG_PERSISTENT;
		sublayer.providerKey = &(provider.providerKey);
		sublayer.weight = 0x8000;
		sublayer.displayData.name = SUBLAYER_NAME;

		result = FwpmSubLayerAdd0(engineHandle, &sublayer, nullptr);
		if (!(result == FWP_E_ALREADY_EXISTS || result == ERROR_SUCCESS))
		{
			break;
		}

		/*FWPM_CALLOUT0 callout;
		ZeroMemory(&callout, sizeof(callout));
		callout.calloutKey = CALLOUT_GUID;
		callout.applicableLayer = FWPM_LAYER_OUTBOUND_MAC_FRAME_ETHERNET;
		callout.displayData.name = CALLOUT_NAME;*/
		//callout.flags = FWPM_CALLOUT_FLAG_PERSISTENT;

		result = AddCallout(engineHandle, MAC_IN_CALLOUT_GUID, FWPM_LAYER_INBOUND_MAC_FRAME_NATIVE);
		result = AddCallout(engineHandle, MAC_OUT_CALLOUT_GUID, FWPM_LAYER_OUTBOUND_MAC_FRAME_NATIVE);
		/*FWPM_FILTER0 filter;
		ZeroMemory(&filter, sizeof(filter));

		filter.layerKey = FWPM_LAYER_OUTBOUND_MAC_FRAME_ETHERNET;
		filter.subLayerKey = sublayer.subLayerKey;
		filter.action.type = FWP_ACTION_CALLOUT_INSPECTION;
		filter.action.calloutKey = CALLOUT_GUID;
		filter.providerKey = &(provider.providerKey);
		filter.weight.type = FWP_EMPTY;
		filter.displayData.name = FILTER_NAME;
		filter.filterKey = MAC_OUT_FILTER_GUID;

		filter.numFilterConditions = 1;
		ZeroMemory(filterConditions, sizeof(filterConditions));
		filterConditions[0].fieldKey = FWPM_CONDITION_L2_FLAGS;
		filterConditions[0].matchType = FWP_MATCH_FLAGS_ANY_SET;
		filterConditions[0].conditionValue.type = FWP_UINT32;
		filterConditions[0].conditionValue.uint32 =
			FWP_CONDITION_L2_IS_NATIVE_ETHERNET
			| FWP_CONDITION_L2_IS_WIFI;

		filter.filterCondition = filterConditions;

		result = FwpmFilterAdd0(engineHandle, &filter, nullptr, nullptr);*/

		FWPM_FILTER0 filter;
		ZeroMemory(&filter, sizeof(filter));

		result = AddFilter(engineHandle, FWPM_LAYER_INBOUND_MAC_FRAME_NATIVE, MAC_IN_CALLOUT_GUID, MAC_IN_FILTER_GUID, sublayer.subLayerKey);
		result = AddFilter(engineHandle, FWPM_LAYER_OUTBOUND_MAC_FRAME_NATIVE, MAC_OUT_CALLOUT_GUID, MAC_OUT_FILTER_GUID, sublayer.subLayerKey);

		//filter.layerKey = FWPM_LAYER_INBOUND_MAC_FRAME_NATIVE;
		//filter.filterKey = MAC_IN_FILTER_GUID;
		//filter.subLayerKey = sublayer.subLayerKey;
		//filter.action.type = FWP_ACTION_CALLOUT_INSPECTION;
		//filter.action.calloutKey = IP_CALLOUT_GUID;
		////filter.providerKey = &(provider.providerKey);
		//filter.providerKey = nullptr;
		//filter.weight.type = FWP_EMPTY;
		//filter.displayData.name = FILTER_NAME;

		//filter.numFilterConditions = 0;
		//result = FwpmFilterAdd0(engineHandle, &filter, nullptr, nullptr);
		/*filter.numFilterConditions = 1;
		ZeroMemory(filterConditions, sizeof(filterConditions));
		filterConditions[0].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
		filterConditions[0].matchType = FWP_MATCH_GREATER_OR_EQUAL;
		filterConditions[0].conditionValue.type = FWP_UINT16;
		filterConditions[0].conditionValue.uint16 = 80;

		filter.filterCondition = &filterConditions[0];*/


	} while (FALSE);

	if (engineHandle)
	{
		if (result != ERROR_SUCCESS)
		{
			FwpmTransactionAbort(engineHandle);
		}
		else
		{
			FwpmTransactionCommit(engineHandle);
		}
		FwpmEngineClose(engineHandle);
	}


	if (result == ERROR_SUCCESS)
	{
		return fileHandle;
	}
	else
	{
		CloseHandle(fileHandle);
		return 0;
	}
}

bool AddCallout(HANDLE engineHandle, GUID guid, GUID layer)
{
	WCHAR CALLOUT_NAME[] = L"Sniffercalluot";

	FWPM_CALLOUT0 callout;
	ZeroMemory(&callout, sizeof(callout));
	callout.calloutKey = guid;
	callout.applicableLayer = layer;
	callout.displayData.name = CALLOUT_NAME;

	DWORD result = FwpmCalloutAdd(engineHandle, &callout, nullptr, nullptr);
	if (!(result == FWP_E_ALREADY_EXISTS || result == ERROR_SUCCESS))
	{
		return false;
	}

	return true;
}

ULONG GetPacket(HANDLE handle, PUCHAR buffer, ULONG bufferLength)
{
	ULONG returned = 0;
	ReadFile(handle,
		buffer,
		bufferLength,
		&returned,
		nullptr);
	return returned;
}

void StopSniffing(HANDLE handle)
{
	if (handle != nullptr)
	{
		CloseHandle(handle);
	}
}
