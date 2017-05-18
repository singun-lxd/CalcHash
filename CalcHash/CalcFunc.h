#include "hash.h"

#pragma once

typedef struct _ThreadData
{
	HWND hWnd;
	LPCSTR lpszFile;
	BOOL bFile;
	INT* pnErrCode;
	DWORD dwType;
	BOOL* pbCancel;
} ThreadData;

unsigned __stdcall CalcThread(LPVOID lpParameter);

void GetFileSizeTextString(LONGLONG llFileSize, CString& strFileSize);
void GetNumberSubsectionString(LONGLONG llNumber, UINT nSubLength, CString& strSubsection);