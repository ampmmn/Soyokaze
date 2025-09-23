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


bool IsCtrlKeyPressed(Parameter* param)
{
	auto namedParam = GetNamedParameter(param);
	if (param == nullptr) {
		return false;
	}
	return namedParam->GetNamedParamBool(_T("CtrlKeyPressed"));
}

bool IsShiftKeyPressed(Parameter* param)
{
	auto namedParam = GetNamedParameter(param);
	if (param == nullptr) {
		return false;
	}
	return namedParam->GetNamedParamBool(_T("ShiftKeyPressed"));
}
bool IsAltKeyPressed(Parameter* param)
{
	auto namedParam = GetNamedParameter(param);
	if (param == nullptr) {
		return false;
	}
	return namedParam->GetNamedParamBool(_T("AltKeyPressed"));
}

bool IsWinKeyPressed(Parameter* param)
{
	auto namedParam = GetNamedParameter(param);
	if (param == nullptr) {
		return false;
	}
	return namedParam->GetNamedParamBool(_T("WinKeyPressed"));
}

uint32_t GetModifierKeyState(Parameter* param, uint32_t mask)
{
	auto namedParam = GetNamedParameter(param);
	if (param == nullptr) {
		return false;
	}

	uint32_t state = 0;
	if ((mask & MASK_CTRL) && namedParam->GetNamedParamBool(_T("CtrlKeyPressed"))) {
		state |= MASK_CTRL;
	}
	if ((mask & MASK_SHIFT) && namedParam->GetNamedParamBool(_T("ShiftKeyPressed"))) {
		state |= MASK_SHIFT;
	}
	if ((mask & MASK_ALT) && namedParam->GetNamedParamBool(_T("AltKeyPressed"))) {
		state |= MASK_ALT;
	}
	if ((mask & MASK_WIN) && namedParam->GetNamedParamBool(_T("WinKeyPressed"))) {
		state |= MASK_WIN;
	}

	return state;
}


}
}
}


