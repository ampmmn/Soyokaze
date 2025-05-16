#pragma once

#include "commands/common/UserCommandProviderBase.h"

namespace launcherapp { namespace commands { namespace remote {


class RemoteClientCommandProvider :
	public launcherapp::commands::common::UserCommandProviderBase
{
private:
	RemoteClientCommandProvider();
	~RemoteClientCommandProvider() override;

public:
	CString GetName() override;

	// $B:n@.$G$-$k%3%^%s%I$N<oN`$rI=$9J8;zNs$r<hF@(B
	CString GetDisplayName() override;

	// $B%3%^%s%I$N<oN`$N@bL@$r<($9J8;zNs$r<hF@(B
	CString GetDescription() override;

	// $BHs8x3+%3%^%s%I$+$I$&$+(B
	bool IsPrivate() const override;

	// $B%3%^%s%I?75,:n@.%@%$%"%m%0(B
	bool NewDialog(CommandParameter* param) override;

	// Provider$B4V$NM%@h=g0L$rI=$9CM$rJV$9!#>.$5$$$[$IM%@h(B
	uint32_t GetOrder() const override;

	DECLARE_COMMANDPROVIDER(RemoteClientCommandProvider)

	DECLARE_LOADFROM(RemoteClientCommandProvider)
};


}}} // end of namespace launcherapp::commands::remote

