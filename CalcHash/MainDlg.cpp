// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "StringDefine.h"
#include "MainDlg.h"
#include "aerosubc.h"
#include "WinFunc.h"
#include "PrivilegeDlg.h"
#include "PrivilegeNewDlg.h"
#include "Win7ShellApi.h"
#include "MessageDlg.h"
#include "FileMenu.h"
#include <process.h>

//对话框构造函数
CMainDlg::CMainDlg()
{
	//初始化成员变量
	m_bUnique = TRUE;
	m_hThread = NULL;
	m_llSize = 0;
	m_nErrCode = 0;
	m_bUseAero = FALSE;
	m_bAdmin = FALSE;
	m_uWinVer = WINVER_WINXP;
	m_nLnkOption = 0;
	m_lpszCmdLine = NULL;
	m_pTaskbarList = NULL;
	::memset(&m_stTime, 0x0, sizeof(m_stTime));
}

//对话框析构函数
CMainDlg::~CMainDlg()
{
	//清理工作
	if(m_hThread != NULL)
	{
		DWORD dwRet = ::WaitForSingleObject(m_hThread, 3000);
		if (dwRet != WAIT_OBJECT_0)
		{
			::TerminateThread(m_hThread, 0);
		}
		::CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

LRESULT CMainDlg::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UINT_PTR nID = (wParam & 0xFFF0);
	if (nID == IDM_TOGGLE)
	{
		if(::MessageDlg(m_hWnd, IDS_MSGBOX_AERO_TOGGLE,
			MB_ICONEXCLAMATION | MB_OKCANCEL | MB_DEFBUTTON2) == IDOK)
		{
			if (!::SwitchAero())
				::MessageDlg(m_hWnd, IDS_MSGBOX_AERO_FAILED, MB_ICONERROR);
		}
		bHandled = TRUE;
	}
	else if (nID == SC_CLOSE)
	{
		if (m_bUnique && m_uWinVer >= WINVER_WIN7)
		{
			SetTaskCommand();
		}

		if (m_pTaskbarList != NULL)
		{
			m_pTaskbarList->Release();
			m_pTaskbarList = NULL;
		}

		SaveConfig();
		bHandled = FALSE;
	}
	else
	{
		bHandled = FALSE;
	}

	return 0;
}

LRESULT CMainDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//窗口居中
	CenterWindow();

	//设置图标
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
		::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);

	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
		::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	TOKEN_ELEVATION_TYPE emTeType = TokenElevationTypeDefault;
	::GetProcessElevation(&emTeType, &m_bAdmin);
	if (emTeType == TokenElevationTypeLimited)
	{
		m_bAdmin = FALSE;
	}

	m_uWinVer = ::GetWindowsVersion();
	BOOL bUseAero = LoadConfig();
	m_bAeroConfig = bUseAero;
	BOOL bArgsHandled = HandleCmdLineArgs(bUseAero);

	InitControls(bUseAero);

	if (!bArgsHandled)
		HandleCmdLineFiles();

	return FALSE;
}

void CMainDlg::InitControls(BOOL bUseAero)
{
	//控件赋值
	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
	GetDlgItem(IDC_EDIT2).SetFocus();
	GetDlgItem(IDC_EDIT3).EnableWindow(FALSE);

	CEdit edtText;
	edtText.Attach(GetDlgItem(IDC_EDIT3));
	edtText.SetLimitText(MAX_PATH);

	// 右键菜单选项
	::CheckMenuAvailable(m_hWnd);

	// Vista 及以上版本
	if (m_uWinVer >= WINVER_VISTA)
	{
		//使用新式对话框
		g_bUseTaskDlg = TRUE;

		//Aero控件初始化
		if (bUseAero && AeroAutoSubclass(m_hWnd, ASC_SUBCLASS_ALL_CONTROLS, 0L))
		{
			if (m_uWinVer < WINVER_WIN8)
			{
				HMENU hSysMenu = GetSystemMenu(FALSE);
				if (hSysMenu != NULL)
				{
					CString strMenu;
					strMenu.LoadString(IDS_MENU_TOGGLE_AERO);
					AppendMenu(hSysMenu, MF_SEPARATOR, NULL, NULL);
					AppendMenu(hSysMenu, MF_STRING, IDM_TOGGLE, strMenu);
				}
			}

			m_bUseAero = TRUE;
		}

		// Win7 及以上版本
		if (m_uWinVer >= WINVER_WIN7)
		{
			if (m_bUseAero)
			{
				if (m_uWinVer < WINVER_WIN8)
				{
					CString strAero;
					strAero.LoadString(IDS_JUMPLIST_TOGGLE_AERO);
					::CreateJumpList(strAero, ARG_SWITCH_AERO);
				}
				else
				{
					SetTaskCommand();
				}
			}
			else
			{
				SetTaskCommand();
			}

			// 任务栏接口
			::CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, 
				__uuidof(ITaskbarList3), (void **)&m_pTaskbarList);

			if (m_pTaskbarList != NULL)
			{
				if (m_bAdmin)
				{
					// 在任务栏图标上显示 UAC 小图标
					SetTimer(TIMER_ID, TIMER_WAIT);
				}
			}
		}

		// 管理员权限下
		if (m_bAdmin)
		{
			// 必须在这里执行才有效
			::EnableUACMessage();
		}
	}
}

