#include "pch.h"
#include "SHA1.h"
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

struct SHA1::PImpl
{
	BCRYPT_ALG_HANDLE mAlgHandle;
	BCRYPT_HASH_HANDLE mHashHandle;
	std::vector<uint8_t> mHashObj;
	std::vector<uint8_t> mHashData;
};

SHA1::SHA1() : in(new PImpl)
{
	in->mAlgHandle = nullptr;
	in->mHashHandle = nullptr;

	BCryptOpenAlgorithmProvider(&in->mAlgHandle, BCRYPT_SHA1_ALGORITHM, NULL, 0);

	DWORD data = 0;

	DWORD objLen = 0;
	BCryptGetProperty(in->mAlgHandle, BCRYPT_OBJECT_LENGTH, (PBYTE)&objLen, sizeof(DWORD), &data, 0);

	in->mHashObj.resize(objLen);
	BCryptCreateHash(in->mAlgHandle, &in->mHashHandle, &in->mHashObj.front(), objLen, NULL, 0, 0);
}

SHA1::~SHA1()
{
	if (in->mHashObj.size()) {
		BYTE* p = (BYTE*)&in->mHashObj.front();
		if (p) {
			BCryptDestroyHash(p);
		}
	}

	if (in->mAlgHandle) {
		BCryptCloseAlgorithmProvider(in->mAlgHandle, 0);
	}
}

void SHA1::Add(const std::vector<uint8_t>& data)
{
	if (data.size() > 0) {
		BCryptHashData((BCRYPT_HASH_HANDLE)&in->mHashObj.front(), (BYTE*)&data.front(), (ULONG)data.size(), 0);
	}
}

void SHA1::Add(const CString& data)
{
	if (data.GetLength() > 0) {
		BCryptHashData((BCRYPT_HASH_HANDLE)&in->mHashObj.front(), (BYTE*)(LPCTSTR)data, (ULONG)(data.GetLength() * sizeof(TCHAR)), 0);
	}
}


CString SHA1::Finish()
{
	DWORD data = 0;

	DWORD hashLen = 0;
	BCryptGetProperty(in->mAlgHandle, BCRYPT_HASH_LENGTH, (PBYTE)&hashLen, sizeof(DWORD), &data, 0);
	in->mHashData.resize(hashLen);

	BCryptFinishHash((BCRYPT_HASH_HANDLE)&in->mHashObj.front(), &in->mHashData.front(), (ULONG)in->mHashData.size(), 0);

	// 先頭4byteをASCII文字列化する
	auto p = (uint8_t*)&in->mHashData.front();

	CString output;
	output.Format(_T("%02x%02x%02x%02x"), p[0], p[1], p[2], p[3]);

	return output;
}

