#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/url_directoryindex/DirectoryIndexQueryResult.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

class CommandParam;

class URLDirectoryIndexCommand : public launcherapp::commands::common::UserCommandBase
{
public:
	URLDirectoryIndexCommand();
	virtual ~URLDirectoryIndexCommand();

	void SetSubPath(const CString& subPath);
	CString GetSubPath();
	void LoadCanidates(bool& isHTML);

	void Query(Pattern* pattern, DirectoryIndexQueryResult& results);

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	int EditDialog(HWND parent) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	static CString GetType();

	static bool NewDialog(const Parameter* param, URLDirectoryIndexCommand** newCmd);
	static bool CastFrom(launcherapp::core::Command* cmd, URLDirectoryIndexCommand** newCmd); 
	
public:
	URLDirectoryIndexCommand& SetParam(const CommandParam& param);
	URLDirectoryIndexCommand& GetParam(CommandParam& param);


protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

