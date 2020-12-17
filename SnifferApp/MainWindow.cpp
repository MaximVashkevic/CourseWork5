#include "MainWindow.h"
#include  <Windows.h>
#include <memory>
#include <sstream>
#include <string>

static void OnPacketCaptured(void* object, std::shared_ptr<BasePacket>* pPacket)
{
	PostMessage((HWND)object, WM_PACKET, 0, (LPARAM)pPacket);
}

static void OnPacketSelected(void* object, std::shared_ptr<BasePacket>* pPacket)
{
	PostMessage((HWND)object, WM_INFO, 0, (LPARAM)pPacket);
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		HandleCreate();
		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	case WM_COMMAND:
	{
		HandleCommand(wParam);
		break;
	}
	case WM_PACKET:
	{
		HandlePacket(*static_cast<std::shared_ptr<BasePacket>*>((void*)lParam));
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MainWindow::HandleCreate()
{
	GetClientRect(hwnd, &clientRect);
	SetupMenu();
	SetupPacketList();
	SetupPacketInfo();
	provider.SetOnCapture(hwnd, OnPacketCaptured);
	provider.SetOnSelect(hwnd, OnPacketSelected);
}

void MainWindow::SetupPacketInfo()
{
	hPacketInfo = CreateWindow(L"EDIT",
		nullptr,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		clientRect.right / 2, 0, clientRect.right, clientRect.bottom,
		hwnd, HMENU(PACKET_INFO_ID), nullptr, nullptr);
}

void MainWindow::SetupPacketList()
{
	hPacketList = CreateWindow(
		L"LISTBOX", L"Packets",
		WS_CHILD | WS_VSCROLL | WS_BORDER | WS_VISIBLE | LBS_NOTIFY,
		0, 0, clientRect.right / 2, clientRect.bottom,
		hwnd, HMENU(PACKET_LIST_ID), nullptr, nullptr);
}

void MainWindow::SetupMenu()
{
	hMenu = CreateMenu();
	AppendMenu(hMenu, MF_STRING, START_ID, L"Start");
	AppendMenu(hMenu, MF_STRING, STOP_ID, L"Stop");
	SetMenu(hwnd, hMenu);
}

void MainWindow::HandleCommand(WPARAM wParam)
{
	switch (LOWORD(wParam))
	{
	case START_ID:
	{
		provider.StartCapture();
		break;
	}
	case STOP_ID:
	{
		provider.StopCapture();
		break;
	}

	default:
		break;
	}
}

void MainWindow::HandlePacket(std::shared_ptr<BasePacket> packet)
{
	std::wstringstream s;

	s << packet->Destination() << L"<-" << packet->Source()
		<< " : " << packet->Length() << L" : " << packet->Protocol();

	if (hPacketList != nullptr)
		SendMessage(hPacketList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s.str().c_str()));
}
