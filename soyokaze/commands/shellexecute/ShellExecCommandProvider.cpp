#include "pch.h"
#include "ShellExecCommandProvider.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "core/CommandHotKeyManager.h"
#include "commands/shellexecute/CommandEditDialog.h"
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


ShellExecCommandProvider::ShellExecCommandProvider() : in(new PImpl)
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

		CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
		if (typeStr.IsEmpty() == FALSE && typeStr != ShellExecCommand::GetType()) {
			continue;
		}

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);

		ShellExecCommand::ATTRIBUTE normalAttr;
		ShellExecCommand::ATTRIBUTE noParamAttr;

		CString name = cmdFile->GetName(entry);
		CString descriptionStr = cmdFile->Get(entry, _T("description"), _T(""));
		int runAs = cmdFile->Get(entry, _T("runas"), 0);

		normalAttr.mPath = cmdFile->Get(entry, _T("path"), _T(""));
		normalAttr.mDir = cmdFile->Get(entry, _T("dir"), _T(""));
		normalAttr.mParam = cmdFile->Get(entry, _T("parameter"), _T(""));
		normalAttr.mShowType = cmdFile->Get(entry, _T("show"), normalAttr.mShowType);

		noParamAttr.mPath = cmdFile->Get(entry, _T("path0"), _T(""));
		noParamAttr.mDir = cmdFile->Get(entry, _T("dir0"), _T(""));
		noParamAttr.mParam = cmdFile->Get(entry, _T("parameter0"), _T(""));
		noParamAttr.mShowType = cmdFile->Get(entry, _T("show0"), noParamAttr.mShowType);

		auto command = new ShellExecCommand();
		command->SetName(name);
		command->SetDescription(descriptionStr);
		command->SetRunAs(runAs);
		if (normalAttr.mPath.IsEmpty() == FALSE) {
			command->SetAttribute(normalAttr);
		}
		if (noParamAttr.mPath.IsEmpty() == FALSE) {
			command->SetAttributeForParam0(noParamAttr);
		}

		// 登録
		cmdRepo->RegisterCommand(command);
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
	// 新規作成ダイアログを表示
	CString value;

	CommandEditDialog dlg;
	if (param && param->GetNamedParam(_T("COMMAND"), &value)) {
		dlg.SetName(value);
	}
	if (param && param->GetNamedParam(_T("PATH"), &value)) {
		dlg.SetPath(value);
	}
	if (param && param->GetNamedParam(_T("DESCRIPTION"), &value)) {
		dlg.SetDescription(value);
	}
	if (param && param->GetNamedParam(_T("ARGUMENT"), &value)) {
		dlg.SetParam(value);
	}

	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = new ShellExecCommand();
	newCmd->SetName(dlg.mName);
	newCmd->SetDescription(dlg.mDescription);
	newCmd->SetRunAs(dlg.mIsRunAsAdmin);

	ShellExecCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath = dlg.mPath;
	normalAttr.mParam = dlg.mParameter;
	normalAttr.mDir = dlg.mDir;
	normalAttr.mShowType = dlg.GetShowType();
	newCmd->SetAttribute(normalAttr);

	if (dlg.mIsUse0) {
		ShellExecCommand::ATTRIBUTE param0Attr;
		param0Attr.mPath = dlg.mPath0;
		param0Attr.mParam = dlg.mParameter0;
		param0Attr.mDir = dlg.mDir;
		param0Attr.mShowType = dlg.GetShowType();
		newCmd->SetAttributeForParam0(param0Attr);
	}
	else {
		ShellExecCommand::ATTRIBUTE param0Attr;
		newCmd->SetAttributeForParam0(param0Attr);
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd);

	// ホットキー設定を更新
	if (dlg.mHotKeyAttr.IsValid()) {

		auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
		CommandHotKeyMappings hotKeyMap;
		hotKeyManager->GetMappings(hotKeyMap);

		hotKeyMap.AddItem(dlg.mName, dlg.mHotKeyAttr);

		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		pref->Save();
	}

	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool ShellExecCommandProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void ShellExecCommandProvider::QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& comands)
{
	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t ShellExecCommandProvider::ShellExecCommandProvider::GetOrder() const
{
	return 100;
}

uint32_t ShellExecCommandProvider::ShellExecCommandProvider::AddRef()
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
