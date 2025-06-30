#pragma once

#include "commands/common/UserCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace regexp {


class RegExpCommandProvider :
	public launcherapp::commands::common::UserCommandProviderBase
{
	RegExpCommandProvider();
	~RegExpCommandProvider() override;

public:
	CString GetName() override;

	// $B:n@.$G$-$k%3%^%s%I$N<oN`$rI=$9J8;zNs$r<hF@(B
	CString GetDisplayName() override;

	// $B%3%^%s%I$N<oN`$N@bL@$r<($9J8;zNs$r<hF@(B
	CString GetDescription() override;

	// $B%3%^%s%I?75,:n@.%@%$%"%m%0(B
	bool NewDialog(CommandParameter* param) override;

	// Provider$B4V$NM%@h=g0L$rI=$9CM$rJV$9!#>.$5$$$[$IM%@h(B
	uint32_t GetOrder() const override;
	// Provider$B$,07$&%3%^%s%I<oJL(B($BI=<(L>(B)$B$rNs5s(B
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(RegExpCommandProvider)
	DECLARE_LOADFROM(RegExpCommandProvider)
};


}
}
}

