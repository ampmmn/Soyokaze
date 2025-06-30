#pragma once

#include "commands/common/UserCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace volumecontrol {

// $B2;NLD4@a%3%^%s%I$r@8@.$9$k$?$a$N%/%i%9(B
class VolumeCommandProvider :
	public launcherapp::commands::common::UserCommandProviderBase
{
private:
	VolumeCommandProvider();
	~VolumeCommandProvider() override;

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

	DECLARE_COMMANDPROVIDER(VolumeCommandProvider)

// UserCommandProviderBase
	DECLARE_LOADFROM(VolumeCommandProvider)
};

} // end of namespace volumecontrol
} // end of namespace commands
} // end of namespace launcherapp

