#pragma once

#include "commands/common/AdhocCommandProviderBase.h"


namespace launcherapp {
namespace commands {
namespace mailto {


class MailToCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	MailToCommandProvider();
	virtual ~MailToCommandProvider();

public:
	void AddHistory(const CString& word, const CString& fullPath);

public:
	virtual CString GetName();
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(MailToCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace mailto
} // end of namespace commands
} // end of namespace launcherapp

