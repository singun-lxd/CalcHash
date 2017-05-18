// CalcHash.cpp : main source file for CalcHash.exe
//

#include "stdafx.h"
#include "resource.h"
#include "CalcHash.h"
#include "MainDlg.h"
#include "StringDefine.h"
#include "FileMenu.h"
#include "WinFunc.h"

CAppModule _Module;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	if (lpstrCmdLine != NULL && ::_tcsicmp(lpstrCmdLine, ARG_SET_MENU) == 0)
	{
		DealWithWinver();
		return SetFileMenu();
	}

	HANDLE hMutex = NULL;
	BOOL bUnique = TRUE;
	BOOL bCmd = CheckSwitchAeroCmd(hMutex, bUnique, lpstrCmdLine);
	if (!bCmd)
	{
		if(hMutex != NULL)
			::CloseHandle(hMutex);

		return 0;
	}

	HRESULT hRes = ::CoInitialize(NULL);

	ATLASSERT(SUCCEEDED(hRes));

	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	CMainDlg dlgMain;
	dlgMain.m_lpszCmdLine = lpstrCmdLine;
	dlgMain.m_bUnique = bUnique;
	dlgMain.DoModal();

	//关闭Mutex
	if(hMutex != NULL)
		::CloseHandle(hMutex);

	_Module.Term();
	::CoUninitialize();

	return 0;
}

//如果已经启动了一个实例，调用该实例切换 Aero
BOOL CheckSwitchAeroCmd(HANDLE& hMutex, BOOL& bIsUnique, LPTSTR lpstrCmdLine)
{
	BOOL bReturn = TRUE;
	bIsUnique = TRUE;

	hMutex = ::CreateMutex(NULL, TRUE, WIN_UNIQUE_MUTEX);
	if (::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		bIsUnique = FALSE;
		if (lpstrCmdLine != NULL && ::_tcsicmp(lpstrCmdLine, ARG_SWITCH_AERO) == 0)
		{
			CString strTitle;
			strTitle.LoadString(IDS_CALC_HASH_TITLE);
			HWND hWnd = ::FindWindow(NULL, strTitle);
			if (hWnd != NULL)
			{
				if (::IsIconic(hWnd))
					::ShowWindow(hWnd, SW_RESTORE);

				hWnd = ::GetLastActivePopup(hWnd);
				::SwitchToThisWindow(hWnd, TRUE);
			}

			COPYDATASTRUCT cpdata = {0};
			cpdata.dwData = 1;
			cpdata.lpData = lpstrCmdLine;
			cpdata.cbData = (DWORD)(::_tcslen(lpstrCmdLine) + 1) * sizeof(TCHAR);
			::SendMessage(hWnd, WM_COPYDATA, COPYDATA_MUTEX, reinterpret_cast<LPARAM>(&cpdata));

			bReturn = FALSE;
		}
	}

	return bReturn;
}

BOOL SetFileMenu()
{
	if (::CheckMenuAvailable() == TRUE)
	{
		if (!::RemoveRightButtonMenu())
		{
			return FALSE;
		}
	}
	else if (!::AddRightButtonMenu())
	{
		::RemoveRightButtonMenu(NULL, FALSE);

		return FALSE;
	}

	return TRUE;
}

void DealWithWinver()
{
	UINT16 uWinVer = ::GetWindowsVersion();
	if (uWinVer >= WINVER_VISTA)
		g_bUseTaskDlg = TRUE;
	else
		::GetPrivateProfileInt(CONFIG_ROOT, CONFIG_ITEM_NOAERO, 0, CONFIG_FILE_NAME); //- - 随便调用一个函数，否则会崩溃
}
