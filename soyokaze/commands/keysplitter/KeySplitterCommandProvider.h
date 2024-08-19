#pragma once

#include "commands/common/RoutineCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace keysplitter {

// $B%-!<2!2<>uBV$K1~$8$F%3%^%s%I$rJ,$1$k$?$a$N%/%i%9(B
class KeySplitterCommandProvider :
	public launcherapp::commands::common::RoutineCommandProviderBase
{
private:
	KeySplitterCommandProvider();
	~KeySplitterCommandProvider() override;

public:
	CString GetName() override;

	// $B:n@.$G$-$k%3%^%s%I$N<oN`$rI=$9J8;zNs$r<hF@(B
	CString GetDisplayName() override;

	// $B%3%^%s%I$N<oN`$N@bL@$r<($9J8;zNs$r<hF@(B
	CString GetDescription() override;

	// $B%3%^%s%I?75,:n@.%@%$%"%m%0(B
	bool NewDialog(const CommandParameter* param) override;

	// Provider$B4V$NM%@h=g0L$rI=$9CM$rJV$9!#>.$5$$$[$IM%@h(B
	uint32_t GetOrder() const override;

	DECLARE_COMMANDPROVIDER(KeySplitterCommandProvider)

// RoutineCommandProviderBase
	DECLARE_LOADFROM(KeySplitterCommandProvider)
};

} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp
