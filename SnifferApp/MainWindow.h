#pragma once
#include "BaseWindow.h"
#include <Windows.h>

class MainWindow : public BaseWindow<MainWindow>
{
public:
	MainWindow(HINSTANCE hInstance) : BaseWindow() {}
	PCWSTR ClassName() const { return L"Window class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
}