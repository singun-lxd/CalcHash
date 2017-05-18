#include "hash.h"

//MD5相关
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

//MD5相关
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}

const UINT_8 CHash::PADDING[64] = { 0x80 };
const char CHash::HEX[16] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'a', 'b',
	'c', 'd', 'e', 'f'
};

//SHA1相关
#define SHA1_MAX_FILE_BUFFER (32 * 20 * 820)

#ifndef ROL32
#define ROL32(_val32,_nBits) _rotl(_val32,_nBits)
#endif

#define SHABLK0(i) (m_block->l[i] = \
	(ROL32(m_block->l[i],24) & 0xFF00FF00) | (ROL32(m_block->l[i],8) & 0x00FF00FF))

#define SHABLK(i) (m_block->l[i&15] = ROL32(m_block->l[(i+13)&15] ^ \
	m_block->l[(i+8)&15] ^ m_block->l[(i+2)&15] ^ m_block->l[i&15],1))

#define _R0(v,w,x,y,z,i) {z+=((w&(x^y))^y)+SHABLK0(i)+0x5A827999+ROL32(v,5);w=ROL32(w,30);}
#define _R1(v,w,x,y,z,i) {z+=((w&(x^y))^y)+SHABLK(i)+0x5A827999+ROL32(v,5);w=ROL32(w,30);}
#define _R2(v,w,x,y,z,i) {z+=(w^x^y)+SHABLK(i)+0x6ED9EBA1+ROL32(v,5);w=ROL32(w,30);}
#define _R3(v,w,x,y,z,i) {z+=(((w|x)&y)|(w&x))+SHABLK(i)+0x8F1BBCDC+ROL32(v,5);w=ROL32(w,30);}
#define _R4(v,w,x,y,z,i) {z+=(w^x^y)+SHABLK(i)+0xCA62C1D6+ROL32(v,5);w=ROL32(w,30);}

//CRC32相关
static const UINT_32 nCrc32Table[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

CHash::CHash()
{
	m_dwType = 0;
	m_bFinaled = false;
	m_block = (SHA1_WORKSPACE_BLOCK*)m_workspace;

	Reset();
}

CHash::~CHash()
{
	Reset();
}

void CHash::Reset()
{
	//MD5初始化
	m_count_md5[0] = m_count_md5[1] = 0;
	m_state_md5[0] = 0x67452301;
	m_state_md5[1] = 0xefcdab89;
	m_state_md5[2] = 0x98badcfe;
	m_state_md5[3] = 0x10325476;

	//SHA1初始化
	m_state_sha[0] = 0x67452301;
	m_state_sha[1] = 0xEFCDAB89;
	m_state_sha[2] = 0x98BADCFE;
	m_state_sha[3] = 0x10325476;
	m_state_sha[4] = 0xC3D2E1F0;

	m_count_sha[0] = 0;
	m_count_sha[1] = 0;

	//CRC32初始化
	m_crc = (UINT_32)~0;
}

//MD5转换
void CHash::Transform(const UINT_8 block[64])
{

	UINT_32 a = m_state_md5[0], b = m_state_md5[1], c = m_state_md5[2], d = m_state_md5[3], x[16];

	for (size_t i = 0, j = 0; j < 64; ++i, j += 4)
	{
		x[i] = ((UINT_32)block[j]) | (((UINT_32)block[j + 1]) << 8) |
			(((UINT_32)block[j + 2]) << 16) | (((UINT_32)block[j + 3]) << 24);
	}

	FF (a, b, c, d, x[ 0], S11, 0xd76aa478);
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756);
	FF (c, d, a, b, x[ 2], S13, 0x242070db);
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee);
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf);
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a);
	FF (c, d, a, b, x[ 6], S13, 0xa8304613);
	FF (b, c, d, a, x[ 7], S14, 0xfd469501);
	FF (a, b, c, d, x[ 8], S11, 0x698098d8);
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af);
	FF (c, d, a, b, x[10], S13, 0xffff5bb1);
	FF (b, c, d, a, x[11], S14, 0x895cd7be);
	FF (a, b, c, d, x[12], S11, 0x6b901122);
	FF (d, a, b, c, x[13], S12, 0xfd987193);
	FF (c, d, a, b, x[14], S13, 0xa679438e);
	FF (b, c, d, a, x[15], S14, 0x49b40821);

	GG (a, b, c, d, x[ 1], S21, 0xf61e2562);
	GG (d, a, b, c, x[ 6], S22, 0xc040b340);
	GG (c, d, a, b, x[11], S23, 0x265e5a51);
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa);
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d);
	GG (d, a, b, c, x[10], S22,  0x2441453);
	GG (c, d, a, b, x[15], S23, 0xd8a1e681);
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8);
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6);
	GG (d, a, b, c, x[14], S22, 0xc33707d6);
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87);
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed);
	GG (a, b, c, d, x[13], S21, 0xa9e3e905);
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8);
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9);
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a);

	HH (a, b, c, d, x[ 5], S31, 0xfffa3942);
	HH (d, a, b, c, x[ 8], S32, 0x8771f681);
	HH (c, d, a, b, x[11], S33, 0x6d9d6122);
	HH (b, c, d, a, x[14], S34, 0xfde5380c);
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44);
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9);
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60);
	HH (b, c, d, a, x[10], S34, 0xbebfbc70);
	HH (a, b, c, d, x[13], S31, 0x289b7ec6);
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa);
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085);
	HH (b, c, d, a, x[ 6], S34,  0x4881d05);
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039);
	HH (d, a, b, c, x[12], S32, 0xe6db99e5);
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8);
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665);

	II (a, b, c, d, x[ 0], S41, 0xf4292244);
	II (d, a, b, c, x[ 7], S42, 0x432aff97);
	II (c, d, a, b, x[14], S43, 0xab9423a7);
	II (b, c, d, a, x[ 5], S44, 0xfc93a039);
	II (a, b, c, d, x[12], S41, 0x655b59c3);
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92);
	II (c, d, a, b, x[10], S43, 0xffeff47d);
	II (b, c, d, a, x[ 1], S44, 0x85845dd1);
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f);
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0);
	II (c, d, a, b, x[ 6], S43, 0xa3014314);
	II (b, c, d, a, x[13], S44, 0x4e0811a1);
	II (a, b, c, d, x[ 4], S41, 0xf7537e82);
	II (d, a, b, c, x[11], S42, 0xbd3af235);
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb);
	II (b, c, d, a, x[ 9], S44, 0xeb86d391);

	m_state_md5[0] += a;
	m_state_md5[1] += b;
	m_state_md5[2] += c;
	m_state_md5[3] += d;
}

