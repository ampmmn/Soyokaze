#include "pch.h"
#include "framework.h"
#include "KeySplitterCommand.h"
#include "commands/keysplitter/KeySplitterParam.h"
#include "commands/keysplitter/KeySplitterEditDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
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
	static CString TEXT_TYPE(_T("振り分け"));
	return TEXT_TYPE;
}

BOOL KeySplitterCommand::Execute(const Parameter& param)
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
	param.GetNamedParam(_T("PARENTS"), &parents);

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

	Parameter paramSub;
	param.CopyParamTo(paramSub);
	paramSub.SetNamedParamString(_T("PARENTS"), parents);


	// 振り分け先のコマンドを実行する
	auto cmdRepo = CommandRepository::GetInstance();
	auto command = cmdRepo->QueryAsWholeMatch(item.mCommandName, false);
	if (command == nullptr) {
		return TRUE;
	}
	BOOL isOK = command->Execute(paramSub);
	command->Release();

	return isOK;
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

int KeySplitterCommand::EditDialog(HWND parent)
{
	SettingDialog dlg(CWnd::FromHandle(parent));
	dlg.SetParam(in->mParam);

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	// 更新後の設定値を取得
	in->mParam = dlg.GetParam();

	in->mGuideStr.Empty();

	// 再登録
	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	return 0;
}

bool KeySplitterCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	UNREFERENCED_PARAMETER(attr);

	return false;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool KeySplitterCommand::IsPriorityRankEnabled()
{
	return false;
}

launcherapp::core::Command*
KeySplitterCommand::Clone()
{
	auto clonedObj = std::make_unique<KeySplitterCommand>();

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

bool KeySplitterCommand::NewDialog(const Parameter* param)
{
	param;  // 非サポート

	SettingDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	auto& paramNew = dlg.GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<KeySplitterCommand>();
	newCmd->SetParam(paramNew);
		
	bool isReloadHotKey = false;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);

	return true;

}

void KeySplitterCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

}
}
}

