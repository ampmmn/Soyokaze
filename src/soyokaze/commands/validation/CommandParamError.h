#pragma once

namespace launcherapp { namespace commands { namespace validation {

class CommandParamError
{
public:
	CommandParamError();
	CommandParamError(int errCode);
	~CommandParamError();

	CString ToString() const;

private:
	int mErrCode{0};
};

}}}


