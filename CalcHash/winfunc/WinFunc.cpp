#include "stdafx.h"
#include "WinFunc.h"
#include "Win7ShellApi.h"
#include <propkey.h>
#include <propvarutil.h>

typedef WINUSERAPI BOOL WINAPI CHANGEWINDOWMESSAGEFILTER(UINT message, DWORD dwFlag);

//允许低权限进程向当前进程发送消息
BOOL EnableUACMessage()
{
	BOOL bResult = FALSE;

	HINSTANCE hDllInst = ::LoadLibrary(_T("user32.dll"));
	if (hDllInst)
	{
		CHANGEWINDOWMESSAGEFILTER *pAddMessageFilterFunc =
			(CHANGEWINDOWMESSAGEFILTER *)::GetProcAddress(hDllInst, "ChangeWindowMessageFilter");
		if (pAddMessageFilterFunc)
		{
			pAddMessageFilterFunc(0x0049, MSGFLT_ADD);
			pAddMessageFilterFunc(WM_DROPFILES, MSGFLT_ADD);
			pAddMessageFilterFunc(WM_COPYDATA, MSGFLT_ADD);
			bResult = TRUE;
		}
		::FreeLibrary(hDllInst);
	}

	return bResult;
}

//获取进程权限函数
BOOL GetProcessElevation(TOKEN_ELEVATION_TYPE* pElevationType, BOOL* pIsAdmin)
{
	HANDLE hToken = NULL;
	DWORD dwSize;

	//获取当前进程令牌
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
		return FALSE;

	BOOL bResult = FALSE;

	//获取令牌信息
	if (::GetTokenInformation(hToken, TokenElevationType,
		pElevationType, sizeof(TOKEN_ELEVATION_TYPE), &dwSize))
	{
		//创建Administrators组相应SID
		BYTE adminSID[SECURITY_MAX_SID_SIZE];
		dwSize = sizeof(adminSID);
		::CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID,
			&dwSize);

		if (*pElevationType == TokenElevationTypeLimited)
		{
			//获取连接令牌的句柄
			HANDLE hUnfilteredToken = NULL;
			::GetTokenInformation(hToken, TokenLinkedToken, (VOID*)
				&hUnfilteredToken, sizeof(HANDLE), &dwSize);

			//检查令牌是否包含管理员SID
			if (::CheckTokenMembership(hUnfilteredToken, &adminSID, pIsAdmin))
			{
				bResult = TRUE;
			}

			//关闭句柄
			::CloseHandle(hUnfilteredToken);
		}
		else
		{  
			*pIsAdmin = ::IsUserAnAdmin();
			bResult = TRUE;
		}  
	}
	else
	{
		*pIsAdmin = ::IsUserAnAdmin();
		bResult = TRUE;
	}

	//关闭句柄
	::CloseHandle(hToken);

	return bResult;
}

//提权
BOOL PrivilegeElevate(LPTSTR lpszArg, DWORD dwSuccessCode)
{
	TCHAR szFullPath[MAX_PATH];
	::ZeroMemory(szFullPath, MAX_PATH);
	::GetModuleFileName(NULL, szFullPath, MAX_PATH);
	DWORD dwExitCode = 0;
	if (::RunAsAdmin(NULL, szFullPath, lpszArg, &dwExitCode) &&
		dwExitCode == dwSuccessCode)
	{
		return TRUE;
	}

	return FALSE;
}

