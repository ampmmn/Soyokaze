#pragma once

#include "hotkey/CommandHotKeyHandlerIF.h"
#include "hotkey/HotKeyAttribute.h"

namespace launcherapp { namespace core {
	class Command;
}}

class ExtraActionHotKeyHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	ExtraActionHotKeyHandler(CString displayName, launcherapp::core::Command* cmd, const HOTKEY_ATTR& hotkeyAttr);
	~ExtraActionHotKeyHandler() override;

	CString GetDisplayName() override;
	bool Invoke() override;
	bool IsTemporaryHandler() override;
	uint32_t AddRef() override;
	uint32_t Release() override;


protected:
	CString mDisplayName;
	launcherapp::core::Command* mCmd{nullptr};
	HOTKEY_ATTR mHotkeyAttr;
	uint32_t mRefCount{1};
};

