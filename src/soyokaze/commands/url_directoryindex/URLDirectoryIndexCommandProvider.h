#pragma once

#include "commands/common/UserCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace url_directoryindex {


class URLDirectoryIndexCommandProvider :
	public launcherapp::commands::common::UserCommandProviderBase
{

private:
	URLDirectoryIndexCommandProvider();
	~URLDirectoryIndexCommandProvider() override;

public:
	virtual CString GetName();

	// $B:n@.$G$-$k%3%^%s%I$N<oN`$rI=$9J8;zNs$r<hF@(B
	virtual CString GetDisplayName();

	// $B%3%^%s%I$N<oN`$N@bL@$r<($9J8;zNs$r<hF@(B
	virtual CString GetDescription();

	// $B%3%^%s%I?75,:n@.%@%$%"%m%0(B
	virtual bool NewDialog(CommandParameter* param);

	// Provider$B4V$NM%@h=g0L$rI=$9CM$rJV$9!#>.$5$$$[$IM%@h(B
	virtual uint32_t GetOrder() const;
	// Provider$B$,07$&%3%^%s%I<oJL(B($BI=<(L>(B)$B$rNs5s(B
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(URLDirectoryIndexCommandProvider)

// UserCommandProviderBase
	void OnBeforeLoad() override;
	bool LoadFrom(CommandEntryIF* entry, Command** command) override;
};


} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

