#pragma once

#include "commands/validation/CommandParamErrorCode.h"
#include "commands/validation/CommandParamError.h"

namespace launcherapp { namespace commands { namespace validation {

bool IsValidCommandName(const CString& name, const CString& orgName, CString& errMsg);
bool IsValidCommandName(const CString& name, const CString& orgName, int* errCode);

}}}

