#include "pch.h"
#include "framework.h"
#include "KeySplitterCommand.h"
#include "core/IFIDDefine.h"
#include "commands/keysplitter/KeySplitterParam.h"
#include "commands/keysplitter/KeySplitterCommandEditor.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/CallbackAction.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace keysplitter {

using CommandRepository = launcherapp::core::CommandRepository;
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;


struct KeySplitterCommand::PImpl
{
	PImpl()
	{
	}
	~PImpl()
	{
	}

	CommandParam mParam;
	CString mGuideStr;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString KeySplitterCommand::GetType() { return _T("KeySplitterCommand"); }

KeySplitterCommand::KeySplitterCommand() : in(std::make_unique<PImpl>())
{
}

KeySplitterCommand::~KeySplitterCommand()
{
}

CString KeySplitterCommand::GetName()
{
	return in->mParam.mName;
}


CString KeySplitterCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString KeySplitterCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool KeySplitterCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	// Ctrlキーが押されているかを設定
	ModifierState state;
	if (modifierFlags & Command::MODIFIER_CTRL) { state.SetPressCtrl(true); }
	if (modifierFlags & Command::MODIFIER_SHIFT) { state.SetPressShift(true); }
	if (modifierFlags & Command::MODIFIER_ALT) { state.SetPressAlt(true); }
	if (modifierFlags & Command::MODIFIER_WIN) { state.SetPressWin(true); }

	bool isVisible = true;
	ITEM item;
	if (in->mParam.GetMapping(state, item) == false) {
		// 割り当てがない場合は、無印Enterの扱い
		isVisible = false;

		ModifierState stateNoModifier;
		if (in->mParam.GetMapping(stateNoModifier, item) == false) {
			// それもなければ何もしない
			return false;
		}
	}

	CString cmdName = in->mParam.mName;

	CString labelText(item.mActionName.IsEmpty() ? item.mCommandName : item.mActionName);

	auto a = new CallbackAction(labelText, [item, cmdName](Parameter* param, String* errMsg) -> bool {

		CString parents;
		GetNamedParamString(param, _T("PARENTS"), parents);

		// 循環参照チェック
		int depth = 0;
		int n = 0;
		CString token = parents.Tokenize(_T("/"), n);
		while(token.IsEmpty() == FALSE) {
	
			if (depth >= 8) {
				// 深さは8まで
				return false;
			}
			if (token == cmdName) {
				// 呼び出し元に自分自身がいる(循環参照)
				return false;
			}
			token = parents.Tokenize(_T("/"), n);
			depth++;
		}
		// 呼び出し元に自分自身を追加
		if (parents.IsEmpty() == FALSE) {
			parents += _T("/");
		}
		parents += cmdName;
	
		// 実行時引数を複製する
		RefPtr<ParameterBuilder> paramSub(ParameterBuilder::Create(), false);
		paramSub->SetParameterString(param->GetParameterString());
		paramSub->SetNamedParamString(_T("PARENTS"), parents);

		paramSub->SetNamedParamBool(_T("RUN_AS_BATCH"), true);
	
		// 振り分け先のコマンドを実行する
		auto cmdRepo = CommandRepository::GetInstance();
		RefPtr<launcherapp::core::Command> command(cmdRepo->QueryAsWholeMatch(item.mCommandName, false));
		if (command == nullptr) {
			if (errMsg) {
				std::string tmp;
				*errMsg = fmt::format("コマンドが見つかりません {}", UTF2UTF(item.mCommandName, tmp));
			}
			return false;
		}

		RefPtr<actions::core::Action> action;
		if (command->GetAction(0, &action) == false) {
			spdlog::error("Failed to get action.");
			return false;
		}
		return action->Perform(paramSub, errMsg);
	});

	// 無印Enterのガイドのみを表示する
	a->SetVisible(isVisible);

	*action = a;

	return true;
}

HICON KeySplitterCommand::GetIcon()
{
	return IconLoader::Get()->GetMMCndMgrIcon(-30608);
}

int KeySplitterCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool KeySplitterCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	UNREFERENCED_PARAMETER(attr);

	return false;
}

launcherapp::core::Command*
KeySplitterCommand::Clone()
{
	auto clonedObj = make_refptr<KeySplitterCommand>();

	clonedObj->in->mParam = in->mParam;

	return clonedObj.release();
}

bool KeySplitterCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());

	return in->mParam.Save(entry);
}

bool KeySplitterCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr !=GetType()) {
		return false;
	}

	return in->mParam.Load(entry);
}

bool KeySplitterCommand::NewDialog(Parameter* param)
{
	CString value;
	CommandParam paramTmp;

	if (GetNamedParamString(param, _T("COMMAND"), value)) {
		paramTmp.mName = value;
	}
	if (GetNamedParamString(param, _T("DESCRIPTION"), value)) {
		paramTmp.mDescription = value;
	}

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(paramTmp);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<KeySplitterCommand>();
	newCmd->SetParam(cmdEditor->GetParam());
		
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());

	return true;

}

void KeySplitterCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

// コマンドを編集するためのダイアログを作成/取得する
bool KeySplitterCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool KeySplitterCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_KEYSPLITTERCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool KeySplitterCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_KEYSPLITTERCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<KeySplitterCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

CString KeySplitterCommand::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("振り分け"));
	return TEXT_TYPE;
}


}
}
}

