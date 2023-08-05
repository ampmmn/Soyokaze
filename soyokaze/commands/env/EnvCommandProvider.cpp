#include "pch.h"
#include "EnvCommandProvider.h"
#include "commands/env/EnvCommand.h"
#include "core/CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace env {


using CommandRepository = soyokaze::core::CommandRepository;

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


EnvCommandProvider::EnvCommandProvider() : in(new PImpl)
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
 	std::vector<CommandQueryItem>& commands
)
{
	CString cmdline = pattern->GetOriginalPattern();

	cmdline = cmdline.Trim();
	ASSERT(cmdline.IsEmpty() == FALSE);

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

	auto cmd = new EnvCommand(valName, value);
	commands.push_back(CommandQueryItem(Pattern::WholeMatch, cmd));
}


} // end of namespace env
} // end of namespace commands
} // end of namespace soyokaze

