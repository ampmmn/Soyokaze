#pragma once

#include "core/CommandHotKeyHandlerIF.h"

class CommandRepository;

class NamedCommandHotKeyHandler : public soyokaze::core::CommandHotKeyHandler
{
public:
	NamedCommandHotKeyHandler(CommandRepository* cmdReposPtr, CString name);
	virtual ~NamedCommandHotKeyHandler();

	virtual CString GetDisplayName();
	virtual bool Invoke();

protected:
	CommandRepository* mCmdReposPtr;
	CString mName;
};