BOOL CMainDlg::HandleCmdLineArgs(BOOL& bUseAero)
{
	BOOL bReturn = TRUE;

	if (m_lpszCmdLine != NULL && m_lpszCmdLine[0] != _T('\0'))
	{
		if (m_lpszCmdLine[0] != _T('-'))
		{
			bReturn = FALSE;
		}
		else if (::_tcsicmp(m_lpszCmdLine, ARG_SET_MENU) == 0)
		{
			CheckDlgButton(IDC_CHECK1, !IsDlgButtonChecked(IDC_CHECK1));
			BOOL bHandled = FALSE;
			SetMenuItem(0, 0, 0, bHandled);
		}
		else if (::_tcsicmp(m_lpszCmdLine, ARG_NO_AERO) == 0)
		{
			bUseAero = FALSE;
		}
		else if (::_tcsicmp(m_lpszCmdLine, ARG_USE_AERO) == 0)
		{
			bUseAero = TRUE;
		}
		else
		{
			::MessageDlg(m_hWnd, IDS_MSGBOX_INVALID_ARG, m_lpszCmdLine, MB_ICONEXCLAMATION);
		}
	}

	return bReturn;
}

void CMainDlg::HandleCmdLineFiles()
{
	CString strCmdLine = m_lpszCmdLine;
	if (strCmdLine.Left(1) == _T("\""))
		strCmdLine.Delete(0);
	if (strCmdLine.Right(1) == _T("\""))
		strCmdLine.Delete(strCmdLine.GetLength() - 1);

	m_lpszCmdLine = strCmdLine.GetBuffer(MAX_PATH);
	if (m_nLnkOption != 2 && strCmdLine.Right(4) == _T(".lnk"))
	{
		TCHAR szTemp[MAX_PATH] = {0};
		if (::GetShortCutFile(m_lpszCmdLine, szTemp, MAX_PATH))
		{
			if (m_nLnkOption == 1)
			{
				m_lpszCmdLine = szTemp;
			}
			else
			{
				BOOL bChecked = FALSE;
				if (::MessageDlg(m_hWnd, IDS_MSGBOX_LNK_SELECT,
					MB_ICONQUESTION | MB_YESNO, &bChecked) == IDYES)
				{
					m_lpszCmdLine = szTemp;
					if (bChecked)
						SaveLnkOption(TRUE);
				}
				else if (bChecked)
				{
					SaveLnkOption(FALSE);
				}
			}
		}
	}
	SetDlgItemText(IDC_EDIT2, m_lpszCmdLine);
	strCmdLine.ReleaseBuffer();
	m_lpszCmdLine = NULL;

	BOOL bHandled = FALSE;
	OnBtnCalc(0, 0, 0, bHandled);
}

void CMainDlg::SetTaskCommand()
{
	CString strAero;
	if (m_bAeroConfig)
	{
		strAero.LoadString(IDS_JUMPLIST_NO_AERO);
		::CreateJumpList(strAero, ARG_NO_AERO);
	}
	else
	{
		strAero.LoadString(IDS_JUMPLIST_USE_AERO);
		::CreateJumpList(strAero, ARG_USE_AERO);
	}
}

LRESULT CMainDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	KillTimer(TIMER_ID);

	HICON hIcon = AtlLoadIconImage(IDI_ADMIN, LR_DEFAULTCOLOR,
		::GetSystemMetrics(SM_CYSMICON), ::GetSystemMetrics(SM_CYSMICON));
	m_pTaskbarList->SetOverlayIcon(m_hWnd, hIcon, NULL);

	return 0;
}

LRESULT CMainDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GetDlgItem(IDC_EDIT2).EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON2).EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT3).EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT2).SetFocus();

	return 0;
}

LRESULT CMainDlg::OnString(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GetDlgItem(IDC_EDIT2).EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON2).EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT3).EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT3).SetFocus();

	return 0;
}

LRESULT CMainDlg::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bCalledHandled = FALSE;
	if (IsDlgButtonChecked(IDC_RADIO1) != TRUE)
	{
		CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
		OnFile(0, 0, 0, bCalledHandled);
	}

	HDROP hDropInfo = (HDROP)wParam;
	TCHAR szFilePathName[MAX_PATH] = {0};
	::DragQueryFile(hDropInfo, 0, szFilePathName, MAX_PATH);
	::DragFinish(hDropInfo);
	LPCTSTR lpszFile = szFilePathName;
	if (m_nLnkOption != 2)
	{
		TCHAR szTemp[MAX_PATH] = {0};
		::_tsplitpath_s(szFilePathName, NULL, 0, NULL, 0, NULL, 0, szTemp, MAX_PATH);
		if (::_tcsicmp(szTemp, _T(".lnk")) == 0 && 
			::GetShortCutFile(szFilePathName, szTemp, MAX_PATH))
		{
			if (m_nLnkOption == 1)
			{
				lpszFile = szTemp;
			}
			else
			{
				BOOL bChecked = FALSE;
				if (::MessageDlg(m_hWnd, IDS_MSGBOX_LNK_SELECT,
					MB_ICONINFORMATION | MB_YESNO, &bChecked) == IDYES)
				{
					lpszFile = szTemp;
					if (bChecked)
						SaveLnkOption(TRUE);
				}
				else if (bChecked)
				{
					SaveLnkOption(FALSE);
				}
			}
		}
	}
	SetDlgItemText(IDC_EDIT2, lpszFile);
	::SetForegroundWindow(IsDlgButtonChecked(IDC_CHECK5), m_hWnd);

	OnBtnCalc(0, 0, 0, bCalledHandled);

	return 0;
}

