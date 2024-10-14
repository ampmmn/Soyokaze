#pragma once

class AppProcess
{
public:
	struct exception {
		exception(const CString& msg) : mMessage(msg) {}
		CString mMessage;
	};
public:
	AppProcess();
	~AppProcess();

	bool IsExist();

// for test
	static void SetSyncObjectName(LPCTSTR name);
	HANDLE GetHandle();

private:
	HANDLE m_hMutexRun;
};
