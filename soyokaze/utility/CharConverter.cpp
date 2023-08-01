#include "pch.h"
#include "CharConverter.h"

#ifdef _MBCS
#error Unicode文字セットでビルドしてください
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace utility {

CharConverter::CharConverter(int codePage) : mCodePage(codePage)
{
}

CharConverter::~CharConverter()
{
}

CString& CharConverter::Convert(const char* src, CString& dst)
{
	int cp = mCodePage;

	int requiredLen = MultiByteToWideChar(cp, 0, src, -1, NULL, 0);

	MultiByteToWideChar(cp, 0, src, -1, dst.GetBuffer(requiredLen), requiredLen);
	dst.ReleaseBuffer();

	return dst;
}

CStringA& CharConverter::Convert(const CString& src, CStringA& dst)
{
	int cp = mCodePage;

	int requiredLen = WideCharToMultiByte(cp, 0, src, -1, NULL, 0, 0, 0);

	char* p = dst.GetBuffer(requiredLen);
	WideCharToMultiByte(cp, 0, src, -1, p, requiredLen, 0, 0);
	dst.GetBuffer();

	return dst;
}



} // end of namespace utility
} // end of namespace soyokaze

