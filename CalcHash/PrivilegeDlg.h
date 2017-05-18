// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "WinFunc.h"

class CPrivilegeDlg : public CDialogImpl<CPrivilegeDlg>
{
public:
	CPrivilegeDlg();
	~CPrivilegeDlg();

	enum { IDD = IDD_PRIVILEGE };

	BEGIN_MSG_MAP(CPrivilegeDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDOK, OnElevate)
	END_MSG_MAP()

protected:
	HWND m_hWndParent;

public:
	LRESULT DoModal(HWND hWnd);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnElevate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

};
