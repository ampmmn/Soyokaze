#include "pch.h"
#include "MailToCommandProvider.h"
#include "commands/mailto/MailToCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace mailto {


using CommandRepository = soyokaze::core::CommandRepository;

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
		commands.push_back(CommandQueryItem(level, in->mCommandPtr));
	}
}

} // end of namespace mailto
} // end of namespace commands
} // end of namespace soyokaze

