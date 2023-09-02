#include "pch.h"
#include "DecodeStringCommandProvider.h"
#include "commands/decodestring/DecodeUriCommand.h"
#include "core/CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace decodestring {


using CommandRepository = soyokaze::core::CommandRepository;

struct DecodeStringCommandProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

	std::unique_ptr<DecodeUriCommand> mDecodeUriCommand;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(DecodeStringCommandProvider)


DecodeStringCommandProvider::DecodeStringCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mDecodeUriCommand = std::make_unique<DecodeUriCommand>();
}

DecodeStringCommandProvider::~DecodeStringCommandProvider()
{
}

CString DecodeStringCommandProvider::GetName()
{
	return _T("DecodeString");
}

// 一時的なコマンドを必要に応じて提供する
void DecodeStringCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	int level = in->mDecodeUriCommand->Match(pattern);
	if (level != Pattern::Mismatch) {
		commands.push_back(CommandQueryItem(level, in->mDecodeUriCommand.get()));
		in->mDecodeUriCommand->AddRef();
	}
}


} // end of namespace decodestring
} // end of namespace commands
} // end of namespace soyokaze