//UINT_32转UINT_8
void CHash::Encode(const UINT_32* pInput, UINT_8* pOutput, UINT_32 uLen)
{
	for (size_t i = 0, j = 0; j < uLen; ++i, j += 4)
	{
		pOutput[j]= (UINT_8)(pInput[i] & 0xff);
		pOutput[j + 1] = (UINT_8)((pInput[i] >> 8) & 0xff);
		pOutput[j + 2] = (UINT_8)((pInput[i] >> 16) & 0xff);
		pOutput[j + 3] = (UINT_8)((pInput[i] >> 24) & 0xff);
	}
}

//SHA1转换
void CHash::Transform(UINT_32* pState, const UINT_8* pBuffer)
{
	UINT_32 a = pState[0], b = pState[1], c = pState[2], d = pState[3], e = pState[4];

	memcpy(m_block, pBuffer, 64);

	_R0(a,b,c,d,e, 0); _R0(e,a,b,c,d, 1); _R0(d,e,a,b,c, 2); _R0(c,d,e,a,b, 3);
	_R0(b,c,d,e,a, 4); _R0(a,b,c,d,e, 5); _R0(e,a,b,c,d, 6); _R0(d,e,a,b,c, 7);
	_R0(c,d,e,a,b, 8); _R0(b,c,d,e,a, 9); _R0(a,b,c,d,e,10); _R0(e,a,b,c,d,11);
	_R0(d,e,a,b,c,12); _R0(c,d,e,a,b,13); _R0(b,c,d,e,a,14); _R0(a,b,c,d,e,15);
	_R1(e,a,b,c,d,16); _R1(d,e,a,b,c,17); _R1(c,d,e,a,b,18); _R1(b,c,d,e,a,19);
	_R2(a,b,c,d,e,20); _R2(e,a,b,c,d,21); _R2(d,e,a,b,c,22); _R2(c,d,e,a,b,23);
	_R2(b,c,d,e,a,24); _R2(a,b,c,d,e,25); _R2(e,a,b,c,d,26); _R2(d,e,a,b,c,27);
	_R2(c,d,e,a,b,28); _R2(b,c,d,e,a,29); _R2(a,b,c,d,e,30); _R2(e,a,b,c,d,31);
	_R2(d,e,a,b,c,32); _R2(c,d,e,a,b,33); _R2(b,c,d,e,a,34); _R2(a,b,c,d,e,35);
	_R2(e,a,b,c,d,36); _R2(d,e,a,b,c,37); _R2(c,d,e,a,b,38); _R2(b,c,d,e,a,39);
	_R3(a,b,c,d,e,40); _R3(e,a,b,c,d,41); _R3(d,e,a,b,c,42); _R3(c,d,e,a,b,43);
	_R3(b,c,d,e,a,44); _R3(a,b,c,d,e,45); _R3(e,a,b,c,d,46); _R3(d,e,a,b,c,47);
	_R3(c,d,e,a,b,48); _R3(b,c,d,e,a,49); _R3(a,b,c,d,e,50); _R3(e,a,b,c,d,51);
	_R3(d,e,a,b,c,52); _R3(c,d,e,a,b,53); _R3(b,c,d,e,a,54); _R3(a,b,c,d,e,55);
	_R3(e,a,b,c,d,56); _R3(d,e,a,b,c,57); _R3(c,d,e,a,b,58); _R3(b,c,d,e,a,59);
	_R4(a,b,c,d,e,60); _R4(e,a,b,c,d,61); _R4(d,e,a,b,c,62); _R4(c,d,e,a,b,63);
	_R4(b,c,d,e,a,64); _R4(a,b,c,d,e,65); _R4(e,a,b,c,d,66); _R4(d,e,a,b,c,67);
	_R4(c,d,e,a,b,68); _R4(b,c,d,e,a,69); _R4(a,b,c,d,e,70); _R4(e,a,b,c,d,71);
	_R4(d,e,a,b,c,72); _R4(c,d,e,a,b,73); _R4(b,c,d,e,a,74); _R4(a,b,c,d,e,75);
	_R4(e,a,b,c,d,76); _R4(d,e,a,b,c,77); _R4(c,d,e,a,b,78); _R4(b,c,d,e,a,79);

	pState[0] += a;
	pState[1] += b;
	pState[2] += c;
	pState[3] += d;
	pState[4] += e;
}

