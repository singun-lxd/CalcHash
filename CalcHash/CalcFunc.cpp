#include "stdafx.h"
#include "CalcFunc.h"
#include <fstream>
#include <string>
using namespace std;

//计算哈希线程函数
unsigned __stdcall CalcThread(LPVOID lpParameter)
{
	ThreadData* pData = reinterpret_cast<ThreadData *>(lpParameter);

	LPTSTR lpszHash = new TCHAR[MD5_LEN + SHA1_LEN + CRC32_LEN + 3];
	while(1)
	{
		CHash hash;
		if (pData->bFile == TRUE)
		{
			locale::global(locale(""));
			ifstream fsFileIn(pData->lpszFile, ios::binary);
			locale::global(locale("C"));
			if (!fsFileIn.is_open())
			{
				*pData->pnErrCode = -1;
				break;
			}
			fsFileIn.seekg(0, ios::end);
			DWORD dwLen = static_cast<DWORD>(fsFileIn.tellg());
			fsFileIn.seekg(0, ios::beg);
			hash.Update(fsFileIn, dwLen, pData->hWnd, pData->pbCancel, pData->dwType);
			fsFileIn.close();
		}
		else
		{
			hash.Update(reinterpret_cast<const UINT_8*>(pData->lpszFile),
				static_cast<unsigned int>(::strlen(pData->lpszFile)),
				pData->dwType);
		}

		if ((pData->dwType & TYPE_MD5) == TYPE_MD5)
		{
			TCHAR szMd5[MD5_LEN + 1];
			hash.OutputResult(szMd5, CHash::REPORT_MD5);
			_tcscpy_s(lpszHash, MD5_LEN + 1, szMd5);
		}
		else
		{
			_tcscpy_s(lpszHash, 1, _T(""));
		}

		if ((pData->dwType & TYPE_SHA1) == TYPE_SHA1)
		{
			TCHAR szSha1[SHA1_LEN + 1];
			hash.OutputResult(szSha1, CHash::REPORT_SHA_HEX);
			_tcscat_s(lpszHash, MD5_LEN + 2, _T("\n"));
			_tcscat_s(lpszHash, MD5_LEN + SHA1_LEN + 2, szSha1);
		}

		if ((pData->dwType & TYPE_CRC32) == TYPE_CRC32)
		{
			TCHAR szCRC32[CRC32_LEN + 1];
			hash.OutputResult(szCRC32, CHash::REPORT_CRC32);
			_tcscat_s(lpszHash, MD5_LEN + SHA1_LEN + 3, _T("\n"));
			_tcscat_s(lpszHash, MD5_LEN + SHA1_LEN + CRC32_LEN + 3, szCRC32);
		}
		break;
	}

	::SendMessage(pData->hWnd, WM_CALC_STOP, static_cast<WPARAM>(*(pData->pnErrCode)),
		reinterpret_cast<LPARAM>(reinterpret_cast<LPCTSTR>(lpszHash)));

	//清理工作
	delete [](pData->lpszFile);
	return 0;
}

void GetFileSizeTextString(LONGLONG llFileSize, CString& strFileSize)
{
	int nFlag;
	CString strEnd;

	strFileSize.Empty();

	if (llFileSize < 1000)
	{
		strEnd = _T("B");
		strFileSize.Format(_T("%I64d"), llFileSize);
	}
	else if (llFileSize < 1000 * 1024)
	{
		strEnd = _T("KB");
		strFileSize.Format(_T("%0.3f"), static_cast<long double>(llFileSize) / 1024);
	}
	else if (llFileSize < 1000 * 1024 * 1024)
	{
		strEnd = _T("MB");
		strFileSize.Format(_T("%0.3f"), static_cast<long double>(llFileSize) / (1024 * 1024));
	}
	else if (llFileSize < static_cast<LONGLONG>(1000) * 1024 * 1024 * 1024)
	{
		strEnd = _T("GB");
		strFileSize.Format(_T("%0.3f"), static_cast<long double>(llFileSize) / (1024 * 1024 * 1024));
	}
	else
	{
		strEnd = _T("TB");
		strFileSize.Format(_T("%0.3f"), static_cast<long double>(llFileSize) /
			(static_cast<LONGLONG>(1024) * 1024 * 1024 * 1024));
	}

	nFlag = strFileSize.Find(_T('.'));
	if (nFlag != -1)
	{
		if (nFlag >= 3)
			strFileSize = strFileSize.Left(nFlag);
		else
			strFileSize = strFileSize.Left(4);
	}

	strFileSize += _T(" ");
	strFileSize += strEnd;
}

void GetNumberSubsectionString(LONGLONG llNumber, UINT nSubLength, CString& strSubsection)
{
	strSubsection.Format(_T("%I64d"), llNumber);

	if (nSubLength > 0)
	{
		int nLen = strSubsection.GetLength();
		int nIndex = nLen % nSubLength;
		if (nIndex == 0)
			nIndex = nSubLength;
		for (; nIndex < nLen; nIndex = nIndex + nSubLength)
		{
			strSubsection.Insert(nIndex, _T(','));
			nLen++;
			nIndex++;
		}
	}
}