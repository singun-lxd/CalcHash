#include "stdafx.h"
#include "StringDefine.h"
#include "MessageDlg.h"
#include "resource.h"

BOOL g_bUseTaskDlg = FALSE;

int MessageDlg(HWND hWnd, UINT nId, UINT nType, BOOL* pbChecked)
{
	int nReturn = 0;

	CString strMsgbox;
	strMsgbox.LoadString(nId);
	if (g_bUseTaskDlg)
	{
		MessageTaskDlg dlgMessageTask;
		nReturn = dlgMessageTask.DoModal(hWnd, strMsgbox, nType, pbChecked);
	}
	else
	{
		CString strTitle;
		strTitle.LoadString(IDS_CALC_HASH_TITLE);
		nReturn = ::MessageBox(hWnd, strMsgbox, strTitle, nType);
	}
	return nReturn;
}

int MessageDlg(HWND hWnd, UINT nId, UINT nIdAppend, UINT nType, BOOL* pbChecked)
{
	CString strMsgbox, str1, str2;
	str1.LoadString(nId);
	str2.LoadString(nIdAppend);
	strMsgbox.Format(REG_APPEND_FORMAT, str1, str2);

	int nReturn = 0;
	if (g_bUseTaskDlg)
	{
		MessageTaskDlg dlgMessageTask;
		nReturn = dlgMessageTask.DoModal(hWnd, strMsgbox, nType, pbChecked);
	}
	else
	{
		CString strTitle;
		strTitle.LoadString(IDS_CALC_HASH_TITLE);
		nReturn = ::MessageBox(hWnd, strMsgbox, strTitle, nType);
	}
	return nReturn;
}

int MessageDlg(HWND hWnd, UINT nId, LPCTSTR strAppend, UINT nType, BOOL* pbChecked)
{
	CString strMsgbox, strFormat;
	strFormat.LoadString(nId);
	strMsgbox.Format(strFormat, strAppend);

	int nReturn = 0;
	if (g_bUseTaskDlg)
	{
		MessageTaskDlg dlgMessageTask;
		nReturn = dlgMessageTask.DoModal(hWnd, strMsgbox, nType, pbChecked);
	}
	else
	{
		CString strTitle;
		strTitle.LoadString(IDS_CALC_HASH_TITLE);
		nReturn = ::MessageBox(hWnd, strMsgbox, strTitle, nType);
	}
	return nReturn;
}

MessageTaskDlg::MessageTaskDlg()
{
	m_nReturn = 0;
	m_bNotAsk = FALSE;

	m_strTitle.LoadString(IDS_CALC_HASH_TITLE);
	SetWindowTitle(m_strTitle);
}

BOOL MessageTaskDlg::OnButtonClicked(int buttonId)
{
	m_nReturn = buttonId;

	ModifyFlags(0, TDF_ALLOW_DIALOG_CANCELLATION);

	return FALSE;
}

void MessageTaskDlg::OnVerificationClicked(bool bChecked)
{
	if (bChecked)
		m_bNotAsk = TRUE;
	else
		m_bNotAsk = FALSE;
}

int MessageTaskDlg::DoModal(HWND hWnd, LPCTSTR lpszText, UINT nType, BOOL* pbChecked)
{
 	SetButton(nType);
	SetIcon(nType);
	SetMainInstructionText(lpszText);
	if (pbChecked != NULL)
	{
		ModifyFlags(TDF_VERIFICATION_FLAG_CHECKED, 0);
		SetVerificationText(IDS_LNK_NOT_ASK_ME);
	}

	CTaskDialogImpl::DoModal(hWnd, NULL, NULL, pbChecked);

	return m_nReturn;
}

void MessageTaskDlg::SetButton(UINT nType)
{
	if ((nType & MB_OKCANCEL) == MB_OKCANCEL)
	{
		SetCommonButtons(TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON);
	}
	else if ((nType & MB_YESNO) == MB_YESNO)
	{
		SetCommonButtons(TDCBF_YES_BUTTON | TDCBF_NO_BUTTON);
		ModifyFlags(TDF_ALLOW_DIALOG_CANCELLATION, 0);
	}
	else if ((nType & MB_YESNOCANCEL) == MB_YESNOCANCEL)
	{
		SetCommonButtons(TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON);
	}
	else if ((nType & MB_RETRYCANCEL) == MB_RETRYCANCEL)
	{
		SetCommonButtons(TDCBF_RETRY_BUTTON | TDCBF_CANCEL_BUTTON);
	}

	if ((nType & MB_DEFBUTTON2) == MB_DEFBUTTON2)
	{
		SetDefaultButton(2);
	}
	else if ((nType & MB_DEFBUTTON3) == MB_DEFBUTTON3)
	{
		SetDefaultButton(3);
	}
	else if ((nType & MB_DEFBUTTON4) == MB_DEFBUTTON4)
	{
		SetDefaultButton(4);
	}
}

void MessageTaskDlg::SetIcon(UINT nType)
{
	if ((nType & MB_ICONINFORMATION) == MB_ICONINFORMATION)
	{
		SetMainIcon(TD_INFORMATION_ICON);
	}
	else if ((nType & MB_ICONEXCLAMATION) == MB_ICONEXCLAMATION)
	{
		SetMainIcon(TD_WARNING_ICON);
	}
	else if ((nType & MB_ICONERROR) == MB_ICONERROR)
	{
		SetMainIcon(TD_ERROR_ICON);
	}
	else if ((nType & MB_ICONQUESTION) == MB_ICONQUESTION)
	{
		SetMainIcon(TD_SHIELD_ICON);
	}
}