//更新字符串内容并计算
void CHash::Update(const UINT_8* pbData, const UINT_32 uLen, const DWORD dwType, bool bUpdateType)
{
	if (!dwType)
	{
		return;
	}
	if (bUpdateType)
		m_dwType = dwType;
	m_bFinaled = false;

	//MD5计算
	if ((dwType & TYPE_MD5) == TYPE_MD5)
	{
		UINT_32 uIndex, uPartLen;

		uIndex = (UINT_32)((m_count_md5[0] >> 3) & 0x3f);

		if ((m_count_md5[0] += (uLen << 3)) < (uLen << 3))
		{
			++m_count_md5[1];
		}
		m_count_md5[1] += (uLen >> 29);

		uPartLen = 64 - uIndex;

		UINT_32 i;
		if (uLen >= uPartLen)
		{
			memcpy(&m_buffer_md5[uIndex], pbData, uPartLen);
			Transform(m_buffer_md5);

			for (i = uPartLen; i + 63 < uLen; i += 64)
			{
				Transform(&pbData[i]);
			}
			uIndex = 0;
		}
		else
		{
			i = 0;
		}

		memcpy(&m_buffer_md5[uIndex], &pbData[i], uLen - i);
	}
	
	//SHA1计算
	if ((dwType & TYPE_SHA1) == TYPE_SHA1)
	{
		UINT_32 j = ((m_count_sha[0] >> 3) & 0x3F);

		if((m_count_sha[0] += (uLen << 3)) < (uLen << 3))
			++m_count_sha[1];

		m_count_sha[1] += (uLen >> 29);

		UINT_32 i;
		if((j + uLen) > 63)
		{
			i = 64 - j;
			memcpy(&m_buffer_sha[j], pbData, i);
			Transform(m_state_sha, m_buffer_sha);

			for( ; (i + 63) < uLen; i += 64)
				Transform(m_state_sha, &pbData[i]);

			j = 0;
		}
		else
		{
			i = 0;
		}

		if((uLen - i) != 0)
			memcpy(&m_buffer_sha[j], &pbData[i], uLen - i);
	}
	
	//CRC32计算
	if ((dwType & TYPE_CRC32) == TYPE_CRC32)
	{
		UINT_8* pCur = (UINT_8*)pbData;

		UINT_32 nRem = uLen;

		for (; nRem--; ++pCur)
			m_crc = ( m_crc >> 8 ) ^ nCrc32Table[(m_crc ^ *pCur) & 0xff];
	}
}

//更新文件内容并计算
void CHash::Update(ifstream& fsIn, DWORD dwLen, HWND hWnd, const BOOL *pCanceled, const DWORD dwType)
{
	if (!fsIn || !dwType)
		return;

	m_dwType = dwType;
	UINT_32 uIndex = 0;
	UINT_32 uCount = dwLen / BUFFER_SIZE / 100;

	std::streamsize ssLen;
	char buffer[BUFFER_SIZE];
	while (!fsIn.eof())
	{
		if (*(pCanceled) == TRUE)
			break;

		fsIn.read(buffer, BUFFER_SIZE);
		ssLen = fsIn.gcount();
		if (ssLen > 0)
			Update((UINT_8*)buffer, (UINT_32)ssLen, dwType);

		uCount--;
		if (uCount == 0)
		{
			uIndex++;
			::SendMessage(hWnd, WM_CALC_PROGRESS, MSG_CALC_SIGN, (LPARAM)uIndex);
			uCount = dwLen / BUFFER_SIZE / 100;
		}
	}
}

