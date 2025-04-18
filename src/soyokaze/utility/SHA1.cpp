#include "pch.h"
#include "SHA1.h"
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

struct SHA1::PImpl
{
	BCRYPT_ALG_HANDLE mAlgHandle{nullptr};
	BCRYPT_HASH_HANDLE mHashHandle{nullptr};
	std::vector<uint8_t> mHashObj;
	std::vector<uint8_t> mHashData;
};

SHA1::SHA1() : in(new PImpl)
{
	auto ret = BCryptOpenAlgorithmProvider(&in->mAlgHandle, BCRYPT_SHA1_ALGORITHM, NULL, 0);
	if (ret != 0) {
		SPDLOG_ERROR(_T("Failed to BCryptOpenAlgorithmProvider err:{}"), ret);
		return ;
	}

	DWORD data = 0;

	DWORD objLen = 0;
	ret = BCryptGetProperty(in->mAlgHandle, BCRYPT_OBJECT_LENGTH, (PBYTE)&objLen, sizeof(DWORD), &data, 0);
	if (ret != 0) {
		SPDLOG_ERROR(_T("Failed to BCryptGetProperty err:{}"), ret);
		return ;
	}

	in->mHashObj.resize(objLen);
	ret = BCryptCreateHash(in->mAlgHandle, &in->mHashHandle, &in->mHashObj.front(), objLen, NULL, 0, 0);
	if (ret != 0) {
		SPDLOG_ERROR(_T("Failed to BCryptCreateHash err:{}"), ret);
		return ;
	}
}

SHA1::~SHA1()
{
	if (in->mHashObj.size()) {
		BYTE* p = (BYTE*)&in->mHashObj.front();
		if (p) {
			auto ret = BCryptDestroyHash(p);
			if (ret != 0) {
				SPDLOG_ERROR(_T("Failed to BCryptDestroyHash err:{}"), ret);
				return ;
			}
		}
	}

	if (in->mAlgHandle) {
		auto ret = BCryptCloseAlgorithmProvider(in->mAlgHandle, 0);
		if (ret != 0) {
			SPDLOG_ERROR(_T("Failed to BCryptCloseAlgorithmProvider err:{}"), ret);
			return ;
		}
	}
}

void SHA1::Add(const std::vector<uint8_t>& data)
{
	if (data.size() > 0) {
		auto ret = BCryptHashData((BCRYPT_HASH_HANDLE)&in->mHashObj.front(), (BYTE*)&data.front(), (ULONG)data.size(), 0);
		if (ret != 0) {
			SPDLOG_ERROR(_T("Failed to BCryptHashData err:{}"), ret);
		}
	}
}

void SHA1::Add(const CString& data)
{
	if (data.GetLength() > 0) {
		auto ret = BCryptHashData((BCRYPT_HASH_HANDLE)&in->mHashObj.front(), (BYTE*)(LPCTSTR)data, (ULONG)(data.GetLength() * sizeof(TCHAR)), 0);
		if (ret != 0) {
			SPDLOG_ERROR(_T("Failed to BCryptHashData err:{}"), ret);
		}
	}
}


CString SHA1::Finish()
{
	DWORD data = 0;

	DWORD hashLen = 0;
	auto ret = BCryptGetProperty(in->mAlgHandle, BCRYPT_HASH_LENGTH, (PBYTE)&hashLen, sizeof(DWORD), &data, 0);
	if (ret != 0) {
		SPDLOG_ERROR(_T("Failed to BCryptGetProperty err:{}"), ret);
		return _T("");
	}

	in->mHashData.resize(hashLen);

	ret = BCryptFinishHash((BCRYPT_HASH_HANDLE)&in->mHashObj.front(), &in->mHashData.front(), (ULONG)in->mHashData.size(), 0);
	if (ret != 0) {
		SPDLOG_ERROR(_T("Failed to BCryptFinishHash err:{}"), ret);
		return _T("");
	}

	// 先頭4byteをASCII文字列化する
	auto p = (uint8_t*)&in->mHashData.front();

	CString output;
	output.Format(_T("%02x%02x%02x%02x"), p[0], p[1], p[2], p[3]);

	return output;
}