// 以管理员权限运行
BOOL RunAsAdmin(HWND hWnd, LPTSTR lpFile, LPTSTR lpParameters, LPDWORD lpExitCode)
{
	SHELLEXECUTEINFO  sei;
	ZeroMemory (&sei, sizeof(sei));

	sei.cbSize          = sizeof(SHELLEXECUTEINFOW);
	sei.hwnd            = hWnd;
	sei.fMask           = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb          = _T("runas");
	sei.lpFile          = lpFile;
	sei.lpParameters    = lpParameters;
	sei.nShow           = SW_SHOWNORMAL;
	sei.hProcess		= 0;

	if (::ShellExecuteEx(&sei))
	{
		if (lpExitCode != NULL)
		{
			if (::WaitForSingleObject(sei.hProcess, INFINITE) == WAIT_OBJECT_0)
			{
				::GetExitCodeProcess(sei.hProcess, lpExitCode);
			}
		}
		::CloseHandle(sei.hProcess);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

//获取Windows版本
UINT16 GetWindowsVersion()
{
	UINT16 uWinVer = LOWORD(::GetVersion());  
	uWinVer = MAKEWORD(HIBYTE(uWinVer), LOBYTE(uWinVer));  

	return uWinVer;  
}

void SetForegroundWindow(BOOL bTopMost, HWND hWnd)
{
	if (!bTopMost)
	{
		::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
		::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
	}
	::SetForegroundWindow(hWnd); 
}

BOOL SwitchAero()
{
	BOOL bReturn = FALSE;

	CDwmApiImpl dwmApi;
	if(dwmApi.Initialize() && SUCCEEDED(dwmApi.DwmEnableComposition(
		dwmApi.IsDwmCompositionEnabled() 
		? DWM_EC_DISABLECOMPOSITION
		: DWM_EC_ENABLECOMPOSITION)))
	{
		bReturn = TRUE;
	}

	return bReturn;
}

BOOL GetShortCutFile(LPCTSTR lpszLnkFile, LPTSTR lpszTarget, INT nSize)
{
	HRESULT           hResult;
	IShellLink        *pShellLink;
	IPersistFile      *pPersistFile;

	hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellLink, (void**)&pShellLink);
	if(SUCCEEDED(hResult))
	{
		hResult = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
		if(SUCCEEDED(hResult))
		{
			hResult = pPersistFile->Load(lpszLnkFile, STGM_READ);
			if(SUCCEEDED(hResult))
				hResult = pShellLink->GetPath(lpszTarget, nSize, NULL, 0);
			pPersistFile->Release();
		}
		pShellLink->Release();
	}

	return SUCCEEDED(hResult);
}

BOOL CreateJumpList(LPCTSTR lpszName, LPCTSTR lpszArg)
{
	HRESULT hResult;
	//创建List
	ICustomDestinationList *pList = NULL;
	hResult = CoCreateInstance(CLSID_DestinationList, NULL, 
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pList));
	if(SUCCEEDED(hResult))
	{
		//BeginList
		UINT uMinSlots;
		IObjectArray *pOARemoved = NULL;
		hResult = pList->BeginList(&uMinSlots, IID_PPV_ARGS(&pOARemoved));
		if(SUCCEEDED(hResult))
		{
			//ObjectCollection
			IObjectCollection *pOCTasks = NULL;
			hResult = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL,
				CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pOCTasks));
			if(SUCCEEDED(hResult))
			{
				hResult = AddShellLink(pOCTasks, lpszName, lpszArg);
				if(SUCCEEDED(hResult))
				{
					//ObjectArray
					IObjectArray *pOATasks = NULL;
					hResult = pOCTasks->QueryInterface(IID_PPV_ARGS(&pOATasks));
					if(SUCCEEDED(hResult))
					{
						hResult = pList->AddUserTasks(pOATasks);
						if(SUCCEEDED(hResult))
						{
							hResult = pList->CommitList();
						}
						pOATasks->Release();
					}
				}
				pOCTasks->Release();
			}
			pOARemoved->Release();
		}
		pList->Release();
	}

	return SUCCEEDED(hResult);
}

HRESULT AddShellLink(IObjectCollection *pOCTasks, LPCTSTR lpszName, LPCTSTR lpszArg)
{
	HRESULT hResult = E_FAIL;

	if (lpszName && lpszArg)
	{
		//创建ShellLink
		IShellLink *pSLAutoRun = NULL;
		hResult = CoCreateInstance(CLSID_ShellLink, NULL, 
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSLAutoRun));
		if(SUCCEEDED(hResult))
		{
			//获取应用程序路径
			TCHAR szPath[MAX_PATH];
			GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);
			hResult = pSLAutoRun->SetPath(szPath);
			if(SUCCEEDED(hResult))
			{
				if(SUCCEEDED(hResult))
				{
					//图标
					hResult = pSLAutoRun->SetIconLocation(szPath, 0);
					if(SUCCEEDED(hResult))
					{
						//命令行参数
						hResult = pSLAutoRun->SetArguments(lpszArg);
						if(SUCCEEDED(hResult))
						{
							hResult = SetTitle(pSLAutoRun, lpszName);
							if(SUCCEEDED(hResult))
							{
								hResult = pOCTasks->AddObject(pSLAutoRun);
							}
						}
					}
				}
			}
			pSLAutoRun->Release();
		}
	}

	return hResult;
}

HRESULT SetTitle(IShellLink * pShellLink, LPCTSTR szTitle)
{
	HRESULT hResult;
	//标题
	IPropertyStore *pPS = NULL;
	hResult = pShellLink->QueryInterface(IID_PPV_ARGS(&pPS));
	if(SUCCEEDED(hResult))
	{
		PROPVARIANT pvTitle;
		hResult = InitPropVariantFromString(szTitle,&pvTitle);
		if(SUCCEEDED(hResult))
		{
			hResult = pPS->SetValue(PKEY_Title, pvTitle);
			if(SUCCEEDED(hResult))
			{
				hResult = pPS->Commit();
			}
			PropVariantClear(&pvTitle);
		}
		pPS->Release();
	}
	return hResult;
}