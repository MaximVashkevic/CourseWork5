#include <Windows.h>
#include "main.h"
#include "MainWindow.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WND_CLASS_NAME L"SnifferWindow"

#define WIDTH 1000
#define HEIGHT 600

BOOL HasAdministratorPrivileges()
{
	BOOL result = FALSE;
	HANDLE hToken = NULL;
	TOKEN_ELEVATION elevation;
	DWORD dwSize;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
		{
			result = elevation.TokenIsElevated;
		}
	}

	if (hToken)
	{
		CloseHandle(hToken);
	}

	return result;
}

HWND hWindow;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

	if (!HasAdministratorPrivileges())
	{
		MessageBox(nullptr, L"Run application with administrator privileges", L"Administrator privileges required", MB_OK);
		return 0;
	}
	
	MainWindow window(hInstance);

	if (!window.Create(hInstance, L"Sniffer", WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX | WS_MAXIMIZEBOX)))
	{
		return 0;
	}

	ShowWindow(window.GetHandle(), nCmdShow);

	MSG msg = { };

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
