#include "pch.h"
#include "CharConverter.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace common {


CharConverter::CharConverter()
{
}

CharConverter::~CharConverter()
{
}

CString& CharConverter::Convert(const char* src, CString& dst)
{
	// Todo: UTF-8とCP932を選べるようにする
	int cp = CP_UTF8;

	int requiredLen = MultiByteToWideChar(cp, 0, src, -1, NULL, 0);

	MultiByteToWideChar(cp, 0, src, -1, mBuff.GetBuffer(requiredLen), requiredLen);
	mBuff.ReleaseBuffer();

	dst = mBuff;

	return dst;
}


} // end of namespace common
} // end of namespace commands
} // end of namespace soyokaze

