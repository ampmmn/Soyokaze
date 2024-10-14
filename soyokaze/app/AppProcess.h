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

private:
	HANDLE m_hMutexRun;
};
