#include "pch.h"
#include "framework.h"
#include "KeySplitterCommand.h"
#include "core/IFIDDefine.h"
#include "commands/keysplitter/KeySplitterParam.h"
#include "commands/keysplitter/KeySplitterCommandEditor.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "actions/core/ActionParameter.h"
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

CString KeySplitterCommand::GetGuideString()
{
	if (in->mGuideStr.IsEmpty()) {

		CString tmp;

		for (int i = 0; i < 16; ++i) {
			ModifierState state(i);

			ITEM item;
			if (in->mParam.GetMapping(state, item) == false) {
				continue;
			}

			if (tmp.IsEmpty() == FALSE) {
				tmp += _T(",");
			}
			tmp += state.ToString();
			tmp += _T(":");
			tmp += item.mCommandName;
		}
		in->mGuideStr= tmp;
	}
	return in->mGuideStr;
}

CString KeySplitterCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL KeySplitterCommand::Execute(Parameter* param)
{
	// Ctrlキーが押されているかを設定
	ModifierState state;
	state.SetPressCtrl((GetAsyncKeyState(VK_CONTROL) & 0x8000));
	state.SetPressShift((GetAsyncKeyState(VK_SHIFT) & 0x8000));
	state.SetPressAlt((GetAsyncKeyState(VK_MENU) & 0x8000));
	state.SetPressWin((GetAsyncKeyState(VK_LWIN) & 0x8000));

	ITEM item;
	if (in->mParam.GetMapping(state, item) == false) {
		// 割り当てがない場合は、無印Enterの扱い
		ModifierState state2;
		if (in->mParam.GetMapping(state2, item) == false) {
			// それもなければ何もしない
			return TRUE;
		}
	}

	CString parents;
	GetNamedParamString(param, _T("PARENTS"), parents);

	CString cmdName = in->mParam.mName;

	// 循環参照チェック
	int depth = 0;
	int n = 0;
	CString token = parents.Tokenize(_T("/"), n);
	while(token.IsEmpty() == FALSE) {

		if (depth >= 8) {
			// 深さは8まで
			return FALSE;
		}
		if (token == cmdName) {
			// 呼び出し元に自分自身がいる(循環参照)
			return FALSE;
		}
		token = parents.Tokenize(_T("/"), n);
		depth++;
	}
	// 呼び出し元に自分自身を追加
	if (parents.IsEmpty() == FALSE) {
		parents += _T("/");
	}
	parents += cmdName;

	RefPtr<ParameterBuilder> paramSub(ParameterBuilder::Create(), false);
	paramSub->SetParameterString(param->GetParameterString());
	paramSub->SetNamedParamString(_T("PARENTS"), parents);


	// 振り分け先のコマンドを実行する
	auto cmdRepo = CommandRepository::GetInstance();
	RefPtr<launcherapp::core::Command> command(cmdRepo->QueryAsWholeMatch(item.mCommandName, false));
	if (command == nullptr) {
		return TRUE;
	}

	return command->Execute(paramSub);
}

CString KeySplitterCommand::GetErrorString()
{
	return _T("");
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
	entry->Set(_T("description"), GetDescription());

	CString key;

	for (int i = 0; i < 16; ++i) {

		ModifierState state(i);
		ITEM item;
		bool hasItem = in->mParam.GetMapping(state, item);
		key.Format(_T("Use%d"), i);
		entry->Set(key, hasItem);

		if (hasItem == false) {
			continue;
		}

		key.Format(_T("Command%d"), i);
		entry->Set(key, item.mCommandName);
	}

	return true;
}

bool KeySplitterCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr !=GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));

	CString key;
	for (int i = 0; i < 16; ++i) {

		ModifierState state(i);
		key.Format(_T("Use%d"), i);
		bool hasEntry = entry->Get(key, false);

		if (hasEntry == false) {
			in->mParam.DeleteMapping(state);
			continue;
		}

		ITEM item;
		 

		key.Format(_T("Command%d"), i);
		item.mCommandName = entry->Get(key, _T(""));

		in->mParam.SetMapping(state, item);
	}

	return true;
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

