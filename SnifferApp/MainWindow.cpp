#include "MainWindow.h"
#include  <Windows.h>
#include <memory>
#include <sstream>
#include <string>

static void OnPacketCaptured(void* object, BasePacket* pPacket)
{
	if (pPacket != NULL)
	{
		PostMessage((HWND)object, WM_PACKET, 0, (LPARAM)pPacket);
	}
}

static void OnPacketSelected(void* object, BasePacket* pPacket)
{
	if (pPacket != NULL)
	{
		PostMessage((HWND)object, WM_INFO, 0, (LPARAM)pPacket);
	}
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
		DestroyWindow(hPacketInfo);
		DestroyWindow(hPacketList);
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
		HandlePacket((BasePacket*)lParam);
		break;
	}
	case WM_INFO:
	{
		HandleInfo((BasePacket*)lParam);
		break;
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
	HFONT hFont = CreateFont(0, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
	SendMessage(hPacketInfo, WM_SETFONT, (WPARAM)hFont, TRUE);
	DeleteObject(hFont);
	DeleteObject(hFont);
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
	case PACKET_LIST_ID:
	{
		if (HIWORD(wParam) == LBN_SELCHANGE)
		{
			if (hPacketList != nullptr)
			{
				auto index = SendMessage(hPacketList, LB_GETCURSEL, 0, 0);
				if (index != LB_ERR)
				{
					provider.SelectPacket(index);
				}
			}
		}
		break;
	}
	}
}

void MainWindow::HandlePacket(BasePacket* packet)
{
	std::wstringstream s;

	s << packet->Destination() << L"<-" << packet->Source()
		<< " : " << packet->Length() << L" : " << packet->Protocol();

	if (hPacketList != nullptr)
	{
		SendMessage(hPacketList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s.str().c_str()));
	}
}

void MainWindow::HandleInfo(BasePacket* packet)
{
	if (hPacketInfo != nullptr)
	{
		std::wstring s = packet->Bytes();
		SendMessage(hPacketInfo, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(s.c_str()));
	}
}