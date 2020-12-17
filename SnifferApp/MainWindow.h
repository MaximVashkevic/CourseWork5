#pragma once
#include "BaseWindow.h"
#include <Windows.h>
#include "PacketProvider.h"

#define WM_INFO (WM_APP + 0x10)
#define WM_PACKET (WM_APP + 0x11)

class MainWindow : public BaseWindow<MainWindow>
{
public:
	explicit MainWindow(HINSTANCE hInstance) : BaseWindow() {}
	PCWSTR ClassName() const { return L"Window class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void HandleCreate();
	void SetupPacketInfo();
	void SetupPacketList();
	void SetupMenu();
	void HandleCommand(WPARAM wParam);
	void HandlePacket(std::shared_ptr<BasePacket>);
	PacketProvider provider;
	HMENU hMenu;
	HWND hPacketList;
	HWND hPacketInfo;
	RECT clientRect;
	static const WORD START_ID = 1;
	static const WORD STOP_ID = 2;
	static const WORD PACKET_LIST_ID = 3;
	static const WORD PACKET_INFO_ID = 4;
};