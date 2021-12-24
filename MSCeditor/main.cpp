
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdlib.h>
#include <tchar.h>

#include <windows.h> 
#include <CommCtrl.h>

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#include "resource.h"
#include "version.h"
#include "resize.h"

#include <string>

WNDPROC DefaultListCtrlProc;
HWND hDialog;
HINSTANCE hInst;
PVOID pResizeState = NULL;

static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

const std::wstring Version = std::to_wstring(VERSION_MAJOR) + L"." + std::to_wstring(VERSION_MINOR);
const std::wstring Title = L"DialogResizeDemo " + Version;

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	hInst = hInstance;
	ResizeDialogInitialize(hInst);
	return static_cast<int>(DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DlgProc, NULL));
} 

// ======================
// dlg procs
// ======================

static LRESULT CALLBACK ListCtrlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_RBUTTONDOWN:
	{
		LVHITTESTINFO itemclicked;
		long x, y;
		x = (long)GET_X_LPARAM(lParam);
		y = (long)GET_Y_LPARAM(lParam);
		itemclicked.pt.x = x;
		itemclicked.pt.y = y;
		int lResult = ListView_SubItemHitTest(hwnd, &itemclicked);
		if (lResult != -1)
		{
			RECT rekt;
			GetWindowRect(hwnd, &rekt);
			rekt.top += GET_Y_LPARAM(lParam);
			rekt.left += GET_X_LPARAM(lParam);
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		LVHITTESTINFO itemclicked;
		itemclicked.pt.x = GET_X_LPARAM(lParam);
		itemclicked.pt.y = GET_Y_LPARAM(lParam);
		const int lResult = ListView_SubItemHitTest(hwnd, &itemclicked);
		if (lResult != -1)
		{
			if (itemclicked.iSubItem == 1) { break; }

			RECT rekt;
			ListView_GetSubItemRect(hwnd, itemclicked.iItem, itemclicked.iSubItem, LVIR_BOUNDS, &rekt);
			const int pos_left = rekt.left;

			ListView_GetSubItemRect(hwnd, itemclicked.iItem, itemclicked.iSubItem, LVIR_LABEL, &rekt);
			const int height = rekt.bottom - rekt.top;
			const int width = rekt.right - pos_left;
		}
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		LVHITTESTINFO htnfo;
		htnfo.pt.x = (long)GET_X_LPARAM(lParam);
		htnfo.pt.y = (long)GET_Y_LPARAM(lParam);
		const int lResult = ListView_SubItemHitTest(hwnd, &htnfo);
		return TRUE;
	}
	case WM_SIZE:
	{
		ListView_SetColumnWidth(hwnd, 1, LOWORD(lParam) - SendMessage(hwnd, LVM_GETCOLUMNWIDTH, 0, 0) - GetSystemMetrics(SM_CXVSCROLL));
		return FALSE;
	}
	case WM_COMMAND:
	{
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			const auto ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			const auto oSize = SendMessage((HWND)lParam, (UINT)CB_GETLBTEXTLEN, (WPARAM)ItemIndex, 0);
			std::wstring value(oSize, '\0');
			SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)value.data());
			break;
		}

		break;
	}
	}
	return CallWindowProc(DefaultListCtrlProc, hwnd, message, wParam, lParam);
}

static void ClearStatic(HWND hStatic, HWND hDlg)
{
	RECT rekt;
	GetClientRect(hStatic, &rekt);
	InvalidateRect(hStatic, &rekt, TRUE);
	MapWindowPoints(hStatic, hDlg, (POINT*)&rekt, 2);
	RedrawWindow(hDlg, &rekt, NULL, RDW_ERASE | RDW_INVALIDATE);
}

static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	ResizeDialogProc(hwnd, Message, wParam, lParam, &pResizeState);

	switch (Message)
	{
		case WM_INITDIALOG:
		{
			//init common controls
			INITCOMMONCONTROLSEX iccex;
			iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			iccex.dwICC = ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;

			if (!InitCommonControlsEx(&iccex))
			{
				MessageBox(hwnd, L"Could not initialize common controls!", Title.c_str(), MB_OK | MB_ICONERROR);
				EndDialog(hwnd, 0);
			}

			hDialog = hwnd;

			DefaultListCtrlProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd, IDC_List2), GWLP_WNDPROC, (LONG_PTR)ListCtrlProc);

			// Set Icon and title
			HICON hicon = static_cast<HICON>(LoadImageW(hInst, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED | LR_DEFAULTSIZE));
			SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicon);
			SetWindowText(hDialog, (LPCWSTR)Title.c_str());

			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case ID_FILE_EXIT:
					EndDialog(hDialog, 0);
					break;
			}
			
			// FILTER
			if ((HIWORD(wParam) == EN_UPDATE) && (LOWORD(wParam) == IDC_FILTER))
			{
				UINT size = GetWindowTextLength(GetDlgItem(hwnd, IDC_FILTER)) + 1;
				std::wstring str(size, '\0');
				GetWindowText(GetDlgItem(hwnd, IDC_FILTER), (LPWSTR)str.data(), size);
				str.resize(size - 1);

				break;
			}
		} 

		case WM_SIZE:
			ClearStatic(GetDlgItem(hwnd, IDC_STXT1), hDialog);
			ClearStatic(GetDlgItem(hwnd, IDC_STXT2), hDialog);
			break;

		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == GetDlgItem(hDialog, IDC_OUTPUT3))
			{
				SetBkMode((HDC)wParam, COLOR_WINDOW);
				return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_WINDOW));
			}
			else if ((HWND)lParam == GetDlgItem(hDialog, IDC_OUTPUT4))
			{
				SetBkMode((HDC)wParam, COLOR_WINDOW);
				SetTextColor((HDC)wParam, RGB(255, 0, 0));
				return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_WINDOW));
			}
			else
			{
				SetBkMode((HDC)wParam, COLOR_WINDOW);
				return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_WINDOW));
			}
			break;

		case WM_CLOSE:
			EndDialog(hDialog, 0);
			return FALSE;

		default:
			return FALSE;
	}
	return TRUE;
}