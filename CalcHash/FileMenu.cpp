#include "stdafx.h"
#include "resource.h"
#include "StringDefine.h"
#include "MessageDlg.h"

BOOL CheckMenuAvailable(HWND hWnd)
{
	// 右键菜单选项
	HKEY hKey;
	CString strMenuItem, strMenu;
	strMenu.LoadString(IDS_RIGHT_CLICK_MENU);
	strMenuItem.Format(REG_MENU_ITEM, strMenu);
	long lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, strMenuItem, NULL, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS)
	{
		::RegCloseKey(hKey);
		if (hWnd != NULL)
			::CheckDlgButton(hWnd, IDC_CHECK1, TRUE);

		return TRUE;
	}

	return FALSE;
}

BOOL AddRightButtonMenu(HWND hWnd, BOOL bUINotify)
{
	CString strMenuItem, strMenu;
	strMenu.LoadString(IDS_RIGHT_CLICK_MENU);
	strMenuItem.Format(REG_MENU_ITEM, strMenu);
	HKEY hResult;
	TCHAR szPath[MAX_PATH];
	long lValue = 0;
	::GetModuleFileName(NULL, szPath, MAX_PATH);

	CString strKeyValue;
	strKeyValue.Format(REG_MENU_FORMAT, szPath);

	DWORD dwPos = 0;
	//创建“校验哈希值”子键
	lValue = ::RegCreateKeyEx(HKEY_CLASSES_ROOT, strMenuItem, NULL, NULL,
		REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY | KEY_ALL_ACCESS,
		NULL, &hResult, &dwPos);
	if (lValue == ERROR_SUCCESS)
	{
		//将该子键的“默认”数据项的数据设置为“校验哈希值”
		lValue = ::RegSetValueEx(hResult, NULL, 0, REG_SZ,
			(const BYTE*)(LPCTSTR)strMenu, MAX_PATH);
		if (lValue != ERROR_SUCCESS)
		{
			if (bUINotify)
			{
				::MessageDlg(hWnd, IDS_MSGBOX_REGSET_ERROR, IDS_MSGBOX_REG_ERROR, MB_ICONERROR);
				if (hWnd != NULL)
					::CheckDlgButton(hWnd, IDC_CHECK1, FALSE);
			}
			return FALSE;
		}
		//添加菜单图标
		lValue = ::RegSetValueEx(hResult, REG_MENU_ICON, NULL, REG_EXPAND_SZ,
			(const BYTE*)szPath, MAX_PATH);
		::RegCloseKey(hResult);
		if (lValue != ERROR_SUCCESS)
		{
			if (bUINotify)
			{
				::MessageDlg(hWnd, IDS_MSGBOX_REGICON_ERROR, IDS_MSGBOX_REG_ERROR, MB_ICONERROR);
			}
			return FALSE;
		}
	}
	else
	{
		if (bUINotify)
		{
			::MessageDlg(hWnd, IDS_MSGBOX_REGCREATE_ERROR, IDS_MSGBOX_REG_ERROR, MB_ICONERROR);
			if (hWnd != NULL)
				::CheckDlgButton(hWnd, IDC_CHECK1, FALSE);
		}
		return FALSE;
	}
	//创建“command”子键
	CString strMenuFull;
	strMenuFull.Format(REG_MENU_FULL, strMenu);
	lValue = ::RegCreateKeyEx(HKEY_CLASSES_ROOT, strMenuFull, NULL, NULL,
		REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY | KEY_ALL_ACCESS, NULL,
		&hResult, &dwPos);   
	if (lValue == ERROR_SUCCESS)
	{
		//将该子键的“默认”数据项的数据设置为用户应用程序的路径   
		lValue = ::RegSetValueEx(hResult, NULL, NULL, REG_SZ,
			(const BYTE*)(LPCTSTR)strKeyValue, MAX_PATH);
		::RegCloseKey(hResult);
		if (lValue != ERROR_SUCCESS)
		{
			if (bUINotify)
			{
				::MessageDlg(hWnd, IDS_MSGBOX_REGEXE_ERROR, IDS_MSGBOX_REG_ERROR, MB_ICONERROR);
				if (hWnd != NULL)
					::CheckDlgButton(hWnd, IDC_CHECK1, FALSE);
			}
			return FALSE;
		}
	}
	else
	{
		if (bUINotify)
		{
			::MessageDlg(hWnd, IDS_MSGBOX_REGCREATE_ERROR, IDS_MSGBOX_REG_ERROR, MB_ICONERROR);
			if (hWnd != NULL)
				::CheckDlgButton(hWnd, IDC_CHECK1, FALSE);
		}
		return FALSE;
	}

	return TRUE;
}

BOOL RemoveRightButtonMenu(HWND hWnd, BOOL bUINotify)
{
	CString strMenuItem, strMenu;
	strMenu.LoadString(IDS_RIGHT_CLICK_MENU);
	strMenuItem.Format(REG_MENU_ITEM, strMenu);

	HKEY hKey;
	long lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, strMenuItem, NULL, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS)
	{
		//删除“command”子键
		lRet = ::RegDeleteKey(hKey, REG_MENU_COMMAND);
		if (lRet != ERROR_SUCCESS)
		{
			if (bUINotify)
			{
				::MessageDlg(hWnd, IDS_MSGBOX_REGDEL_ERROR, IDS_MSGBOX_REG_ERROR, MB_ICONERROR);
				if (hWnd != NULL)
					::CheckDlgButton(hWnd, IDC_CHECK1, TRUE);
			}
			::RegCloseKey(hKey);
			return FALSE;
		}
		::RegCloseKey(hKey);
	}
	else
	{
		if (bUINotify)
		{
			::MessageDlg(hWnd, IDS_MSGBOX_REGOPEN_ERROR, MB_ICONERROR);
			if (hWnd != NULL)
				::CheckDlgButton(hWnd, IDC_CHECK1, TRUE);
		}
		return FALSE;
	}
	lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, REG_MENU_SHELL, NULL, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS)
	{
		//删除“校验哈希值”子键
		lRet = ::RegDeleteKey(hKey, strMenu);
		if (lRet != ERROR_SUCCESS)
		{
			if (bUINotify)
			{
				::MessageDlg(hWnd, IDS_MSGBOX_REGDEL_ERROR, IDS_MSGBOX_REG_ERROR, MB_ICONERROR);
				if (hWnd != NULL)
					::CheckDlgButton(hWnd, IDC_CHECK1, TRUE);
			}
			::RegCloseKey(hKey);
			return FALSE;
		}
		::RegCloseKey(hKey);
	}
	else
	{
		if (bUINotify)
		{
			::MessageDlg(hWnd, IDS_MSGBOX_REGOPEN_ERROR, IDS_MSGBOX_REG_ERROR, MB_ICONERROR);
			if (hWnd != NULL)
				::CheckDlgButton(hWnd, IDC_CHECK1, TRUE);
		}
		return FALSE;
	}

	return TRUE;
}