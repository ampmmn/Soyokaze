#pragma once

class DemotedProcessToken
{
public:
	DemotedProcessToken();
	~DemotedProcessToken();

	HANDLE FetchPrimaryToken();

private:
	HANDLE mPrimaryToken;
	HANDLE mShellProcessHandle;
	HANDLE mShProcTokenHandle;
};

