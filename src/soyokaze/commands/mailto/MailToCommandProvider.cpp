#include "pch.h"
#include "MailToCommandProvider.h"
#include "commands/mailto/MailToCommand.h"
#include "commands/core/CommandRepository.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace mailto {


using CommandRepository = launcherapp::core::CommandRepository;

struct MailToCommandProvider::PImpl
{
	MailToCommand* mCommandPtr;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(MailToCommandProvider)


MailToCommandProvider::MailToCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mCommandPtr = new MailToCommand();
}

MailToCommandProvider::~MailToCommandProvider()
{
	if (in->mCommandPtr) {
		in->mCommandPtr->Release();
	}
}

CString MailToCommandProvider::GetName()
{
	return _T("MailToCommand");
}

// 一時的なコマンドを必要に応じて提供する
void MailToCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	int level = in->mCommandPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mCommandPtr->AddRef();
		commands.Add(CommandQueryItem(level, in->mCommandPtr));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t MailToCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(MailToCommand::TypeDisplayName());
	return 1;
}

} // end of namespace mailto
} // end of namespace commands
} // end of namespace launcherapp

