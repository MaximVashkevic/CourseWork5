#pragma once
#include <vector>
#include <memory>
#include "BasePacket.h"
#include "../SnifferLib/SnifferLib.h"
#include "EthernetPacket.h"

#define MAX_CAPTURE_SIZE 2000

class PacketProvider {

public:
	void SetOnCapture(void* object, void (*func)(void* object, BasePacket* packet))
	{
		captureObserver = object;
		OnPacketCaptured = func;
	}

	void SetOnSelect(void* object, void (*func)(void* object, BasePacket* packet))
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
				if (captureHandle)
				{
					capturing = true;
					HANDLE threadHandle = CreateThread(nullptr, 0, CaptureLoop, this, 0, nullptr);
					CloseHandle(threadHandle);
				}
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
				StopSniffing(captureHandle);
				capturing = false;
			}
			LeaveCriticalSection(&criticalSection);
		}
	}

	void SelectPacket(int id)
	{
		OnPacketSelected(selectObserver, packets[id]);
	}

	~PacketProvider()
	{
		EnterCriticalSection(&criticalSection);
		if (capturing)
		{
			StopCapture();
		}
		LeaveCriticalSection(&criticalSection);

		for (auto pPacket : packets)
		{
			delete pPacket;
		}

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
				EthernetPacket* pPacket = new EthernetPacket(capturedLength, pBuffer);
				pProvider->packets.push_back(pPacket);
				if (pProvider->OnPacketCaptured)
				{
					pProvider->OnPacketCaptured(pProvider->captureObserver, pPacket);
				}
			}
			LeaveCriticalSection(&pProvider->criticalSection);
		}

		return 0;
	}

	bool capturing = false;
	std::vector<BasePacket*> packets;
	CRITICAL_SECTION criticalSection;
	HANDLE captureHandle;

	void (*OnPacketCaptured)(void* object, BasePacket*);
	void* captureObserver;
	void (*OnPacketSelected)(void* object, BasePacket*);
	void* selectObserver;
};