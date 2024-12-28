#pragma once

#include "hotkey/CommandHotKeyHandlerIF.h"

class NamedCommandHotKeyHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	NamedCommandHotKeyHandler(CString name);
	virtual ~NamedCommandHotKeyHandler();

	virtual CString GetDisplayName();
	virtual bool Invoke();

protected:
	CString mName;
};