LRESULT CMainDlg::OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	COPYDATASTRUCT *pData = reinterpret_cast<COPYDATASTRUCT *>(lParam);
	if (wParam == COPYDATA_MUTEX && pData->dwData == 1)
	{
		if(::MessageDlg(m_hWnd, IDS_MSGBOX_AERO_TOGGLE,
			MB_ICONEXCLAMATION | MB_OKCANCEL | MB_DEFBUTTON2) == IDOK
			&& !::SwitchAero())
		{
			::MessageDlg(m_hWnd, IDS_MSGBOX_AERO_FAILED, MB_ICONERROR);
		}
	}

	return 0;
}

//浏览文件按钮
LRESULT CMainDlg::OnBtnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DWORD dwFlags = 0;
	CFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		REG_DLG_FILTER, m_hWnd);
	if (dlgOpen.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_EDIT2, dlgOpen.m_ofn.lpstrFile);
		BOOL bHandled = FALSE;
		OnBtnCalc(0, 0, 0, bHandled);
	}

	return 0;
}

LRESULT CMainDlg::OnBtnCalc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	LPSTR szText = new CHAR[MAX_PATH + 1];
	if (szText)
		szText[0] = '\0';

	if (IsDlgButtonChecked(IDC_RADIO1) == TRUE)
	{
		GetDlgItemTextA(m_hWnd, IDC_EDIT2, szText, MAX_PATH);
		if (szText && szText[0] == '\0')
		{
			::MessageDlg(m_hWnd, IDS_MSGBOX_NO_FILE, MB_ICONEXCLAMATION);
			GetDlgItem(IDC_EDIT2).SetFocus();
			return 0;
		}

		HANDLE hFile = ::CreateFileA(szText, FILE_READ_EA,
			FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER liSize;
			if (::GetFileSizeEx(hFile, &liSize))
			{
				m_llSize = liSize.QuadPart;
			}
			else
			{
				::MessageDlg(m_hWnd, IDS_MSGBOX_GET_FILE_SIZE_ER, MB_ICONEXCLAMATION);
				GetDlgItem(IDC_EDIT2).SetFocus();
				::CloseHandle(hFile);
				return 0;
			}

			FILETIME ftCreate;
			FILETIME ftLastAccess;
			FILETIME ftLastWrite;
			if (::GetFileTime(hFile, &ftCreate, &ftLastAccess, &ftLastWrite))
			{
				SYSTEMTIME stFile;
				::memset(&stFile, 0x0, sizeof(stFile));
				::FileTimeToSystemTime(&ftLastWrite, &stFile);
				TIME_ZONE_INFORMATION tzInfo;
				::GetTimeZoneInformation(&tzInfo);
				::SystemTimeToTzSpecificLocalTime(&tzInfo, &stFile, &m_stTime);
			}
			::CloseHandle(hFile);
		}
		else
		{
			::MessageDlg(m_hWnd, IDS_MSGBOX_INVALID_FILE, MB_ICONEXCLAMATION);
			GetDlgItem(IDC_EDIT2).SetFocus();
			return 0;
		}
		m_sThreadData.bFile = TRUE;
	}
	else
	{
		GetDlgItemTextA(m_hWnd, IDC_EDIT3, szText, MAX_PATH);
		size_t sLen = ::strlen(szText);
		if (sLen == 0)
		{
			::MessageDlg(m_hWnd, IDS_MSGBOX_NO_STRING, MB_ICONEXCLAMATION);
			GetDlgItem(IDC_EDIT3).SetFocus();
			return 0;
		}
		m_llSize = static_cast<LONGLONG>(sLen);
		m_sThreadData.bFile = FALSE;
	}

	m_bCancel = FALSE;
	m_sThreadData.hWnd = m_hWnd;
	m_sThreadData.lpszFile = szText;
	m_sThreadData.pnErrCode = &m_nErrCode;
	m_sThreadData.dwType = 0;
	m_sThreadData.pbCancel = &m_bCancel;

	if (IsDlgButtonChecked(IDC_CHECK2) == TRUE)
		m_sThreadData.dwType |= TYPE_MD5;

	if (IsDlgButtonChecked(IDC_CHECK3) == TRUE)
		m_sThreadData.dwType |= TYPE_SHA1;

	if (IsDlgButtonChecked(IDC_CHECK4) == TRUE)
		m_sThreadData.dwType |= TYPE_CRC32;

	if (m_sThreadData.dwType == 0)
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_NO_ALGORITHM, MB_ICONEXCLAMATION);
		GetDlgItem(IDC_CHECK2).SetFocus();
		return 0;
	}

	DragAcceptFiles(FALSE);

	CListBox lsbResult;
	lsbResult.Attach(GetDlgItem(IDC_LIST1));
	lsbResult.ResetContent();
	lsbResult.ShowWindow(SW_HIDE);
	SetDlgItemText(IDC_EDIT1, _T(""));
	m_strCurMd5.Empty();
	m_strCurSha1.Empty();
	m_strCurCrc32.Empty();

	GetDlgItem(IDC_RADIO1).EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO2).EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT1).EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT2).EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT3).EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON1).EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON2).EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON3).EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON4).EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON6).EnableWindow(FALSE);
	GetDlgItem(IDOK).EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK2).EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK3).EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK4).EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO3).EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO4).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_NOTICE).ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON5).ShowWindow(SW_SHOW);

	CProgressBarCtrl pgbProgress;
	pgbProgress.Attach(GetDlgItem(IDC_PROGRESS1));
	pgbProgress.SetPos(0);
	pgbProgress.ShowWindow(SW_SHOW);

	HMENU hSysMenu = GetSystemMenu(FALSE);
	EnableMenuItem(hSysMenu, SC_CLOSE, MF_DISABLED);
	Invalidate();

	m_nErrCode = 0;

	//启动线程
