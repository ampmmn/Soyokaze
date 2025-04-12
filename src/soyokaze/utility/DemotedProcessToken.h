#pragma once

class DemotedProcessToken
{
public:
	DemotedProcessToken();
	~DemotedProcessToken();

	HANDLE FetchPrimaryToken();

	static bool IsRunningAsAdmin();

private:
	HANDLE mPrimaryToken;
	HANDLE mShellProcessHandle;
	HANDLE mShProcTokenHandle;
};

