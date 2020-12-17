#pragma once
#include <Windows.h>

template <class DerivedType>
class BaseWindow
{
public:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DerivedType* pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (DerivedType*)pCreate->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->hwnd = hWnd;
		}
		else
		{
			pThis = (DerivedType*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		}
		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	BaseWindow() : hwnd(NULL) {}

	BOOL Create(HINSTANCE hInstance, PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		HWND hWndParent = 0,
		HMENU hMenu = 0)
	{
		WNDCLASS wc = {};
		wc.lpfnWndProc = DerivedType::WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = ClassName();
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

		RegisterClass(&wc);

		hwnd = CreateWindowEx(dwExStyle, ClassName(), lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, this);

		return hwnd ? TRUE : FALSE;
	}

	HWND GetHandle() const { return hwnd; }
protected:
	virtual PCWSTR ClassName() const = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	HWND hwnd;
};