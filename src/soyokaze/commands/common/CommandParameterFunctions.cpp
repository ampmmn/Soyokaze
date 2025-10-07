#include "pch.h"
#include "CommandParameterFunctions.h"
#include "actions/core/ActionParameterIF.h"
#include "core/IFIDDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

using Parameter = launcherapp::actions::core::Parameter;
using NamedParameter = launcherapp::actions::core::NamedParameter;

RefPtr<NamedParameter> GetNamedParameter(Parameter* param)
{
	if (param == nullptr) {
		return std::move(RefPtr<NamedParameter>());
	}

	NamedParameter* namedParam = nullptr;
	if (param->QueryInterface(IFID_ACTIONNAMEDPARAMETER, (void**)&namedParam) == false) {
		return std::move(RefPtr<NamedParameter>());
	}
	return RefPtr<NamedParameter>(namedParam, false);
}

}
}
}


