#pragma once

#include "commands/core/CommandProviderIF.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

class BookmarkCommandProvider :
	public launcherapp::core::CommandProvider
{
	using Command = launcherapp::core::Command;
	using CommandParameter = launcherapp::core::CommandParameter;

private:
	BookmarkCommandProvider();
	virtual ~BookmarkCommandProvider();

public:
	// $B=i2s5/F0$N=i4|2=$r9T$&(B
	void OnFirstBoot() override;

	// $B%3%^%s%I$NFI$_9~$_(B
	void LoadCommands(CommandFile* commandFile) override;

	CString GetName() override;

	// $B:n@.$G$-$k%3%^%s%I$N<oN`$rI=$9J8;zNs$r<hF@(B
	CString GetDisplayName() override;

	// $B%3%^%s%I$N<oN`$N@bL@$r<($9J8;zNs$r<hF@(B
	CString GetDescription() override;

	// $B%3%^%s%I?75,:n@.%@%$%"%m%0(B
	bool NewDialog(const CommandParameter* param) override;

	// $BHs8x3+%3%^%s%I$+$I$&$+(B($B?75,:n@.BP>]$K$7$J$$(B)
	bool IsPrivate() const override;

	// $B0l;~E*$J%3%^%s%I$rI,MW$K1~$8$FDs6!$9$k(B
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;

	// Provider$B4V$NM%@h=g0L$rI=$9CM$rJV$9!#>.$5$$$[$IM%@h(B
	uint32_t GetOrder() const override;

	// $B@_Dj%Z!<%8$r<hF@$9$k(B
	bool CreateSettingPages(CWnd* parent, std::vector<SettingPage*>& pages) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

	DECLARE_COMMANDPROVIDER(BookmarkCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

