#include "pch.h"
#include "Base64.h"
#include <wincrypt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace utility {
namespace base64 {

CString EncodeBase64(const std::vector<uint8_t>& stm)
{
	// 長さを調べる
	DWORD dstLen = 0;
	if (stm.size() == 0) {
		spdlog::error("EncodeBase64 : empty");
		return CString();
	}
	if (CryptBinaryToString( &stm.front(), (int)stm.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &dstLen) == FALSE) {
		spdlog::error("EncodeBase64 : failed to get size");
		return CString();
	}

	// 変換する
	CString dstStr;
	if (CryptBinaryToString( &stm.front(), (int)stm.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, dstStr.GetBuffer(dstLen+1), &dstLen ) == FALSE) {
		spdlog::error("EncodeBase64 : failed to encode");
	}
	dstStr.ReleaseBuffer();

	return dstStr;
}

bool DecodeBase64(const CString& src, std::vector<uint8_t>& stm)
{
	DWORD dstLen = 0;
	if (CryptStringToBinary( (LPCTSTR)src, src.GetLength(), CRYPT_STRING_BASE64, nullptr, &dstLen, nullptr, nullptr ) == FALSE) {
		spdlog::error("DecodeBase64 : failed to get size");
		return false;
	}

	std::vector<uint8_t> dst(dstLen);
	if (CryptStringToBinary( (LPCTSTR)src, src.GetLength(), CRYPT_STRING_BASE64, &dst.front(), &dstLen, nullptr, nullptr)== FALSE) {
		spdlog::error("DecodeBase64 : failed to convert");
		return false;
	}

	stm.swap(dst);

	return true;
}

}
}
