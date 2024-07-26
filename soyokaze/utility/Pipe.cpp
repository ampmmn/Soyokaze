#include "pch.h"
#include "Pipe.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


Pipe::Pipe() : mReadHandle(nullptr), mWriteHandle(nullptr)
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	BOOL isOK = CreatePipe(&mReadHandle, &mWriteHandle, &sa, 0);
	if (isOK == FALSE) {
		spdlog::warn(_T("Failed to createpipe."));
	}
}

Pipe::~Pipe()
{
	if (mWriteHandle) {
		CloseHandle(mWriteHandle);
	}
	if (mReadHandle) {
		CloseHandle(mReadHandle);
	}
}

HANDLE Pipe::GetWriteHandle()
{
	return mWriteHandle;
}

HANDLE Pipe::GetReadHandle()
{
	return mReadHandle;
}


size_t Pipe::ReadAll(Buffer& output)
{
	size_t readTotal = 0;

	HANDLE hRead = mReadHandle;

	char buff[65536] = {};
	DWORD bufLen = (DWORD)sizeof(buff);

	DWORD rest = 0;
	PeekNamedPipe(hRead, buff, bufLen, NULL, &rest, 0);

	while (rest > 0) {
		DWORD readed = 0;
		if (ReadFile(hRead, buff,  bufLen < rest? bufLen : rest, &readed, NULL) == FALSE) {
			SPDLOG_ERROR(_T("Failed to ReadFile! err={:x}"), GetLastError());
			break;
		}

		rest -= readed;
		readTotal += readed;

		output.insert(output.end(), buff, buff + readed);
	}

	return readTotal;
}

