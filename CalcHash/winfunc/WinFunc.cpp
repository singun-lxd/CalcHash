#include "stdafx.h"
#include "WinFunc.h"
#include "Win7ShellApi.h"
#include <propkey.h>
#include <propvarutil.h>

typedef WINUSERAPI BOOL WINAPI CHANGEWINDOWMESSAGEFILTER(UINT message, DWORD dwFlag);

//�����Ȩ�޽�����ǰ���̷�����Ϣ
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

//��ȡ����Ȩ�޺���
BOOL GetProcessElevation(TOKEN_ELEVATION_TYPE* pElevationType, BOOL* pIsAdmin)
{
	HANDLE hToken = NULL;
	DWORD dwSize;

	//��ȡ��ǰ��������
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
		return FALSE;

	BOOL bResult = FALSE;

	//��ȡ������Ϣ
	if (::GetTokenInformation(hToken, TokenElevationType,
		pElevationType, sizeof(TOKEN_ELEVATION_TYPE), &dwSize))
	{
		//����Administrators����ӦSID
		BYTE adminSID[SECURITY_MAX_SID_SIZE];
		dwSize = sizeof(adminSID);
		::CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID,
			&dwSize);

		if (*pElevationType == TokenElevationTypeLimited)
		{
			//��ȡ�������Ƶľ��
			HANDLE hUnfilteredToken = NULL;
			::GetTokenInformation(hToken, TokenLinkedToken, (VOID*)
				&hUnfilteredToken, sizeof(HANDLE), &dwSize);

			//��������Ƿ��������ԱSID
			if (::CheckTokenMembership(hUnfilteredToken, &adminSID, pIsAdmin))
			{
				bResult = TRUE;
			}

			//�رվ��
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

	//�رվ��
	::CloseHandle(hToken);

	return bResult;
}

//��Ȩ
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

// �Թ���ԱȨ������
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

//��ȡWindows�汾
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
	//����List
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
		//����ShellLink
		IShellLink *pSLAutoRun = NULL;
		hResult = CoCreateInstance(CLSID_ShellLink, NULL, 
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSLAutoRun));
		if(SUCCEEDED(hResult))
		{
			//��ȡӦ�ó���·��
			TCHAR szPath[MAX_PATH];
			GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);
			hResult = pSLAutoRun->SetPath(szPath);
			if(SUCCEEDED(hResult))
			{
				if(SUCCEEDED(hResult))
				{
					//ͼ��
					hResult = pSLAutoRun->SetIconLocation(szPath, 0);
					if(SUCCEEDED(hResult))
					{
						//�����в���
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
	//����
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