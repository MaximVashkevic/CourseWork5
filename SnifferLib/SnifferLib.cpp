// SnifferLib.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"
#include "SnifferLib.h"
#include <fwptypes.h>
#include <fwpmu.h>

#pragma comment(lib, "fwpuclnt.lib")

// TODO: This is an example of a library function
void fnSnifferLib()
{
}

DWORD StartSniffing()
{
	HANDLE engineHandle = NULL;
	DWORD result = ERROR_SUCCESS;
	FWPM_SESSION0 session;
	do
	{
		ZeroMemory(&session, sizeof(session));
		session.txnWaitTimeoutInMSec = INFINITE;

		result = FwpmEngineOpen0(
			NULL,
			RPC_C_AUTHN_WINNT,
			NULL,
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

		result = FwpmProviderAdd0(engineHandle, &provider, NULL);

		if (result != FWP_E_ALREADY_EXISTS || result != ERROR_SUCCESS)
		{
			break;
		}

		FWPM_SUBLAYER0 sublayer;
		ZeroMemory(&sublayer, sizeof(sublayer));
		sublayer.subLayerKey = SUBLAYER_GUID;
		sublayer.flags = FWPM_SUBLAYER_FLAG_PERSISTENT;
		sublayer.providerKey = &(provider.providerKey);
		sublayer.weight = 0x8000;

		result = FwpmSubLayerAdd0(engineHandle, &sublayer, NULL);
		if (result != ERROR_SUCCESS)
		{
			if (result == FWP_E_ALREADY_EXISTS)
			{
				result = ERROR_SUCCESS;
			}

			break;
		}

		FWPM_CALLOUT0 callout;
		ZeroMemory(&callout, sizeof(callout));
		callout.calloutKey = CALLOUT_GUID;
		callout.applicableLayer = FWPM_LAYER_OUTBOUND_MAC_FRAME_ETHERNET;
		result = FwpmCalloutAdd(engineHandle, &callout, NULL, NULL);

		FWPM_FILTER0 filter;
		ZeroMemory(&filter, sizeof(filter));

		filter.layerKey = FWPM_LAYER_OUTBOUND_MAC_FRAME_ETHERNET;
		filter.subLayerKey = SUBLAYER_GUID;
		filter.action.type = FWP_ACTION_CALLOUT_INSPECTION;
		filter.action.calloutKey = CALLOUT_GUID;
		filter.providerKey = &(provider.providerKey);
		filter.weight.type = FWP_EMPTY;
		filter.numFilterConditions = 0;



		if (engineHandle && result != ERROR_SUCCESS)
		{
			FwpmTransactionAbort(engineHandle);
		}

	} while (FALSE);

	return result;
}
