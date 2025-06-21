#include "pch.h"
#include "framework.h"
#include "CommandMap.h"
#include "commands/core/CommandFile.h"
#include <set>

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

void CommandMap::LoadSettings(Settings& settings)
{
	for (auto& item : mMap) {
		auto cmd = item.second;
		settings.Add(cmd);
	}
}

void CommandMap::RestoreSettings(Settings& settings)
{
	std::set<CString> eraseTargets;
	for (auto& item : mMap) {
		auto cmd = item.second;
		if (settings.Restore(cmd) == false) {
			// リストアできなかったものは、リストア用データ作成時点で存在していなかった
			// (=後から追加された)ものなので、消す
			eraseTargets.insert(item.first);
		}
	}

	for (auto& target : eraseTargets) {
		Unregister(target);
	}
}

void CommandMap::Clear()
{
	for (auto& item : mMap) {
		item.second->Release();
	}
	mMap.clear();
}

// コマンドオブジェクトに紐づけられた名前を問い合わせる
bool CommandMap::QueryRegisteredNameFor(launcherapp::core::Command* targetCmd, CString& registeredName)
{
	auto it = mMap.begin();
	for (;it != mMap.end(); ++it) {
		auto& name = it->first;
		auto& cmd = it->second;

		// 変更対象のオブジェクトを探す
		if (targetCmd != cmd) {
			continue;
		}

		registeredName = name;
		return true;
	}

	// 該当なし
	return false;
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
		commands.Add(CommandQueryItem(matchLevel, command));
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct CommandMap::Settings::PImpl
{
	CommandFile mCommandFile;
	std::map<launcherapp::core::Command*, CommandEntryIF*> mEntryMap;
};

CommandMap::Settings::Settings() : in(new PImpl)
{
}

CommandMap::Settings::~Settings()
{
}

void CommandMap::Settings::Add(launcherapp::core::Command* cmd)
{
	ASSERT(cmd);

	auto entry = in->mCommandFile.NewEntry(cmd->GetName());
	cmd->Save(entry);
	in->mEntryMap[cmd] = entry;

}

bool CommandMap::Settings::Restore(launcherapp::core::Command* cmd)
{
	ASSERT(cmd);

	auto it = in->mEntryMap.find(cmd);
	if (it == in->mEntryMap.end()) {
		SPDLOG_INFO(_T("Entry does not exist. {}"), (LPCTSTR)cmd->GetName());
		return false;
	}

	auto entry = it->second;
	if (cmd->Load(entry) == false) {
		SPDLOG_WARN(_T("Failed to restore. {}"), (LPCTSTR)cmd->GetName());
	}

	in->mEntryMap.erase(it);

	return true;
}

