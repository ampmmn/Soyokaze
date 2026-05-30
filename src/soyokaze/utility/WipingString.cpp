#include "pch.h"
#include "WipingString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

WipingString::WipingString::WipingString()
{
}

WipingString::WipingString(LPCSTR s) : CString(s)
{
}

WipingString::~WipingString()
{
	// メモリを0で埋める
	auto p = GetBuffer(GetLength());
	SecureZeroMemory(p, sizeof(TCHAR) * GetLength());
	ReleaseBuffer();
}


WipingString& WipingString::operator = (const CString& str)
{
	CString::operator=(str);
	return *this;
}

