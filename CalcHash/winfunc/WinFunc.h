#pragma once
#include <uxtheme.h>
#include <dwmapi.h>

#define WINVER_WIN8		0x0602
#define WINVER_WIN7		0x0601
#define WINVER_VISTA	0x0600
#define WINVER_WINXP	0x0501

#ifndef MSGFLT_ADD
#define MSGFLT_ADD 1
#endif
#ifndef MSGFLT_REMOVE
#define MSGFLT_REMOVE 2
#endif

class CDwmApiImpl
{
private:
	HINSTANCE m_hDwmApiLib;
	BOOL IsInitialized(void);

public:
	CDwmApiImpl(void);
	~CDwmApiImpl(void);
	BOOL Initialize(void);
	HRESULT DwmExtendFrameIntoClientArea(HWND hWnd,const MARGINS* pMarInset);
	BOOL IsDwmCompositionEnabled(void);
	HRESULT DwmEnableComposition(UINT uCompositionAction);
};

BOOL EnableUACMessage();
BOOL GetProcessElevation(TOKEN_ELEVATION_TYPE* pElevationType, BOOL* pIsAdmin);
BOOL PrivilegeElevate(LPTSTR lpszArg, DWORD dwSuccessCode);
BOOL RunAsAdmin(HWND hWnd, LPTSTR lpFile, LPTSTR lpParameters, LPDWORD lpExitCode = NULL);
UINT16 GetWindowsVersion();
BOOL SwitchAero();
void SetForegroundWindow(BOOL bTopMost, HWND hWnd);
BOOL GetShortCutFile(LPCTSTR lpszLnkFile, LPTSTR lpszTarget, INT nSize);
BOOL CreateJumpList(LPCTSTR lpszName, LPCTSTR lpszArg);
HRESULT AddShellLink(IObjectCollection *pOCTasks, LPCTSTR lpszName, LPCTSTR lpszArg);
HRESULT SetTitle(IShellLink * pShellLink, LPCTSTR szTitle);
