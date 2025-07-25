#pragma once

#include "commands/validation/CommandParamErrorCode.h"

namespace launcherapp { namespace commands { namespace common {

bool IsValidCommandName(const CString& name, const CString& orgName, CString& errMsg);
bool IsValidCommandName(const CString& name, const CString& orgName, int* errCode);

}}}