#ifdef _DEBUG
	m_dwStart = ::GetTickCount();
#endif

	m_hThread = reinterpret_cast<HANDLE>(
		::_beginthreadex(NULL, 0, CalcThread,
		reinterpret_cast<LPVOID>(&m_sThreadData), 0, NULL));

	return 0;
}

LRESULT CMainDlg::OnBtnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_bCancel = TRUE;

	return 0;
}

LRESULT CMainDlg::OnBtnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_strCurMd5.IsEmpty() && m_strCurSha1.IsEmpty() && m_strCurCrc32.IsEmpty())
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_NO_RESULT, MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT2).SetFocus();
		return 0;
	}

	// 复制选中文本到剪贴板
	if (::OpenClipboard(m_hWnd))
	{
		::EmptyClipboard();
		HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE,
			(MD5_LEN + SHA1_LEN + CRC32_LEN + 24) * sizeof(TCHAR));
		LPTSTR lpszText = reinterpret_cast<LPTSTR>(::GlobalLock(hMem));

		CString strVerify;
		if (!m_strCurMd5.IsEmpty())
		{
			strVerify = HEAD_MD5_ENG;
			strVerify += _T(" ");
			strVerify += m_strCurMd5;
		}
		if (!m_strCurSha1.IsEmpty())
		{
			if (!strVerify.IsEmpty())
				strVerify += _T("\r\n");
			strVerify += HEAD_SHA1_ENG;
			strVerify += _T(" ");
			strVerify += m_strCurSha1;
		}
		if (!m_strCurCrc32.IsEmpty())
		{
			if (!strVerify.IsEmpty())
				strVerify += _T("\r\n");
			strVerify += HEAD_CRC32_ENG;
			strVerify += _T(" ");
			strVerify += m_strCurCrc32;
		}

		::memcpy(lpszText, strVerify.GetBuffer(),
			(MD5_LEN + SHA1_LEN + CRC32_LEN + 23) * sizeof(TCHAR));
		lpszText[MD5_LEN + SHA1_LEN + CRC32_LEN + 23] = '\0';
		::SetClipboardData(CF_UNICODETEXT, hMem);
		::GlobalUnlock(hMem);
		::CloseClipboard();
	}
	else
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_CLIPBOARD_ERROR, MB_ICONERROR);
	}

	return 0;
}

LRESULT CMainDlg::OnBtnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// 从剪贴板粘贴文本
	if (::OpenClipboard(m_hWnd))
	{
		HGLOBAL hMem = ::GetClipboardData(CF_UNICODETEXT);
		LPTSTR lpwszText = reinterpret_cast<LPTSTR>(::GlobalLock(hMem));
		if (lpwszText != NULL)
		{
			// 添加 UNICODE 文本
			SetDlgItemText(IDC_EDIT1, lpwszText);
		}
		else
		{
			// 添加 ANSI 文本
			hMem = ::GetClipboardData(CF_TEXT);
			LPSTR lpaszText = reinterpret_cast<LPSTR>(::GlobalLock(hMem));
			if (lpaszText != NULL)
			{
				::SetDlgItemTextA(m_hWnd, IDC_EDIT1, lpaszText);
			}
		}
		::GlobalUnlock(hMem);
		::CloseClipboard();
	}
	else
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_CLIPBOARD_ERROR, MB_ICONERROR);
	}

	BOOL bCalledHandled = FALSE;
	OnBtnVerify(0, 0, 0, bCalledHandled);

	return 0;
}

BOOL CMainDlg::VerifyString(CString& strVerify, DWORD& dwErrNum, DWORD& dwVerType)
{
	if (strVerify.Left(4).CompareNoCase(HEAD_MD5_CHS) == 0 ||
		strVerify.Left(4).CompareNoCase(HEAD_MD5_ENG) == 0)
	{
		if (m_strCurMd5.IsEmpty())
			return FALSE;

		strVerify.Delete(0, 4);
		strVerify.Trim();
		if (m_strCurMd5.CompareNoCase(strVerify) != 0)
			dwErrNum |= TYPE_MD5;
		else
			dwVerType |= TYPE_MD5;
		return TRUE;
	}

	if (strVerify.Left(5).CompareNoCase(HEAD_SHA1_CHS) == 0 ||
		strVerify.Left(5).CompareNoCase(HEAD_SHA1_ENG) == 0)
	{
		if (m_strCurSha1.IsEmpty())
			return FALSE;

		strVerify.Delete(0, 5);
		strVerify.Trim();
		if (m_strCurSha1.CompareNoCase(strVerify) != 0)
			dwErrNum |= TYPE_SHA1;
		else
			dwVerType |= TYPE_SHA1;
		return TRUE;
	}

	if (strVerify.Left(6).CompareNoCase(HEAD_CRC32_CHS) == 0 ||
		strVerify.Left(6).CompareNoCase(HEAD_CRC32_ENG) == 0)
	{
		if (m_strCurCrc32.IsEmpty())
			return FALSE;

		strVerify.Delete(0, 6);
		strVerify.Trim();
		if (m_strCurCrc32.CompareNoCase(strVerify) != 0)
			dwErrNum |= TYPE_CRC32;
		else
			dwVerType |= TYPE_CRC32;
		return TRUE;
	}

	return FALSE;
}

