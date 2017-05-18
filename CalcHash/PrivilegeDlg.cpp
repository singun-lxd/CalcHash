#include "stdafx.h"
#include "resource.h"
#include "PrivilegeDlg.h"
#include "StringDefine.h"

#ifndef BCM_SETSHIELD
#define BCM_SETSHIELD            (BCM_FIRST + 0x000C)
#endif

CPrivilegeDlg::CPrivilegeDlg()
{
	m_hWndParent = NULL;
}

CPrivilegeDlg::~CPrivilegeDlg()
{

}

LRESULT CPrivilegeDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//窗口居中
	CenterWindow();

	CButton btnAdmin;
	btnAdmin.Attach(GetDlgItem(IDOK));
	btnAdmin.SendMessage(BCM_SETSHIELD, 0, TRUE);

	CString strText, str1, str2, str3;
	strText.LoadString(IDS_PRIVILEGE_MORE);
	SetWindowText(strText);
	strText.LoadString(IDS_PRIVILEGE_NEED);
	GetDlgItem(IDC_STATIC_MAIN).SetWindowText(strText);
	strText.LoadString(IDS_PRIVILEGE_EXTENDED);
	GetDlgItem(IDC_STATIC_EXTENDED).SetWindowText(strText);
	str1.LoadString(IDS_PRIVILEGE_DESC);
	str2.LoadString(IDS_PRIVILEGE_TIP2);
	str3.LoadString(IDS_PRIVILEGE_CANCEL2);
	strText.Format(_T("* %s\n[ %s ]\n\n* %s"),	str1, str2, str3);
	GetDlgItem(IDC_STATIC_DESC).SetWindowText(strText);
	strText.LoadString(IDS_PRIVILEGE_CMD1);
	GetDlgItem(IDOK).SetWindowText(strText);
	strText.LoadString(IDS_PRIVILEGE_CMD2);
	GetDlgItem(IDCANCEL).SetWindowText(strText);

	return TRUE;
}

LRESULT CPrivilegeDlg::OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//只处理特定的STATIc
	HDC hDC = (HDC)wParam;
	HWND hWnd = (HWND)lParam;
	if(hWnd == GetDlgItem(IDC_STATIC_MAIN))
	{
		::SetTextColor(hDC, RGB(0, 0, 255));
		::SetBkMode(hDC, TRANSPARENT);
		return (LRESULT)GetStockObject(NULL_BRUSH);
	}
	else if (hWnd == GetDlgItem(IDC_STATIC_EXTENDED))
	{
		::SetTextColor(hDC, RGB(255, 0, 0));
		::SetBkMode(hDC, TRANSPARENT);
		return (LRESULT)GetStockObject(NULL_BRUSH);
	}

	SetMsgHandled(FALSE);

	return 0;
}

LRESULT CPrivilegeDlg::DoModal(HWND hWnd)
{
	m_hWndParent = hWnd;
	CDialogImpl::DoModal();

	return 0;
}

LRESULT CPrivilegeDlg::OnElevate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GetDlgItem(IDOK).EnableWindow(FALSE);
	GetDlgItem(IDCANCEL).EnableWindow(FALSE);
	HMENU hSysMenu = ::GetSystemMenu(m_hWnd, FALSE);
	::EnableMenuItem(hSysMenu, SC_CLOSE, MF_DISABLED);
	if (::PrivilegeElevate(ARG_SET_MENU, 1))
	{
		::CheckDlgButton(m_hWndParent, IDC_CHECK1,
			!::IsDlgButtonChecked(m_hWndParent, IDC_CHECK1));
	}

	EndDialog(IDOK);

	return 0;
}

LRESULT CPrivilegeDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);

	return 0;
}

