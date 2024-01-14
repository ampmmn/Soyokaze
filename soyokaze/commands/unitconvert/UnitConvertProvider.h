#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace soyokaze {
namespace commands {
namespace unitconvert {


class UnitConvertProvider :
	public soyokaze::commands::common::AdhocCommandProviderBase
{
private:
	UnitConvertProvider();
	virtual ~UnitConvertProvider();

public:

	virtual CString GetName();
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	// 設定ページを取得する
	bool CreateSettingPages(CWnd* parent, std::vector<SettingPage*>& pages) override;

	DECLARE_COMMANDPROVIDER(UnitConvertProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace unitconvert
} // end of namespace commands
} // end of namespace soyokaze

