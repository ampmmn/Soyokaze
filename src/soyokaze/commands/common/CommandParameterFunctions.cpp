#include "pch.h"
#include "CommandParameterFunctions.h"
#include "commands/core/CommandParameterIF.h"
#include "core/IFIDDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

using CommandParameter = launcherapp::core::CommandParameter;
using CommandNamedParameter = launcherapp::core::CommandNamedParameter;

RefPtr<CommandNamedParameter> GetCommandNamedParameter(CommandParameter* param)
{
	if (param == nullptr) {
		return std::move(RefPtr<CommandNamedParameter>());
	}

	CommandNamedParameter* namedParam = nullptr;
	if (param->QueryInterface(IFID_COMMANDNAMEDPARAMETER, (void**)&namedParam) == false) {
		return std::move(RefPtr<CommandNamedParameter>());
	}
	return RefPtr<CommandNamedParameter>(namedParam, false);
}


bool IsCtrlKeyPressed(launcherapp::core::CommandParameter* param)
{
	auto namedParam = GetCommandNamedParameter(param);
	if (param == nullptr) {
		return false;
	}
	return namedParam->GetNamedParamBool(_T("CtrlKeyPressed"));
}

bool IsShiftKeyPressed(launcherapp::core::CommandParameter* param)
{
	auto namedParam = GetCommandNamedParameter(param);
	if (param == nullptr) {
		return false;
	}
	return namedParam->GetNamedParamBool(_T("ShiftKeyPressed"));
}
bool IsAltKeyPressed(launcherapp::core::CommandParameter* param)
{
	auto namedParam = GetCommandNamedParameter(param);
	if (param == nullptr) {
		return false;
	}
	return namedParam->GetNamedParamBool(_T("AltKeyPressed"));
}

bool IsWinKeyPressed(launcherapp::core::CommandParameter* param)
{
	auto namedParam = GetCommandNamedParameter(param);
	if (param == nullptr) {
		return false;
	}
	return namedParam->GetNamedParamBool(_T("WinKeyPressed"));
}

uint32_t GetModifierKeyState(launcherapp::core::CommandParameter* param, uint32_t mask)
{
	auto namedParam = GetCommandNamedParameter(param);
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


