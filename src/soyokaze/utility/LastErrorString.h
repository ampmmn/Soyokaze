#pragma once

class LastErrorString
{
public:
	LastErrorString(DWORD er) : mMsgBuf(nullptr)
	{
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		FormatMessage(flags, NULL, er, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&mMsgBuf, 0, NULL);
	}
	~LastErrorString()
	{
		LocalFree(mMsgBuf);
	}

	operator LPCTSTR() const {
		return (LPCTSTR)mMsgBuf;
	}

private:
	void* mMsgBuf;
};

