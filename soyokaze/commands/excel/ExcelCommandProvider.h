#pragma once

#include "core/CommandProviderIF.h"


namespace soyokaze {
namespace commands {
namespace excel {


class ExcelCommandProvider :
	public soyokaze::core::CommandProvider
{
	using Command = soyokaze::core::Command;
	using CommandParameter = soyokaze::core::CommandParameter;

private:
	ExcelCommandProvider();
	virtual ~ExcelCommandProvider();

public:
	// $B=i2s5/F0$N=i4|2=$r9T$&(B
	virtual void OnFirstBoot();

	// $B%3%^%s%I$NFI$_9~$_(B
	virtual void LoadCommands(CommandFile* commandFile);

	virtual CString GetName();

	// $B:n@.$G$-$k%3%^%s%I$N<oN`$rI=$9J8;zNs$r<hF@(B
	virtual CString GetDisplayName();

	// $B%3%^%s%I$N<oN`$N@bL@$r<($9J8;zNs$r<hF@(B
	virtual CString GetDescription();

	// $B%3%^%s%I?75,:n@.%@%$%"%m%0(B
	virtual bool NewDialog(const CommandParameter* param);

	// $BHs8x3+%3%^%s%I$+$I$&$+(B($B?75,:n@.BP>]$K$7$J$$(B)
	virtual bool IsPrivate() const;

	// $B0l;~E*$J%3%^%s%I$rI,MW$K1~$8$FDs6!$9$k(B
	virtual void QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& comands);

	// Provider$B4V$NM%@h=g0L$rI=$9CM$rJV$9!#>.$5$$$[$IM%@h(B
	virtual uint32_t GetOrder() const;

	virtual uint32_t AddRef();
	virtual uint32_t Release();

	DECLARE_COMMANDPROVIDER(ExcelCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace excel
} // end of namespace commands
} // end of namespace soyokaze