BOOL CMainDlg::VerifyString(CString& strVerify, DWORD& dwErrNum, DWORD& dwVerType,
							int& nCount, const int* nSel)
{
	if (!strVerify.IsEmpty())
	{
		strVerify.Trim();
		nCount++;
		if (nCount == nSel[0])
		{
			if (m_strCurMd5.CompareNoCase(strVerify) != 0)
				dwErrNum |= TYPE_MD5;
			else
				dwVerType |= TYPE_MD5;
		}
		else if (nCount == nSel[1])
		{
			if (m_strCurSha1.CompareNoCase(strVerify) != 0)
				dwErrNum |= TYPE_SHA1;
			else
				dwVerType |= TYPE_SHA1;
		}
		else if (nCount == nSel[2])
		{
			if (m_strCurCrc32.CompareNoCase(strVerify) != 0)
				dwErrNum |= TYPE_CRC32;
			else
				dwVerType |= TYPE_CRC32;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

LRESULT CMainDlg::OnBtnVerify(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DWORD dwErrNum = 0, dwVerType = 0;
	int nEscCount = 0, nLineCount = 0;
	if (m_strCurMd5.IsEmpty() && m_strCurSha1.IsEmpty() && m_strCurCrc32.IsEmpty())
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_NEED_CALC, MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT2).SetFocus();
		return 0;
	}

	CString strVerify;
	GetDlgItemText(IDC_EDIT1, strVerify);
	strVerify.Trim();
	if (strVerify.IsEmpty())
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_NOT_EMPTY, MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT1).SetFocus();
		return 0;
	}

	CString strTmp;
	int nMd5Start = 0;
	int nMd5End = strVerify.Find(TEXT_NEW_LINE);
	while (nMd5End != -1)
	{
		nLineCount++;
		strTmp = strVerify.Mid(nMd5Start, nMd5End - nMd5Start);
		if (VerifyString(strTmp, dwErrNum, dwVerType) == FALSE)
		{
			nEscCount++;
		}
		nMd5Start = nMd5End + 2;
		nMd5End = strVerify.Find(TEXT_NEW_LINE, nMd5End + 2);
	}
	nLineCount++;
	strTmp = strVerify.Mid(nMd5Start);
	strTmp.TrimLeft();
	if (VerifyString(strTmp, dwErrNum, dwVerType) == FALSE)
	{
		nEscCount++;
	}

	//不符合格式
	if (nEscCount == nLineCount)
	{
		nEscCount = 0;
		nLineCount = 0;
		int nCount = 0;
		nMd5Start = 0;
		nMd5End = strVerify.Find(TEXT_NEW_LINE);
		int nSel[MAX_SEL] = {-1, -1, -1};
		if (!m_strCurMd5.IsEmpty())
		{
			nSel[0] = 1;
			if (!m_strCurSha1.IsEmpty())
			{
				nSel[1] = 2;
				if (!m_strCurCrc32.IsEmpty())
					nSel[2] = 3;
			}
			else
			{
				if (!m_strCurCrc32.IsEmpty())
					nSel[2] = 2;
			}
		}
		else
		{
			if (!m_strCurSha1.IsEmpty())
			{
				nSel[1] = 1;
				if (!m_strCurCrc32.IsEmpty())
					nSel[2] = 2;
			}
			else
			{
				if (!m_strCurCrc32.IsEmpty())
					nSel[2] = 1;
			}
		}
		while (nMd5End != -1)
		{
			nLineCount++;
			strTmp = strVerify.Mid(nMd5Start, nMd5End - nMd5Start);
			if (VerifyString(strTmp, dwErrNum, dwVerType, nCount, nSel) == FALSE)
			{
				nEscCount++;
			}
			nMd5Start = nMd5End + 2;
			nMd5End = strVerify.Find(TEXT_NEW_LINE, nMd5End + 2);
		}
		strTmp = strVerify.Mid(nMd5Start);
		strTmp.TrimLeft();
		if (VerifyString(strTmp, dwErrNum, dwVerType, nCount, nSel) == FALSE)
		{
			nEscCount++;
		}
	}

	if (dwErrNum == 0)
	{
		if (nEscCount > 0)
		{
			strTmp.Empty();
			if ((dwVerType & TYPE_MD5) == TYPE_MD5)
				strTmp += RESULT_MD5;
			if ((dwVerType & TYPE_SHA1) == TYPE_SHA1)
				strTmp += RESULT_SHA1;
			if ((dwVerType & TYPE_CRC32) == TYPE_CRC32)
				strTmp += RESULT_CRC32;
			::MessageDlg(m_hWnd, IDS_MSGBOX_VERIFY_SUCCESS, strTmp, MB_ICONINFORMATION);
		}
		else
		{
			::MessageDlg(m_hWnd, IDS_MSGBOX_VERIFY_ALL, MB_ICONINFORMATION);
		}
	}
	else
	{
		strTmp.Empty();
		if ((dwErrNum & TYPE_MD5) == TYPE_MD5)
			strTmp += RESULT_MD5;
		if ((dwErrNum & TYPE_SHA1) == TYPE_SHA1)
			strTmp += RESULT_SHA1;
		if ((dwErrNum & TYPE_CRC32) == TYPE_CRC32)
			strTmp += RESULT_CRC32;
		::MessageDlg(m_hWnd, IDS_MSGBOX_VERIFY_FAIL, strTmp, MB_ICONEXCLAMATION);
	}

	return 0;
}

