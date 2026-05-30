#include "pch.h"
#include "WipingBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


WipingBuffer::WipingBuffer(size_t len, wchar_t c) : mData(len, c)
{
}

WipingBuffer::~WipingBuffer()
{
	// メモリを0で埋める
	SecureZeroMemory(Data(), Length());
}


size_t WipingBuffer::Length()
{
	return mData.size();
}

wchar_t* WipingBuffer::Data()
{
	return mData.data();
}
