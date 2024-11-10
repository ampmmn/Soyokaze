#pragma once

class DemoteedProcessToken
{
public:
	DemoteedProcessToken();
	~DemoteedProcessToken();

	HANDLE FetchPrimaryToken();

private:
	HANDLE mPrimaryToken;
	HANDLE mShellProcessHandle;
	HANDLE mShProcTokenHandle;
};