BOOL CMainDlg::QuickMatch(LPCTSTR lpszVerify)
{
	if (m_strCurMd5.IsEmpty() && m_strCurSha1.IsEmpty() && m_strCurCrc32.IsEmpty())
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_NEED_CALC, MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT2).SetFocus();
		return 0;
	}
	
	CString strVerify = lpszVerify;
	strVerify.Trim();

	if (m_strCurMd5.CompareNoCase(strVerify) == 0)
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_SAME_RESULT, ALGORITHM_MD5, MB_ICONINFORMATION);
		return TRUE;
	}

	if (m_strCurSha1.CompareNoCase(strVerify) == 0)
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_SAME_RESULT, ALGORITHM_SHA1, MB_ICONINFORMATION);
		return TRUE;
	}

	if (m_strCurCrc32.CompareNoCase(strVerify) == 0)
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_SAME_RESULT, ALGORITHM_CRC32, MB_ICONINFORMATION);
		return TRUE;
	}

	::MessageDlg(m_hWnd, IDS_MSGBOX_DIFF_RESULT, MB_ICONWARNING);

	return FALSE;
}

LRESULT CMainDlg::OnBtnQuickMatch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// 从剪贴板粘贴文本
	if (::OpenClipboard(m_hWnd))
	{
		HGLOBAL hMem = ::GetClipboardData(CF_UNICODETEXT);
		LPTSTR lpwszText = reinterpret_cast<LPTSTR>(::GlobalLock(hMem));
		if (lpwszText != NULL)
		{
			QuickMatch(lpwszText);
		}
		else
		{
			// 添加 ANSI 文本
			hMem = ::GetClipboardData(CF_TEXT);
			LPSTR lpaszText = reinterpret_cast<LPSTR>(::GlobalLock(hMem));
			if (lpaszText != NULL)
			{
				int nLen = static_cast<int>(::strlen(lpaszText));
				int nSize = ::MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<LPCSTR>(lpaszText), nLen, NULL, 0);
				lpwszText = reinterpret_cast<LPTSTR>(::malloc(nSize * sizeof(TCHAR)));
				::MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<LPCSTR>(lpaszText), nLen, lpwszText, nSize);
				QuickMatch(lpwszText);
				::free(lpwszText);
			}
			else
			{
				::MessageDlg(m_hWnd, IDS_MSGBOX_CLIPBOARD_EMPTY, MB_ICONWARNING);
			}
		}
		::GlobalUnlock(hMem);
		::CloseClipboard();
	}
	else
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_CLIPBOARD_ERROR, MB_ICONERROR);
	}

	return 0;
}

LRESULT CMainDlg::SetMenuItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!m_bAdmin)
	{
		CheckDlgButton(IDC_CHECK1, !IsDlgButtonChecked(IDC_CHECK1));

		if (m_uWinVer >= WINVER_VISTA)
		{
			CPrivilegeNewDlg dlgPrivilege;
			dlgPrivilege.DoModal(m_hWnd);
		}
		else
		{
			CPrivilegeDlg dlgPrivilege;
			dlgPrivilege.DoModal(m_hWnd);
		}
	}
	else
	{
		if (IsDlgButtonChecked(IDC_CHECK1))
		{
			if (!::AddRightButtonMenu(m_hWnd))
				::RemoveRightButtonMenu(m_hWnd, FALSE);
		}
		else
		{
			::RemoveRightButtonMenu(m_hWnd);
		}
	}

	return 0;
}

LRESULT CMainDlg::OnCalcProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandle)
{
	if (wParam == MSG_CALC_SIGN)
	{
		int nPos = static_cast<int>(lParam);

		CProgressBarCtrl pgbProgress;
		pgbProgress.Attach(GetDlgItem(IDC_PROGRESS1));
		pgbProgress.SetPos(nPos);

		if (m_pTaskbarList != NULL)
		{
			m_pTaskbarList->SetProgressValue(m_hWnd, nPos, 100);
		}
	}

	return 0;
}

