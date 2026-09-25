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
	if (in->mHashHandle) {
		auto ret = BCryptDestroyHash(in->mHashHandle);
		if (ret != 0) {
			SPDLOG_ERROR(_T("Failed to BCryptDestroyHash err:{}"), ret);
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
	if (data.size() > 0 && in->mHashHandle) {
		auto ret = BCryptHashData(in->mHashHandle, (BYTE*)&data.front(), (ULONG)data.size(), 0);
		if (ret != 0) {
			SPDLOG_ERROR(_T("Failed to BCryptHashData err:{}"), ret);
		}
	}
}

void SHA1::Add(const CString& data)
{
	if (data.GetLength() > 0 && in->mHashHandle) {
		auto ret = BCryptHashData(in->mHashHandle, (BYTE*)(LPCTSTR)data, (ULONG)(data.GetLength() * sizeof(TCHAR)), 0);
		if (ret != 0) {
			SPDLOG_ERROR(_T("Failed to BCryptHashData err:{}"), ret);
		}
	}
}


CString SHA1::Finish(bool fullDigest)
{
	if (in->mHashHandle == nullptr) {
		return _T("");
	}

	DWORD data = 0;

	DWORD hashLen = 0;
	auto ret = BCryptGetProperty(in->mAlgHandle, BCRYPT_HASH_LENGTH, (PBYTE)&hashLen, sizeof(DWORD), &data, 0);
	if (ret != 0) {
		SPDLOG_ERROR(_T("Failed to BCryptGetProperty err:{}"), ret);
		return _T("");
	}

	in->mHashData.resize(hashLen);

	ret = BCryptFinishHash(in->mHashHandle, &in->mHashData.front(), (ULONG)in->mHashData.size(), 0);
	if (ret != 0) {
		SPDLOG_ERROR(_T("Failed to BCryptFinishHash err:{}"), ret);
		return _T("");
	}

	CString output;
	const size_t byteCount = fullDigest ? in->mHashData.size() : (std::min)(size_t(4), in->mHashData.size());
	for (size_t i = 0; i < byteCount; ++i) {
		CString byteString;
		byteString.Format(_T("%02x"), in->mHashData[i]);
		output += byteString;
	}

	return output;
}

