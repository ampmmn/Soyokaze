#include "pch.h"
#include "UWPCommandProvider.h"
#include "commands/uwp/UWPApplicationItem.h"
#include "commands/uwp/UWPCommand.h"
#include "commands/uwp/UWPApplications.h"
#include "core/CommandRepository.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace uwp {

using CommandRepository = soyokaze::core::CommandRepository;

struct UWPCommandProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

	std::vector<ITEM> mItems;
	UWPApplications mUWPApps;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(UWPCommandProvider)


UWPCommandProvider::UWPCommandProvider() : in(std::make_unique<PImpl>())
{
}

UWPCommandProvider::~UWPCommandProvider()
{
}

CString UWPCommandProvider::GetName()
{
	return _T("UWPApps");
}

// 一時的なコマンドを必要に応じて提供する
void UWPCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	in->mUWPApps.GetApplications(in->mItems);

	for (auto& item : in->mItems) {

		int level = pattern->Match(item.mName);
		if (level == Pattern::Mismatch) {
			continue;
		}
		auto cmd = std::make_unique<UWPCommand>(item);

		commands.push_back(CommandQueryItem(level, cmd.release()));
	}
}


} // end of namespace uwp
} // end of namespace commands
} // end of namespace soyokaze