//处理计算线程返回消息
LRESULT CMainDlg::OnCalcStop(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandle)
{
#ifdef _DEBUG
	DWORD dwEnd = ::GetTickCount();
	CString strLen;
	strLen.Format(_T("%u"), dwEnd - m_dwStart);
	::OutputDebugString(strLen);
	::OutputDebugString(_T("\n"));
#endif

	CListBox lsbResult;
	lsbResult.Attach(GetDlgItem(IDC_LIST1));
	lsbResult.DeleteString(0);

	//从参数得到加密结果
	if (int(wParam) == 0)	//完成
	{
		CString strLoad;
		if (IsDlgButtonChecked(IDC_RADIO1) == TRUE)
		{
			CString strTmp, strByteSize, strFileSize;
			strLoad.LoadString(IDS_LIST_TYPE_FILE);
			strTmp.Format(strLoad,
				m_stTime.wYear, m_stTime.wMonth, m_stTime.wDay,
				m_stTime.wHour, m_stTime.wMinute, m_stTime.wSecond);
			lsbResult.InsertString(0, strTmp);
			strLoad.LoadString(IDS_LIST_SIZE);
			::GetNumberSubsectionString(m_llSize, 3, strByteSize);
			::GetFileSizeTextString(m_llSize, strFileSize);
			strTmp.Format(strLoad, strByteSize, strFileSize);
			lsbResult.InsertString(1, strTmp);
		}
		else
		{
			strLoad.LoadString(IDS_LIST_TYPE_STRING);
			lsbResult.InsertString(0, strLoad);
			CString strSize, strByteSize, strFileSize;
			::GetNumberSubsectionString(m_llSize, 3, strByteSize);
			::GetFileSizeTextString(m_llSize, strFileSize);
			strLoad.LoadString(IDS_LIST_SIZE);
			strSize.Format(strLoad, strByteSize, strFileSize);
			lsbResult.InsertString(1, strSize);
		}
		LPCTSTR lpszResult = reinterpret_cast<LPCTSTR>(lParam);
		if (m_bCancel == TRUE)
		{
			strLoad.LoadString(IDS_LIST_CANCEL);
			lsbResult.InsertString(2, strLoad);
		}
		else
		{
			CString strMd5 = lpszResult;
			if (IsDlgButtonChecked(IDC_RADIO3) == TRUE)
				strMd5.MakeUpper();
			else
				strMd5.MakeLower();

			int nMd5End = 0;
			CString strTmp;
			if (IsDlgButtonChecked(IDC_CHECK2) == TRUE)
			{
				nMd5End = strMd5.Find(TEXT_ENTER_KEY);
				if (nMd5End == -1)
					nMd5End = strMd5.GetLength();
				m_strCurMd5 = strMd5.Left(nMd5End);
				strTmp = HEAD_MD5_ENG;
				strTmp += _T(" ");
				strTmp += m_strCurMd5;
				lsbResult.InsertString(2, strTmp);
			}
			if (IsDlgButtonChecked(IDC_CHECK3) == TRUE)
			{
				int nMd5End2 = strMd5.Find(TEXT_ENTER_KEY, nMd5End + 1);
				if (nMd5End2 == -1)
					m_strCurSha1 = strMd5.Mid(nMd5End + 1);
				else
					m_strCurSha1 = strMd5.Mid(nMd5End + 1, nMd5End2 - nMd5End - 1);
				nMd5End = nMd5End2;
				strTmp = HEAD_SHA1_ENG;
				strTmp += _T(" ");
				strTmp += m_strCurSha1;
				lsbResult.InsertString(lsbResult.GetCount(), strTmp);
			}
			if (IsDlgButtonChecked(IDC_CHECK4) == TRUE)
			{
				m_strCurCrc32 = strMd5.Mid(nMd5End + 1);
				strTmp = HEAD_CRC32_ENG;
				strTmp += _T(" ");
				strTmp += m_strCurCrc32;
				lsbResult.InsertString(lsbResult.GetCount(), strTmp);
			}
		}
		delete []lpszResult;
	}
	else	//失败
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_CANNOT_READ_FILE, MB_ICONERROR);
	}
	//清理工作
	::CloseHandle(m_hThread);
	m_hThread = NULL;
	
	//更新界面
	DragAcceptFiles(TRUE);

	GetDlgItem(IDC_RADIO1).EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO2).EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT1).EnableWindow(TRUE);
	if (IsDlgButtonChecked(IDC_RADIO1) == TRUE)
	{
		GetDlgItem(IDC_EDIT2).EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON2).EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_EDIT3).EnableWindow(TRUE);
	}
	GetDlgItem(IDC_BUTTON1).EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON3).EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON4).EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON6).EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK2).EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK3).EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK4).EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO3).EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO4).EnableWindow(TRUE);
	GetDlgItem(IDOK).EnableWindow(TRUE);
	GetDlgItem(IDC_LIST1).ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON5).ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PROGRESS1).ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_NOTICE).ShowWindow(SW_HIDE);

	if (m_pTaskbarList != NULL)
	{
		m_pTaskbarList->SetProgressState(m_hWnd, TBPF_NOPROGRESS);
	}

	HMENU hSysMenu = GetSystemMenu(FALSE);
	if (hSysMenu != NULL)
	{
		EnableMenuItem(hSysMenu, SC_CLOSE, MF_ENABLED);
	}
	Invalidate();

	return 0;
}

LRESULT CMainDlg::OnUpper(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CListBox lsbResult;
	lsbResult.Attach(GetDlgItem(IDC_LIST1));
	for (int nIndex = lsbResult.GetCount(); nIndex >= 2; nIndex--)
		lsbResult.DeleteString(nIndex);

	CString strTmp;
	if (!m_strCurMd5.IsEmpty())
	{
		m_strCurMd5.MakeUpper();
		strTmp = HEAD_MD5_ENG;
		strTmp += _T(" ");
		strTmp += m_strCurMd5;
		lsbResult.InsertString(lsbResult.GetCount(), strTmp);
	}

	if (!m_strCurSha1.IsEmpty())
	{
		m_strCurSha1.MakeUpper();
		strTmp = HEAD_SHA1_ENG;
		strTmp += _T(" ");
		strTmp += m_strCurSha1;
		lsbResult.InsertString(lsbResult.GetCount(), strTmp);
	}

	if (!m_strCurCrc32.IsEmpty())
	{
		m_strCurCrc32.MakeUpper();
		strTmp = HEAD_CRC32_ENG;
		strTmp += _T(" ");
		strTmp += m_strCurCrc32;
		lsbResult.InsertString(lsbResult.GetCount(), strTmp);
	}

	return 0;
}


LRESULT CMainDlg::OnLower(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CListBox lsbResult;
	lsbResult.Attach(GetDlgItem(IDC_LIST1));
	for (int nIndex = lsbResult.GetCount(); nIndex >= 2; nIndex--)
		lsbResult.DeleteString(nIndex);

	CString strTmp;
	if (!m_strCurMd5.IsEmpty())
	{
		m_strCurMd5.MakeLower();
		strTmp = HEAD_MD5_ENG;
		strTmp += _T(" ");
		strTmp += m_strCurMd5;
		lsbResult.InsertString(lsbResult.GetCount(), strTmp);
	}

	if (!m_strCurSha1.IsEmpty())
	{
		m_strCurSha1.MakeLower();
		strTmp = HEAD_SHA1_ENG;
		strTmp += _T(" ");
		strTmp += m_strCurSha1;
		lsbResult.InsertString(lsbResult.GetCount(), strTmp);
	}

	if (!m_strCurCrc32.IsEmpty())
	{
		m_strCurCrc32.MakeLower();
		strTmp = HEAD_CRC32_ENG;
		strTmp += _T(" ");
		strTmp += m_strCurCrc32;
		lsbResult.InsertString(lsbResult.GetCount(), strTmp);
	}

	return 0;
}

