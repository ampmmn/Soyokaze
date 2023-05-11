#pragma once

#include "CommandIF.h"
#include "AppPreferenceListenerIF.h"
#include <vector>

class CommandRepository : public AppPreferenceListenerIF
{
public:
	CommandRepository();
	virtual ~CommandRepository();

public:
	BOOL Load();

	int NewCommandDialog(const CString* cmdNamePtr, const CString* pathPtr, const CString* descStr = nullptr);
	int EditCommandDialog(const CString& cmdName);
	int ManagerDialog();

	bool DeleteCommand(const CString& cmdName);

	void EnumCommands(std::vector<Command*>& commands);

	bool IsBuiltinName(const CString& cmdName);

	void Query(const CString& strQueryStr, std::vector<Command*>& commands);
	Command* QueryAsWholeMatch(const CString& strQueryStr, bool isSearchPath = true);

	bool IsValidAsName(const CString& strQueryStr);

protected:
	virtual void OnAppPreferenceUpdated();

protected:
	struct PImpl;
	PImpl* in;
};

