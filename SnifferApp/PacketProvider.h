#pragma once
#include <vector>
#include <memory>
#include "BasePacket.h"
#include "../SnifferLib/SnifferLib.h"
#include "EthernetPacket.h"

#define MAX_CAPTURE_SIZE 2000

class PacketProvider {

public:
	void SetOnCapture(void* object, void (*func)(void* object, std::shared_ptr<BasePacket>*))
	{
		captureObserver = object;
		OnPacketCaptured = func;
	}

	void SetOnSelect(void* object, void (*func)(void* object, std::shared_ptr<BasePacket>*))
	{
		selectObserver = object;
		OnPacketSelected = func;
	}

	PacketProvider()
	{
		InitializeCriticalSectionAndSpinCount(&criticalSection, 0x1000);
	}

	void StartCapture()
	{
		if (!capturing)
		{
			EnterCriticalSection(&criticalSection);
			if (!capturing)
			{
				captureHandle = StartSniffing();
				capturing = true;
				HANDLE threadHandle = CreateThread(nullptr, 0, CaptureLoop, this, 0, nullptr);
				CloseHandle(threadHandle);
			}
			LeaveCriticalSection(&criticalSection);
		}
	}

	void StopCapture()
	{
		if (capturing)
		{
			EnterCriticalSection(&criticalSection);
			if (capturing)
			{
				captureHandle = StartSniffing();
				capturing = true;
			}
			LeaveCriticalSection(&criticalSection);
		}
	}

	void SelectPacket(int id)
	{
		OnPacketSelected(selectObserver, &packets[id]);
	}

	~PacketProvider()
	{
		EnterCriticalSection(&criticalSection);
		if (capturing)
		{
			StopCapture();
		}
		LeaveCriticalSection(&criticalSection);

		DeleteCriticalSection(&criticalSection);
	}
private:
	static DWORD WINAPI CaptureLoop(PVOID parameter)
	{
		PacketProvider* pProvider = (PacketProvider*)parameter;
		while (pProvider->capturing)
		{
			EnterCriticalSection(&pProvider->criticalSection);
			if (pProvider->capturing)
			{
				UINT8* pBuffer = new UINT8[MAX_CAPTURE_SIZE];
				int capturedLength = GetPacket(pProvider->captureHandle, pBuffer, MAX_CAPTURE_SIZE);
				std::shared_ptr<BasePacket> pPacket = std::make_shared<EthernetPacket>(capturedLength, pBuffer);
				pProvider->packets.push_back(pPacket);
				if (pProvider->OnPacketCaptured)
				{
					pProvider->OnPacketCaptured(pProvider->captureObserver, &pPacket);
				}
			}
			LeaveCriticalSection(&pProvider->criticalSection);
		}

		return 0;
	}

	bool capturing = false;
	std::vector<std::shared_ptr<BasePacket>> packets;
	CRITICAL_SECTION criticalSection;
	HANDLE captureHandle;

	void (*OnPacketCaptured)(void* object, std::shared_ptr<BasePacket>*);
	void* captureObserver;
	void (*OnPacketSelected)(void* object, std::shared_ptr<BasePacket>*);
	void* selectObserver;
};