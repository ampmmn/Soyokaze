#pragma once

namespace launcherapp {

namespace core {
	class CommandParameter;
	class CommandNamedParameter;
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

launcherapp::core::CommandNamedParameter* GetCommandNamedParameter(launcherapp::core::CommandParameter* param);

bool IsCtrlKeyPressed(launcherapp::core::CommandParameter* param);
bool IsShiftKeyPressed(launcherapp::core::CommandParameter* param);
bool IsAltKeyPressed(launcherapp::core::CommandParameter* param);
bool IsWinKeyPressed(launcherapp::core::CommandParameter* param);

uint32_t GetModifierKeyState(launcherapp::core::CommandParameter* param, uint32_t mask);

}
}
}


