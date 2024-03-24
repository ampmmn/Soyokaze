#include "pch.h"
#include "AlignWindowProvider.h"
#include "commands/align_window/AlignWindowCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace align_window {

using CommandRepository = soyokaze::core::CommandRepository;

struct AlignWindowProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

	uint32_t mRefCount = 1;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(AlignWindowProvider)


AlignWindowProvider::AlignWindowProvider() : in(std::make_unique<PImpl>())
{
}

AlignWindowProvider::~AlignWindowProvider()
{
}

// 初回起動の初期化を行う
void AlignWindowProvider::OnFirstBoot()
{
}

// コマンドの読み込み
void AlignWindowProvider::LoadCommands(CommandFile* cmdFile)
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

		AlignWindowCommand* command = nullptr;
		if (AlignWindowCommand::LoadFrom(cmdFile, entry, &command) == false) {
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


CString AlignWindowProvider::GetName()
{
	return _T("AlignWindowCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString AlignWindowProvider::GetDisplayName()
{
	return _LANG_T("Arrange Window Command");
}

// コマンドの種類の説明を示す文字列を取得
CString AlignWindowProvider::GetDescription()
{
	return _LANG_T("Description of aligncommand");
}

// コマンド新規作成ダイアログ
bool AlignWindowProvider::NewDialog(const CommandParameter* param)
{
	AlignWindowCommand* newCmd = nullptr;
	if (AlignWindowCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}
	CommandRepository::GetInstance()->RegisterCommand(newCmd);
	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool AlignWindowProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void AlignWindowProvider::QueryAdhocCommands(
	Pattern* pattern,
 	soyokaze::CommandQueryItemList& commands
)
{
	// サポートしない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t AlignWindowProvider::GetOrder() const
{
	return 1500;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool AlignWindowProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	return true;
}

uint32_t AlignWindowProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t AlignWindowProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace align_window
} // end of namespace commands
} // end of namespace soyokaze

