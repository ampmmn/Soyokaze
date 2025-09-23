#include "pch.h"
#include "SnippetGroupCommand.h"
#include "core/IFIDDefine.h"
#include "commands/snippetgroup/SnippetGroupCommandEditor.h"
#include "commands/snippetgroup/SnippetGroupAdhocCommand.h"
#include "commands/core/CommandRepository.h"
#include "actions/mainwindow/MainWindowSetTextAction.h"
#include "utility/TimeoutChecker.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "hotkey/CommandHotKeyManager.h"
#include "mainwindow/controller/MainWindowController.h"

using SetTextAction = launcherapp::actions::mainwindow::SetTextAction;

namespace launcherapp {
namespace commands {
namespace snippetgroup {

struct SnippetGroupCommand::PImpl
{
	SnippetGroupParam mParam;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CString SnippetGroupCommand::GetType() { return _T("SnippetGroup"); }


SnippetGroupCommand::SnippetGroupCommand() : in(std::make_unique<PImpl>())
{
}

SnippetGroupCommand::~SnippetGroupCommand()
{
}

bool SnippetGroupCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (UserCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	return false;
}

void SnippetGroupCommand::SetParam(const SnippetGroupParam& param)
{
	in->mParam = param;
}

const SnippetGroupParam& SnippetGroupCommand::GetParam()
{
	return in->mParam;
}

CString SnippetGroupCommand::GetName()
{
	return in->mParam.mName;
}

CString SnippetGroupCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString SnippetGroupCommand::GetGuideString()
{
	return _T("キーワード入力すると候補を絞り込むことができます");
}

CString SnippetGroupCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool SnippetGroupCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		// コマンド名単体(後続のパラメータなし)で実行したときはグループ内の候補一覧を列挙させる
		*action = new SetTextAction(_T("キーワード入力すると候補を絞り込むことができます"), GetName() + _T(" "));
		return true;
	}
	return false;
}

HICON SnippetGroupCommand::GetIcon()
{
	// バインダーに矢印みたいなアイコン
	return IconLoader::Get()->GetImageResIcon(-5301);
}

int SnippetGroupCommand::Match(Pattern* pattern)
{
	if (pattern->shouldWholeMatch() && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定
		return Pattern::WholeMatch;
	}
	else if (pattern->shouldWholeMatch() == false) {
		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch || level == Pattern::PartialMatch) {
			return level;
		}

		if (level == Pattern::WholeMatch) {
			// 後続のキーワードが存在する場合は非表示
			return (pattern->GetWordCount() == 1) ? Pattern::WholeMatch : Pattern::HiddenMatch;
		}
	}

	// 通常はこちら
	return Pattern::Mismatch;
}

bool SnippetGroupCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
SnippetGroupCommand::Clone()
{
	auto clonedCmd = make_refptr<SnippetGroupCommand>();
	clonedCmd->in->mParam = in->mParam;
	return clonedCmd.release();
}

bool SnippetGroupCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);
	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);
}

bool SnippetGroupCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}

	SnippetGroupParam paramTmp;
	paramTmp.Load(entry);

	if (paramTmp == in->mParam) {
		// 変化がなければ何もしない
		return true;
	}

	in->mParam.swap(paramTmp);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	return true;
}


bool SnippetGroupCommand::NewDialog(
	Parameter* param,
	SnippetGroupCommand** newCmdPtr
)
{
	// 新規作成ダイアログを表示
	CString value;
	SnippetGroupParam paramTmp;

	if (GetNamedParamString(param, _T("COMMAND"), value)) {
		paramTmp.mName = value;
	}
	if (GetNamedParamString(param, _T("DESCRIPTION"), value)) {
		paramTmp.mDescription = value;
	}

	// 新規作成ダイアログを表示
	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(paramTmp);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = cmdEditor->GetParam();
	auto newCmd = make_refptr<SnippetGroupCommand>();
	newCmd->in->mParam = commandParam;

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool SnippetGroupCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new CommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool SnippetGroupCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_SNIPPETGROUPCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool SnippetGroupCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_SNIPPETGROUPCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<SnippetGroupCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}


/**
 	コマンドの候補として追加表示する項目を取得する
 	@return true:取得成功   false:取得失敗(表示しない)
 	@param[in]  pattern  入力パターン
 	@param[out] commands 表示する候補
*/
bool SnippetGroupCommand::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	utility::TimeoutChecker tm(100);  // 100msecでタイムアウト

	if (in->mParam.mName.CompareNoCase(pattern->GetFirstWord()) != 0) {
		// コマンド名が一致しない場合は検索対象外
		return false;
	}

	int hitCount = 0;
	int limit = 20;

	// 辞書データをひとつずつ比較する
	for (const auto& item : in->mParam.mItems) {

		if (tm.IsTimeout()) {
			// 一定時間たっても終わらなかったらあきらめる
			break;
		}

		int level = pattern->Match(item.mName, 1);
		if (level == Pattern::Mismatch) {
			continue;
		}

		if (level == Pattern::PartialMatch) {
			// 最低でも前方一致扱いにする(先頭のコマンド名は合致しているため)
			level = Pattern::FrontMatch;
		}

		commands.Add(CommandQueryItem(level, new SnippetGroupAdhocCommand(in->mParam, item)));
		if (++hitCount >= limit) {
			break;
		}
	}
	return hitCount > 0;
}

/**
 	追加候補を表示するために内部でキャッシュしているものがあれば、それを削除する
*/
void SnippetGroupCommand::ClearCache()
{
}

CString SnippetGroupCommand::TypeDisplayName()
{
	return _T("定型文グループ");
}


} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