//后续计算
void CHash::Final()
{
	if (m_bFinaled)
		return;

	if ((m_dwType & TYPE_MD5) == TYPE_MD5)
	{
		UINT_8 uBits[8];
		UINT_32 uState[4];
		UINT_32 uCount[2];
		UINT_32 uIndex, uLen;

		memcpy(uState, m_state_md5, 16);
		memcpy(uCount, m_count_md5, 8);

		Encode(m_count_md5, uBits, 8);

		uIndex = (UINT_32)((m_count_md5[0] >> 3) & 0x3f);
		uLen = (uIndex < 56) ? (56 - uIndex) : (120 - uIndex);
		Update(PADDING, uLen, TYPE_MD5, false);

		Update(uBits, 8, TYPE_MD5, false);

		Encode(m_state_md5, m_digest_md5, 16);

		memcpy(m_state_md5, uState, 16);
		memcpy(m_count_md5, uCount, 8);
	}

	if ((m_dwType & TYPE_SHA1) == TYPE_SHA1)
	{
		UINT_8 uFinal[8];

		UINT_32 i;
		for(i = 0; i < 8; ++i)
			uFinal[i] = (UINT_8)((m_count_sha[((i >= 4) ? 0 : 1)]
				>> ((3 - (i & 3)) * 8) ) & 255);

		Update((UINT_8*)"\200", 1, TYPE_SHA1, false);

		while ((m_count_sha[0] & 504) != 448)
			Update((UINT_8*)"\0", 1, TYPE_SHA1, false);

		Update(uFinal, 8, TYPE_SHA1, false);

		for(i = 0; i < 20; ++i)
			m_digest_sha[i] = (UINT_8)((m_state_sha[i >> 2] >> ((3 - (i & 3)) * 8)) & 0xFF);
	}

	m_bFinaled = true;
}

//输出结果
bool CHash::OutputResult(TCHAR* tszReport, REPORT_TYPE rtReportType)
{
	Final();

	if(tszReport == NULL)
		return false;

	if(rtReportType == REPORT_MD5)	//输出MD5
	{
		TCHAR tszTemp[3];

		int t = m_digest_md5[0];
		int a = t / 16;
		int b = t % 16;
		_sntprintf_s(tszTemp, 3, _T("%X%X"), a, b);
		_tcscpy_s(tszReport, 3, tszTemp);

		for(size_t i = 1; i < 16; ++i)
		{
			t = m_digest_md5[i];
			a = t / 16;
			b = t % 16;
			_sntprintf_s(tszTemp, 3, _T("%X%X"), a, b);
			_tcscat_s(tszReport, 33, tszTemp);
		}
	}
	else if(rtReportType == REPORT_SHA_HEX)	//输出SHA-1
	{
		TCHAR tszTemp[16];

		_sntprintf_s(tszTemp, 15, _T("%02X"), m_digest_sha[0]);
		_tcscpy_s(tszReport, 16, tszTemp);

		for(size_t i = 1; i < 20; ++i)
		{
			_sntprintf_s(tszTemp, 15, _T("%02X"), m_digest_sha[i]);
			_tcscat_s(tszReport, 41, tszTemp);
		}
	}
	else if(rtReportType == REPORT_SHA_DIGIT)	//输出SHA-1 Digest
	{
		TCHAR tszTemp[16];

		_sntprintf_s(tszTemp, 15, _T("%u"), m_digest_sha[0]);
		_tcscpy_s(tszReport, 16, tszTemp);

		for(size_t i = 1; i < 20; ++i)
		{
			_sntprintf_s(tszTemp, 15, _T(" %u"), m_digest_sha[i]);
			_tcscat_s(tszReport, 21, tszTemp);
		}
	}
	else if(rtReportType == REPORT_CRC32)	//输出CRC32
	{
		TCHAR tszTemp[9];

		_sntprintf_s(tszTemp, 8, _T("%08X"), ~m_crc);
		_tcscpy_s(tszReport, _tcslen(tszTemp) + 1, tszTemp);
	}
	else
	{
		return false;
	}

#ifdef _DEBUG
	OutputDebugString(tszReport);
	OutputDebugString(_T("\n"));
#endif
	return true;
}