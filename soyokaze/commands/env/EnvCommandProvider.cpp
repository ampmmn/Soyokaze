#include "pch.h"
#include "EnvCommandProvider.h"
#include "commands/env/EnvCommand.h"
#include "commands/core/CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace env {


using CommandRepository = launcherapp::core::CommandRepository;

struct EnvCommandProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(EnvCommandProvider)


EnvCommandProvider::EnvCommandProvider() : in(std::make_unique<PImpl>())
{
}

EnvCommandProvider::~EnvCommandProvider()
{
}

CString EnvCommandProvider::GetName()
{
	return _T("Env");
}

// 一時的なコマンドを必要に応じて提供する
void EnvCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	CString cmdline = pattern->GetWholeString();

	cmdline = cmdline.Trim();
	if (cmdline.IsEmpty()) {
		return;
	}

	if (cmdline.GetLength() < 3) {
		return;
	}
	if (cmdline[0] != _T('%') || cmdline[cmdline.GetLength()-1] != _T('%')) {
		return;
	}

	CString valName = cmdline.Mid(1, cmdline.GetLength()-2);

	size_t reqLen = 0;
	if (_tgetenv_s(&reqLen, nullptr, 0, valName) != 0) {
		return ;
	}
	if (reqLen == 0) {
		return;
	}

	CString value;
	_tgetenv_s(&reqLen, value.GetBuffer((int)reqLen), reqLen, valName);
	value.ReleaseBuffer();

	commands.Add(CommandQueryItem(Pattern::WholeMatch, new EnvCommand(valName, value)));
}


} // end of namespace env
} // end of namespace commands
} // end of namespace launcherapp

