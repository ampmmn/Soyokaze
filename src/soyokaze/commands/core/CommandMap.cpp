#include "pch.h"
#include "framework.h"
#include "CommandMap.h"
#include "commands/core/CommandFile.h"
#include <set>
#pragma warning(push)
#pragma warning(disable: 4995)
#pragma warning(disable: 4324)
#include <absl/container/btree_map.h>
#pragma warning(pop)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct CommandMap::PImpl
{
	absl::btree_map<CString, launcherapp::core::Command*> mMap;
};

CommandMap::CommandMap() : in(new PImpl)
{
}

// コピーコンストラクタ
// 複製はするがコマンドオブジェクトは同じものを参照する
// 参照カウントは+1する
CommandMap::CommandMap(const CommandMap& rhs) : in(new PImpl)
{
	for (auto& item : rhs.in->mMap) {
		auto cmd = item.second;
		in->mMap[item.first] = cmd;
		cmd->AddRef();
	}
}

CommandMap::~CommandMap()
{
	Clear();
}

void CommandMap::LoadSettings(Settings& settings)
{
	for (auto& item : in->mMap) {
		auto cmd = item.second;
		settings.Add(cmd);
	}
}

void CommandMap::RestoreSettings(Settings& settings)
{
	std::set<CString> eraseTargets;
	for (auto& item : in->mMap) {
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
	for (auto& item : in->mMap) {
		item.second->Release();
	}
	in->mMap.clear();
}

// コマンドオブジェクトに紐づけられた名前を問い合わせる
bool CommandMap::QueryRegisteredNameFor(launcherapp::core::Command* targetCmd, CString& registeredName)
{
	auto it = in->mMap.begin();
	for (;it != in->mMap.end(); ++it) {
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
	return in->mMap.find(name) != in->mMap.end();
}

launcherapp::core::Command*
CommandMap::Get(const CString& name)
{
	auto itFind = in->mMap.find(name);
	if (itFind == in->mMap.end()) {
		return nullptr;
	}

	itFind->second->AddRef();
	return itFind->second;
}

void CommandMap::Register(launcherapp::core::Command* cmd)
{
	in->mMap[cmd->GetName()] = cmd;
}

bool CommandMap::Unregister(launcherapp::core::Command* cmd)
{
	return Unregister(cmd->GetName());
}

bool CommandMap::Unregister(const CString& name)
{
	auto itFind = in->mMap.find(name);
	if (itFind == in->mMap.end()) {
		return false;
	}

	itFind->second->Release();
	in->mMap.erase(itFind);
	return true;
}

// リネームによる登録しなおし
bool CommandMap::Reregister(launcherapp::core::Command* targetCmd)
{
	// 変更後の名前
	CString newName = targetCmd->GetName();

	auto it = in->mMap.begin();
	for (;it != in->mMap.end(); ++it) {
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
		in->mMap.erase(it);
		in->mMap[newName] = targetCmd;
		return true;
	}

	// 該当なし
	return false;
}

void CommandMap::Swap(CommandMap& rhs)
{
	in->mMap.swap(rhs.in->mMap);
}

void CommandMap::Query(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	for (auto& item : in->mMap) {

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
	for (auto& item : in->mMap) {

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
	commands.reserve(commands.size() + in->mMap.size());
	for (auto& item : in->mMap) {
		item.second->AddRef();
		commands.push_back(item.second);
	}
	return commands;
}

size_t CommandMap::GetSize() const
{
	return in->mMap.size();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct CommandMap::Settings::PImpl
{
	CommandFile mCommandFile;
	absl::btree_map<launcherapp::core::Command*, CommandEntryIF*> mEntryMap;
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

