#pragma once

#define WIN_UNIQUE_MUTEX		_T("{CalcHash_Singun_201211101447}")

#define	ARG_SET_MENU			_T("-setmenu")
#define	ARG_NO_AERO				_T("-noaero")
#define	ARG_USE_AERO			_T("-useaero")
#define	ARG_SWITCH_AERO			_T("-switch")

#define	REG_MENU_SHELL			_T("*\\shell")
#define	REG_MENU_ITEM			_T("*\\shell\\%s")
#define	REG_MENU_FULL			_T("*\\shell\\%s\\command")
#define	REG_MENU_COMMAND		_T("command")
#define	REG_MENU_ICON			_T("Icon")
#define	REG_MENU_FORMAT			_T("\"%s\" \"%%1\"")

#define	REG_DLG_FILTER			_T("*.*\0*.*\0\0")

#define	REG_APPEND_FORMAT		_T("%s\r\n%s")

#define HEAD_MD5_CHS			_T("MD5£º")
#define HEAD_SHA1_CHS			_T("SHA1£º")
#define HEAD_CRC32_CHS			_T("CRC32£º")
#define HEAD_MD5_ENG			_T("MD5:")
#define HEAD_SHA1_ENG			_T("SHA1:")
#define HEAD_CRC32_ENG			_T("CRC32:")

#define ALGORITHM_MD5			_T("MD5")
#define ALGORITHM_SHA1			_T("SHA1")
#define ALGORITHM_CRC32			_T("CRC32")

#define RESULT_MD5				_T("\nMD5")
#define RESULT_SHA1				_T("\nSHA1")
#define RESULT_CRC32			_T("\nCRC32")

#define TEXT_NEW_LINE			_T("\r\n")
#define TEXT_ENTER_KEY			_T("\n")

#define CONFIG_FILE_NAME		_T("\\Config.ini")
#define CONFIG_ROOT				_T("CalcHash")
#define CONFIG_ITEM_HASH		_T("Hash")
#define CONFIG_ITEM_TOPMOST		_T("TopMost")
#define CONFIG_ITEM_UPPER		_T("Upper")
#define CONFIG_ITEM_NOAERO		_T("NoAero")
#define CONFIG_ITEM_LNKOPTION	_T("LnkOption")