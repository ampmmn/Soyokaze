#include "ProxyCommandRepository.h"
#include "ProxyCommand.h"
#include <map>

namespace launcherproxy {

struct ProxyCommandRepository::PImpl
{
	std::map<std::string, ProxyCommand*> mCommands;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


ProxyCommandRepository::ProxyCommandRepository() : in(new PImpl)
{
}

ProxyCommandRepository::~ProxyCommandRepository()
{
	for (auto& item : in->mCommands) {
		delete item.second;
	}
}

ProxyCommandRepository* ProxyCommandRepository::GetInstance()
{
	static ProxyCommandRepository inst;
	return &inst;
}

bool ProxyCommandRepository::Register(ProxyCommand* command)
{
	auto name = command->GetName();

	auto it = in->mCommands.find(name);
	if (it != in->mCommands.end()) {
		delete it->second;
		in->mCommands.erase(it);
	}
	in->mCommands[name] = command;
	return true;
}

ProxyCommand* ProxyCommandRepository::GetCommand(const std::string& name)
{
	auto it = in->mCommands.find(name);
	if (it == in->mCommands.end()) {
		return nullptr;
	}
	return it->second;
}


} // end of namespace launcherproxy
