// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CalcFunc.h"
#include "MessageDlg.h"

#define	MAX_SEL				3
#define	TIMER_ID			512
#define	TIMER_WAIT			500


class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
	CMainDlg();
	~CMainDlg();

	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
		MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CALC_PROGRESS, OnCalcProgress)
		MESSAGE_HANDLER(WM_CALC_STOP, OnCalcStop)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDOK, OnBtnVerify)
		COMMAND_ID_HANDLER(IDC_RADIO1, OnFile)
		COMMAND_ID_HANDLER(IDC_RADIO2, OnString)
		COMMAND_ID_HANDLER(IDC_RADIO3, OnUpper)
		COMMAND_ID_HANDLER(IDC_RADIO4, OnLower)
		COMMAND_ID_HANDLER(IDC_BUTTON1, OnBtnPaste)
		COMMAND_ID_HANDLER(IDC_BUTTON2, OnBtnBrowse)
		COMMAND_ID_HANDLER(IDC_BUTTON3, OnBtnCalc)
		COMMAND_ID_HANDLER(IDC_BUTTON4, OnBtnCopy)
		COMMAND_ID_HANDLER(IDC_BUTTON5, OnBtnStop)
		COMMAND_ID_HANDLER(IDC_BUTTON6, OnBtnQuickMatch)
		COMMAND_ID_HANDLER(IDC_CHECK1, SetMenuItem)
		COMMAND_ID_HANDLER(IDC_CHECK5, SetTopMost)
		COMMAND_HANDLER(IDC_LIST1, LBN_DBLCLK, OnCopyItem)
	END_MSG_MAP()

protected:
	CString m_strCurMd5;
	CString m_strCurSha1;
	CString m_strCurCrc32;
	LONGLONG m_llSize;
	BOOL m_bCancel;

	INT m_nErrCode;
	UINT m_nLnkOption;
	SYSTEMTIME m_stTime;
	BOOL m_bUseAero;
	BOOL m_bAeroConfig;
	BOOL m_bAdmin;
	UINT16 m_uWinVer;

	//Ïß³Ì
	HANDLE m_hThread;
	ThreadData m_sThreadData;

	ITaskbarList3* m_pTaskbarList;

#ifdef _DEBUG
	DWORD m_dwStart;
#endif

public:
	BOOL m_bUnique;
	LPCTSTR m_lpszCmdLine;

protected:
	BOOL LoadConfig();
	void SaveConfig();
	void SaveLnkOption(BOOL bYes);
	void InitControls(BOOL bUseAero);
	BOOL HandleCmdLineArgs(BOOL& bUseAero);
	void HandleCmdLineFiles();
	void SetTaskCommand();
	BOOL QuickMatch(LPCTSTR lpszVerify);
	BOOL VerifyString(CString& strVerify, DWORD& dwErrNum, DWORD& dwVerType);
	BOOL VerifyString(CString& strVerify, DWORD& dwErrNum, DWORD& dwVerType, int& nCount, const int* nSel);
	LRESULT OnCalcProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandle);
	LRESULT OnCalcStop(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandle);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBtnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBtnQuickMatch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnString(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnUpper(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnLower(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBtnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBtnCalc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBtnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBtnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBtnVerify(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT SetMenuItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT SetTopMost(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopyItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};