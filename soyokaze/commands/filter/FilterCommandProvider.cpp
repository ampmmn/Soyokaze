#include "pch.h"
#include "FilterCommandProvider.h"
#include "commands/filter/FilterCommand.h"
#include "commands/filter/FilterCommandParam.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "core/CommandHotKeyManager.h"
#include "FilterEditDialog.h"
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
namespace filter {


struct FilterCommandProvider::PImpl
{
	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(FilterCommandProvider)


FilterCommandProvider::FilterCommandProvider() : in(new PImpl)
{
	in->mRefCount = 1;
}

FilterCommandProvider::~FilterCommandProvider()
{
}

// 初回起動の初期化を行う
void FilterCommandProvider::OnFirstBoot()
{
	// 特に何もしない
}


// コマンドの読み込み
void FilterCommandProvider::LoadCommands(
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

		CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
		if (typeStr.IsEmpty() == FALSE && typeStr != FilterCommand::GetType()) {
			continue;
		}

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);

		CommandParam newParam;
		newParam.mName = cmdFile->GetName(entry);
		newParam.mDescription = cmdFile->Get(entry, _T("description"), _T(""));

		newParam.mPath = cmdFile->Get(entry, _T("path"), _T(""));
		newParam.mDir = cmdFile->Get(entry, _T("dir"), _T(""));
		newParam.mParameter = cmdFile->Get(entry, _T("parameter"), _T(""));
		newParam.mShowType = cmdFile->Get(entry, _T("show"), SW_HIDE);
		newParam.mPreFilterType = cmdFile->Get(entry, _T("prefiltertype"), 0);
		newParam.mPostFilterType = cmdFile->Get(entry, _T("aftertype"), 0);
		newParam.mAfterCommandName = cmdFile->Get(entry, _T("aftercommand"), _T(""));
		newParam.mAfterFilePath = cmdFile->Get(entry, _T("afterfilepath"), _T(""));
		newParam.mAfterCommandParam = cmdFile->Get(entry, _T("afterparam"), _T("$select"));

		auto command = new FilterCommand();
		command->SetParam(newParam);
		// 登録
		cmdRepo->RegisterCommand(command);
	}
}

CString FilterCommandProvider::GetName()
{
	return _T("FilterCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString FilterCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_FILTERCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString FilterCommandProvider::GetDescription()
{
	return CString((LPCTSTR)IDS_DESCRIPTION_FILTERCOMMAND);
}

// コマンド新規作成ダイアログ
bool FilterCommandProvider::NewDialog(const CommandParameter* param)
{
	// 新規作成ダイアログを表示
	CString value;

	CommandParam tmpParam;

	if (param && param->GetNamedParam(_T("COMMAND"), &value)) {
		tmpParam.mName = value;
	}
	if (param && param->GetNamedParam(_T("PATH"), &value)) {
		tmpParam.mPath = value;
	}
	if (param && param->GetNamedParam(_T("DESCRIPTION"), &value)) {
		tmpParam.mDescription = value;
	}
	if (param && param->GetNamedParam(_T("ARGUMENT"), &value)) {
		tmpParam.mParameter = value;
	}

	FilterEditDialog dlg;
	dlg.SetParam(tmpParam);

	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = new FilterCommand();

	dlg.GetParam(tmpParam);
	newCmd->SetParam(tmpParam);

	CommandRepository::GetInstance()->RegisterCommand(newCmd);

	// ホットキー設定を更新
	if (dlg.mHotKeyAttr.IsValid()) {

		auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
		CommandHotKeyMappings hotKeyMap;
		hotKeyManager->GetMappings(hotKeyMap);

		hotKeyMap.AddItem(tmpParam.mName, dlg.mHotKeyAttr);

		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		pref->Save();
	}

	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool FilterCommandProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void FilterCommandProvider::QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& comands)
{
	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t FilterCommandProvider::FilterCommandProvider::GetOrder() const
{
	return 400;
}

uint32_t FilterCommandProvider::FilterCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t FilterCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze

