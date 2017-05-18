#ifndef ___HASH_H___
#define ___HASH_H___

#include <fstream>

using std::ifstream;

#ifdef _WIN64
#define _AMD64_
#else
#define _X86_
#endif

#define WM_CALC_PROGRESS	WM_USER + 55
#define MSG_CALC_SIGN		128

#include <tchar.h>
#include <windef.h>
#include <winbase.h>
#include <winuser.h>

#ifndef UINT_8
#define UINT_8  unsigned __int8
#endif

#ifndef UINT_32
#define UINT_32 unsigned __int32
#endif

#ifndef INT_64
#define INT_64 __int64
#endif

#ifndef UINT_64
#define UINT_64 unsigned __int64
#endif

#define MD5_LEN			32
#define SHA1_LEN		40
#define CRC32_LEN		8

#define TYPE_MD5		2
#define TYPE_SHA1		4
#define TYPE_CRC32		8

typedef union
{
	UINT_8 c[64];
	UINT_32 l[16];
} SHA1_WORKSPACE_BLOCK;

class CHash
{
public:
	enum REPORT_TYPE
	{
		REPORT_SHA_HEX = 0,
		REPORT_SHA_DIGIT = 1,
		REPORT_MD5 = 2,
		REPORT_CRC32 = 3
	};

	CHash();
	~CHash();

	//重设
	void Reset();

	//输入数据
	void Update(const UINT_8* pbData, const UINT_32 uLen, const DWORD dwType, bool bUpdateType = true);
	void Update(ifstream& fsIn, DWORD dwLen, HWND hWnd, const BOOL *pCanceled, const DWORD dwType);

	//输出结果
	bool OutputResult(TCHAR* tszReport, REPORT_TYPE rtReportType);

private:
	DWORD m_dwType;
	bool m_bFinaled;

	//MD5转换
	void Transform(const UINT_8 block[64]);
	void Encode(const UINT_32* pInput, UINT_8* pOutput, UINT_32 uLen);
	const UINT_8* Digest();

	//SHA-1转换
	void Transform(UINT_32* pState, const UINT_8* pBuffer);

	void Final();

	//MD5成员变量
	UINT_32 m_state_md5[4];
	UINT_32 m_count_md5[2];
	UINT_32 m_reserved0[1]; //数据对齐
	UINT_8 m_buffer_md5[64];
	UINT_8 m_digest_md5[16];
	UINT_32 m_reserved1[3]; //数据对齐

	static const UINT_8 PADDING[64];
	static const char HEX[16];

	//SHA1成员变量
	UINT_32 m_state_sha[5];
	UINT_32 m_count_sha[2];
	UINT_32 m_reserved2[1]; //数据对齐
	UINT_8 m_buffer_sha[64];
	UINT_8 m_digest_sha[20];
	UINT_32 m_reserved3[3]; //数据对齐

	UINT_8 m_workspace[64];

	SHA1_WORKSPACE_BLOCK* m_block; //SHA1数据指针

	UINT_32 m_crc;	//CRC32值

	enum { BUFFER_SIZE = 1024 };
};

#endif // ___HASH_H___
