#include "stdafx.h"
#include "PrivilegeNewDlg.h"
#include "winfunc\WinFunc.h"
#include "StringDefine.h"
#include "resource.h"


CPrivilegeNewDlg::CPrivilegeNewDlg()
{
	SetWindowTitle(IDS_PRIVILEGE_MORE);
	SetMainInstructionText(IDS_PRIVILEGE_NEED);
	SetMainIcon(TD_SHIELD_ICON);
	SetExpandedInformationText(IDS_PRIVILEGE_EXTENDED);

	ModifyFlags(0, TDF_ALLOW_DIALOG_CANCELLATION | 
		TDF_USE_COMMAND_LINKS);
}

void CPrivilegeNewDlg::OnDialogConstructed()
{
	SetButtonElevationRequiredState(Button_Elevate, true);
}

bool CPrivilegeNewDlg::OnButtonClicked(int buttonId)
{
	switch (buttonId)
	{
	case Button_Elevate:
		{
			EnableButton(Button_Elevate, FALSE);
			EnableButton(Button_Cancel, FALSE);
			HMENU hSysMenu = ::GetSystemMenu(m_hWnd, FALSE);
			::EnableMenuItem(hSysMenu, SC_CLOSE, MF_DISABLED);
			if (::PrivilegeElevate(ARG_SET_MENU, 1))
			{
				::CheckDlgButton(m_tdc.hwndParent, IDC_CHECK1,
					!::IsDlgButtonChecked(m_tdc.hwndParent, IDC_CHECK1));
			}
			break;
		}
	case Button_Cancel:
	case IDCANCEL:
		{
			break;
		}
	default:
		{
			ATLASSERT(false);
		}
	}

	return false;
}

int CPrivilegeNewDlg::DoModal(HWND hWnd)
{
	LPCTSTR lpszFormat = _T("%s\n%s");
	CString strText, strText2, strText3, str1, str2;
	str1.LoadString(IDS_PRIVILEGE_DESC);
	str2.LoadString(IDS_PRIVILEGE_TIP1);
	strText.Format(lpszFormat, str1, str2);
	SetContentText(strText);
	str1.LoadString(IDS_PRIVILEGE_CMD1);
	str2.LoadString(IDS_PRIVILEGE_ELEVATE);
	strText2.Format(lpszFormat, str1, str2);
	str1.LoadString(IDS_PRIVILEGE_CMD2);
	str2.LoadString(IDS_PRIVILEGE_CANCEL);
	strText3.Format(lpszFormat, str1, str2);

	const TASKDIALOG_BUTTON buttons[] =
	{
		{ Button_Elevate, strText2 },
		{ Button_Cancel, strText3 },
	};

	SetButtons(buttons,
		_countof(buttons));

	CTaskDialogImpl::DoModal(hWnd);

	return 0;
}

