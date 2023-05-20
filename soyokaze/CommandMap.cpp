#include "pch.h"
#include "framework.h"
#include "CommandMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CommandMap::QueryItem::QueryItem(int level, Command* cmd) : 
	mMatchLevel(level), mCommand(cmd)
{
}

CommandMap::CommandMap()
{
}

CommandMap::CommandMap(const CommandMap& rhs)
{
	for (auto item : rhs.mMap) {
		mMap[item.first] = item.second->Clone();
	}
}

CommandMap::~CommandMap()
{
	Clear();
}

void CommandMap::Clear()
{
	for (auto item : mMap) {
		delete item.second;
	}
}

bool CommandMap::Has(const CString& name) const
{
	return mMap.find(name) != mMap.end();
}

Command* CommandMap::Get(const CString& name)
{
	auto itFind = mMap.find(name);
	if (itFind == mMap.end()) {
		return nullptr;
	}
	return itFind->second;
}

void CommandMap::Register(Command* cmd)
{
	mMap[cmd->GetName()] = cmd;
}

bool CommandMap::Unregister(Command* cmd)
{
	return Unregister(cmd->GetName());
}

bool CommandMap::Unregister(const CString& name)
{
	auto itFind = mMap.find(name);
	if (itFind == mMap.end()) {
		return false;
	}

	delete itFind->second;
	mMap.erase(itFind);
	return true;
}

void CommandMap::Swap(CommandMap& rhs)
{
	mMap.swap(rhs.mMap);
}

void CommandMap::Query(
	Pattern* pattern,
	std::vector<QueryItem>& commands
)
{
	for (auto& item : mMap) {

		auto& command = item.second;

		int matchLevel = command->Match(pattern);
		if (matchLevel == Pattern::Mismatch) {
			continue;
		}
		
		commands.push_back(QueryItem(matchLevel, command));
	}
}

// 最初に見つけた要素を返す
Command* CommandMap::FindOne(Pattern* pattern)
{
	for (auto& item : mMap) {

		auto& command = item.second;
		if (command->Match(pattern) == Pattern::Mismatch) {
			continue;
		}
		return item.second;
	}
	return nullptr;
}

std::vector<Command*>& CommandMap::Enumerate(std::vector<Command*>& commands)
{
	commands.reserve(commands.size() + mMap.size());
	for (auto item : mMap) {
		commands.push_back(item.second);
	}
	return commands;
}

