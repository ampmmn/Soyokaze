#include "pch.h"
#include "framework.h"
#include "CommandMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CommandMap::CommandMap()
{
}

CommandMap::CommandMap(const CommandMap& rhs)
{
	for (auto& item : rhs.mMap) {
		mMap[item.first] = item.second->Clone();
	}
}

CommandMap::~CommandMap()
{
	Clear();
}

void CommandMap::Clear()
{
	for (auto& item : mMap) {
		item.second->Release();
	}
	mMap.clear();
}

bool CommandMap::Has(const CString& name) const
{
	return mMap.find(name) != mMap.end();
}

launcherapp::core::Command*
CommandMap::Get(const CString& name)
{
	auto itFind = mMap.find(name);
	if (itFind == mMap.end()) {
		return nullptr;
	}

	itFind->second->AddRef();
	return itFind->second;
}

void CommandMap::Register(launcherapp::core::Command* cmd)
{
	mMap[cmd->GetName()] = cmd;
}

bool CommandMap::Unregister(launcherapp::core::Command* cmd)
{
	return Unregister(cmd->GetName());
}

bool CommandMap::Unregister(const CString& name)
{
	auto itFind = mMap.find(name);
	if (itFind == mMap.end()) {
		return false;
	}

	itFind->second->Release();
	mMap.erase(itFind);
	return true;
}

// リネームによる登録しなおし
bool CommandMap::Reregister(launcherapp::core::Command* targetCmd)
{
	// 変更後の名前
	CString newName = targetCmd->GetName();

	auto it = mMap.begin();
	for (;it != mMap.end(); ++it) {
		auto& name = it->first;
		auto& cmd = it->second;

		// 変更対象のオブジェクトを探す
		if (targetCmd != cmd) {
			continue;
		}

		if (name == newName) {
			// 名前の変更なし
			return true;
		}
		
		// 変更処理
		mMap.erase(it);
		mMap[newName] = targetCmd;
		return true;
	}

	// 該当なし
	return false;
}

void CommandMap::Swap(CommandMap& rhs)
{
	mMap.swap(rhs.mMap);
}

void CommandMap::Query(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	for (auto& item : mMap) {

		auto& command = item.second;

		int matchLevel = command->Match(pattern);
		if (matchLevel == Pattern::Mismatch) {
			continue;
		}
		command->AddRef();
		commands.push_back(CommandQueryItem(matchLevel, command));
	}
}

// 最初に見つけた要素を返す
launcherapp::core::Command*
CommandMap::FindOne(Pattern* pattern)
{
	for (auto& item : mMap) {

		auto& command = item.second;
		if (command->Match(pattern) == Pattern::Mismatch) {
			continue;
		}
		item.second->AddRef();
		return item.second;
	}
	return nullptr;
}

std::vector<launcherapp::core::Command*>&
CommandMap::Enumerate(std::vector<launcherapp::core::Command*>& commands)
{
	commands.reserve(commands.size() + mMap.size());
	for (auto& item : mMap) {
		item.second->AddRef();
		commands.push_back(item.second);
	}
	return commands;
}

