#include "MainWindow.h"
#include  <Windows.h>

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{

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
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
