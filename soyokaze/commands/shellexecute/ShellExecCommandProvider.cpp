#include "pch.h"
#include "ShellExecCommandProvider.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "core/CommandHotKeyManager.h"
#include "commands/shellexecute/ShellExecSettingDialog.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "CommandHotKeyMappings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = soyokaze::core::CommandRepository;

namespace soyokaze {
namespace commands {
namespace shellexecute {


struct ShellExecCommandProvider::PImpl
{
	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ShellExecCommandProvider)


ShellExecCommandProvider::ShellExecCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mRefCount = 1;
}

ShellExecCommandProvider::~ShellExecCommandProvider()
{
}

// 初回起動の初期化を行う
void ShellExecCommandProvider::OnFirstBoot()
{
	// 特に何もしない
}


// コマンドの読み込み
void ShellExecCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	ASSERT(cmdFile);

	auto cmdRepo = CommandRepository::GetInstance();

	int entries = cmdFile->GetEntryCount();
	for (int i = 0; i < entries; ++i) {

		auto entry = cmdFile->GetEntry(i);
		if (cmdFile->IsUsedEntry(entry)) {
			// 既にロード済(使用済)のエントリ
			continue;
		}

		ShellExecCommand* command = nullptr;
		if (ShellExecCommand::LoadFrom(cmdFile, entry, &command) == false) {
			if (command) {
				command->Release();
			}
			continue;
		}

		// 登録
		cmdRepo->RegisterCommand(command);

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);

	}
}

CString ShellExecCommandProvider::GetName()
{
	return _T("ShellExecuteCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString ShellExecCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_NORMALCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString ShellExecCommandProvider::GetDescription()
{
	return CString((LPCTSTR)IDS_DESCRIPTION_NORMALCOMMAND);
}

// コマンド新規作成ダイアログ
bool ShellExecCommandProvider::NewDialog(const CommandParameter* param)
{
	ShellExecCommand* newCmd = nullptr;
	if (ShellExecCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}
	CommandRepository::GetInstance()->RegisterCommand(newCmd);
	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool ShellExecCommandProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void ShellExecCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t ShellExecCommandProvider::GetOrder() const
{
	return 100;
}

uint32_t ShellExecCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t ShellExecCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

}
}
}
