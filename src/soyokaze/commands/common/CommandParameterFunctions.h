#pragma once

#include "utility/RefPtr.h"

namespace launcherapp {

namespace actions {
namespace core {
	class Parameter;
	class NamedParameter;
}
}

namespace commands {
namespace common {

enum {
	MASK_CTRL = 1,
	MASK_SHIFT = 2,
	MASK_ALT = 4,
	MASK_WIN = 8,
	MASK_ALL = 15,
};

RefPtr<launcherapp::actions::core::NamedParameter> GetNamedParameter(launcherapp::actions::core::Parameter* param);
RefPtr<launcherapp::actions::core::NamedParameter> GetNamedParameter(launcherapp::actions::core::Parameter* param);

bool IsCtrlKeyPressed(launcherapp::actions::core::Parameter* param);
bool IsShiftKeyPressed(launcherapp::actions::core::Parameter* param);
bool IsAltKeyPressed(launcherapp::actions::core::Parameter* param);
bool IsWinKeyPressed(launcherapp::actions::core::Parameter* param);

uint32_t GetModifierKeyState(launcherapp::actions::core::Parameter* param, uint32_t mask);

}
}
}