LRESULT CMainDlg::SetTopMost(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IsDlgButtonChecked(IDC_CHECK5) == TRUE)
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE |SWP_NOSIZE);
	else
		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE |SWP_NOSIZE);

	return 0;
}

void CMainDlg::SaveLnkOption(BOOL bYes)
{
	TCHAR szFilePath[MAX_PATH + 1] = {0};
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);
	::PathRemoveFileSpec(szFilePath);
	::PathAppend(szFilePath, CONFIG_FILE_NAME);

	if (bYes)
		m_nLnkOption = 1;
	else
		m_nLnkOption = 2;

	TCHAR szValue[5];
	::_itot_s(m_nLnkOption, szValue, 10);
	::WritePrivateProfileString(CONFIG_ROOT, CONFIG_ITEM_LNKOPTION, szValue, szFilePath);
}

//加载配置
BOOL CMainDlg::LoadConfig()
{
	TCHAR szFilePath[MAX_PATH + 1] = {0};
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);
	::PathRemoveFileSpec(szFilePath);
	::PathAppend(szFilePath, CONFIG_FILE_NAME);

	DWORD dwType = static_cast<DWORD>(::GetPrivateProfileInt(CONFIG_ROOT, CONFIG_ITEM_HASH,
		TYPE_MD5 | TYPE_SHA1 | TYPE_CRC32, szFilePath));
	if ((dwType & TYPE_MD5) == TYPE_MD5)
		CheckDlgButton(IDC_CHECK2, TRUE);
	if ((dwType & TYPE_SHA1) == TYPE_SHA1)
		CheckDlgButton(IDC_CHECK3, TRUE);
	if ((dwType & TYPE_CRC32) == TYPE_CRC32)
		CheckDlgButton(IDC_CHECK4, TRUE);

	BOOL bTopMost = static_cast<BOOL>(::GetPrivateProfileInt(CONFIG_ROOT, CONFIG_ITEM_TOPMOST, 0, szFilePath));
	if (bTopMost)
	{
		CheckDlgButton(IDC_CHECK5, TRUE);
		SetTopMost(0, 0, 0, bTopMost);
	}

	BOOL bUpper = static_cast<BOOL>(::GetPrivateProfileInt(CONFIG_ROOT, CONFIG_ITEM_UPPER, 1, szFilePath));
	if (bUpper)
		CheckRadioButton(IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
	else
		CheckRadioButton(IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);

	m_nLnkOption = ::GetPrivateProfileInt(CONFIG_ROOT, CONFIG_ITEM_LNKOPTION, 0, szFilePath);

	BOOL bNoAero = static_cast<BOOL>(::GetPrivateProfileInt(CONFIG_ROOT, CONFIG_ITEM_NOAERO, 0, szFilePath));
	return !bNoAero;
}

//加载配置
void CMainDlg::SaveConfig()
{
	TCHAR szFilePath[MAX_PATH + 1] = {0};
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);
	::PathRemoveFileSpec(szFilePath);
	::PathAppend(szFilePath, CONFIG_FILE_NAME);

	TCHAR szValue[5];
	DWORD dwType = 0;
	if (IsDlgButtonChecked(IDC_CHECK2) == TRUE)
		dwType |= TYPE_MD5;
	if (IsDlgButtonChecked(IDC_CHECK3) == TRUE)
		dwType |= TYPE_SHA1;
	if (IsDlgButtonChecked(IDC_CHECK4) == TRUE)
		dwType |= TYPE_CRC32;
	::_ultot_s(dwType, szValue, 10);
	::WritePrivateProfileString(CONFIG_ROOT, CONFIG_ITEM_HASH, szValue, szFilePath);

	BOOL bTopMost = IsDlgButtonChecked(IDC_CHECK5);
	::_itot_s(bTopMost, szValue, 10);
	::WritePrivateProfileString(CONFIG_ROOT, CONFIG_ITEM_TOPMOST, szValue, szFilePath);

	BOOL bUpper = IsDlgButtonChecked(IDC_RADIO3);
	::_itot_s(bUpper, szValue, 10);
	::WritePrivateProfileString(CONFIG_ROOT, CONFIG_ITEM_UPPER, szValue, szFilePath);
}

LRESULT CMainDlg::OnCopyItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CListBox lsbResult;
	lsbResult.Attach(GetDlgItem(IDC_LIST1));
	int nIndex = lsbResult.GetCurSel();
	if (nIndex == -1)
		return 0;

	// 复制选中文本到剪贴板
	if (::OpenClipboard(m_hWnd))
	{
		::EmptyClipboard();
		HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE,
			(MD5_LEN + SHA1_LEN + CRC32_LEN + 23) * sizeof(TCHAR));
		LPTSTR lpszText = reinterpret_cast<LPTSTR>(::GlobalLock(hMem));
		lsbResult.GetText(nIndex, lpszText);
		::SetClipboardData(CF_UNICODETEXT, hMem);
		::GlobalUnlock(hMem);
		::CloseClipboard();

		::MessageDlg(m_hWnd, IDS_MSGBOX_CONTENT_COPIED, MB_ICONINFORMATION);
	}
	else
	{
		::MessageDlg(m_hWnd, IDS_MSGBOX_CLIPBOARD_ERROR, MB_ICONERROR);
	}

	return 0;
}
