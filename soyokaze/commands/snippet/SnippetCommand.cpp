#include "pch.h"
#include "framework.h"
#include "SnippetCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/snippet/SnippetCommandEditDialog.h"
#include "core/CommandRepository.h"
#include "core/CommandHotKeyManager.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;

namespace soyokaze {
namespace commands {
namespace snippet {

using CommandRepository = soyokaze::core::CommandRepository;

struct SnippetCommand::PImpl
{
	PImpl() : mRefCount(1)
	{
	}
	~PImpl()
	{
	}

	CString mName;
	CString mDescription;
	CString mText;

	// 参照カウント
	uint32_t mRefCount;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString SnippetCommand::GetType() { return _T("Snippet"); }

SnippetCommand::SnippetCommand() : in(std::make_unique<PImpl>())
{
}

SnippetCommand::~SnippetCommand()
{
}

CString SnippetCommand::GetName()
{
	return in->mName;
}


CString SnippetCommand::GetDescription()
{
	return in->mDescription;
}

CString SnippetCommand::GetGuideString()
{
	return _T("Enter:定型文をクリップボードにコピー");
}

CString SnippetCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_SNIPPETCOMMAND);
	return TEXT_TYPE;
}

BOOL SnippetCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	Clipboard::Copy(in->mText);

	return TRUE;
}

CString SnippetCommand::GetErrorString()
{
	return _T("");
}

SnippetCommand& SnippetCommand::SetName(LPCTSTR name)
{
	in->mName = name;
	return *this;
}

SnippetCommand& SnippetCommand::SetDescription(LPCTSTR description)
{
	in->mDescription = description;
	return *this;
}


SnippetCommand& SnippetCommand::SetText(const CString& text)
{
	in->mText = text;
	return *this;
}

HICON SnippetCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5301);
}

int SnippetCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool SnippetCommand::IsEditable()
{
	return true;
}

int SnippetCommand::EditDialog(const Parameter* param)
{
	CommandEditDialog dlg;
	dlg.SetOrgName(in->mName);

	dlg.mName = in->mName;
	dlg.mDescription = in->mDescription;
	dlg.mText = in->mText;

	auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
	HOTKEY_ATTR hotKeyAttr;
	bool isGlobal = false;
	if (hotKeyManager->HasKeyBinding(in->mName, &hotKeyAttr, &isGlobal)) {
		dlg.mHotKeyAttr = hotKeyAttr;
		dlg.mIsGlobal = isGlobal;
	}
	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	auto cmdNew = std::make_unique<SnippetCommand>();

	// 追加する処理
	cmdNew->SetName(dlg.mName);
	cmdNew->SetDescription(dlg.mDescription);
	cmdNew->SetText(dlg.mText);

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->UnregisterCommand(this);
	cmdRepo->RegisterCommand(cmdNew.release());

	// ホットキー設定を更新
	CommandHotKeyMappings hotKeyMap;
	hotKeyManager->GetMappings(hotKeyMap);

	hotKeyMap.RemoveItem(hotKeyAttr);
	if (dlg.mHotKeyAttr.IsValid()) {
		hotKeyMap.AddItem(dlg.mName, dlg.mHotKeyAttr, dlg.mIsGlobal);
	}

	auto pref = AppPreference::Get();
	pref->SetCommandKeyMappings(hotKeyMap);
	pref->Save();

	return 0;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool SnippetCommand::IsPriorityRankEnabled()
{
	return true;
}

soyokaze::core::Command*
SnippetCommand::Clone()
{
	auto clonedObj = std::make_unique<SnippetCommand>();

	clonedObj->in->mName = in->mName;
	clonedObj->in->mDescription = in->mDescription;
	clonedObj->in->mText = in->mText;

	return clonedObj.release();
}

bool SnippetCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());

	cmdFile->Set(entry, _T("description"), GetDescription());
	cmdFile->Set(entry, _T("text"), in->mText);

	return true;
}

bool SnippetCommand::Load(CommandFile* cmdFile, void* entry_)
{
	auto entry = (CommandFile::Entry*)entry_;

	in->mName = cmdFile->GetName(entry);
	in->mDescription = cmdFile->Get(entry, _T("description"), _T(""));
	in->mText = cmdFile->Get(entry, _T("text"), _T(""));

	return true;
}

uint32_t SnippetCommand::AddRef()
{
	return ++in->mRefCount;
}

uint32_t SnippetCommand::Release()
{
	auto n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

bool SnippetCommand::NewDialog(const Parameter* param)
{
	// 新規作成ダイアログを表示
	CString value;

	CommandEditDialog dlg;
	if (param && param->GetNamedParam(_T("COMMAND"), &value)) {
		dlg.SetName(value);
	}
	if (param && param->GetNamedParam(_T("DESCRIPTION"), &value)) {
		dlg.SetDescription(value);
	}
	if (param && param->GetNamedParam(_T("TEXT"), &value)) {
		dlg.mText = value;
	}
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<SnippetCommand>();
	newCmd->in->mName = dlg.mName;
	newCmd->in->mDescription = dlg.mDescription;
	newCmd->in->mText = dlg.mText;

	// ホットキー設定を更新
	if (dlg.mHotKeyAttr.IsValid()) {

		auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
		CommandHotKeyMappings hotKeyMap;
		hotKeyManager->GetMappings(hotKeyMap);

		hotKeyMap.AddItem(dlg.mName, dlg.mHotKeyAttr, dlg.mIsGlobal);

		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		pref->Save();
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());
	return true;

}

}
}
}

