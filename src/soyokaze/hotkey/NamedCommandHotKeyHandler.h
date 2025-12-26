#pragma once

#include "hotkey/CommandHotKeyHandlerIF.h"

class NamedCommandHotKeyHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	NamedCommandHotKeyHandler(CString name);
	~NamedCommandHotKeyHandler() override;

	CString GetDisplayName() override;
	bool Invoke() override;
	bool IsTemporaryHandler() override;
	uint32_t AddRef() override;
	uint32_t Release() override;


protected:
	CString mName;
	uint32_t mRefCount{1};
};

