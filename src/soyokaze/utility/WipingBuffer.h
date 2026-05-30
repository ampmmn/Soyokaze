#pragma once

#include <vector>

class WipingBuffer
{
public:
	WipingBuffer(size_t len = 0, wchar_t c = L'\0');
	~WipingBuffer();

	size_t Length();
	wchar_t* Data();

private:
	std::vector<wchar_t> mData;
};
