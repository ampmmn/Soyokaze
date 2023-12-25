#include "pch.h"
#include "AlignWindowCommand.h"
#include "commands/align_window/AlignWindowCommandParam.h"
#include "commands/align_window/AlignWindowSettingDialog.h"
#include "core/CommandRepository.h"
#include "core/CommandHotKeyManager.h"
#include "utility/ScopeAttachThreadInput.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"
#include "commands/common/Message.h"
#include <assert.h>
#include <regex>

namespace soyokaze {
namespace commands {
namespace align_window {

struct AlignWindowCommand::PImpl
{
	CommandParam mParam;
	uint32_t mRefCount = 1;
};

CString AlignWindowCommand::GetType() { return _T("AlignWindow"); }


/**
 	コンストラクタ
*/
 AlignWindowCommand::AlignWindowCommand() : in(std::make_unique<PImpl>())
{
}

/**
 	デストラクタ
*/
 AlignWindowCommand::~AlignWindowCommand()
{
}

/**
 	コマンド名を取得する
 	@return コマンド名
*/
CString AlignWindowCommand::GetName()
{
	return in->mParam.mName;
}

/**
 	コマンドの説明文字列を取得する
 	@return 説明文字列
*/
CString AlignWindowCommand::GetDescription()
{
	return in->mParam.mDescription;
}

/**
 	ガイド欄文字列を取得する
 	@return ガイド欄文字列
*/
CString AlignWindowCommand::GetGuideString()
{
	return _T("Enter:ウインドウを整列する");
}


/**
 	コマンド種別を表す文字列を取得する
 	@return コマンド種別
*/
CString AlignWindowCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE(_T("ウインドウ整列"));
	return TEXT_TYPE;
}

/**
 * @brief 直前に前面にでていたウインドウハンドルを得る
 * @return ウインドウハンドルまたはNULL
 */
static HWND GetNextHwnd()
{
	// 初回は自分自身の入力画面のハンドルが来ることを想定
	HWND hwnd = GetForegroundWindow();
	while (hwnd) {
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);

		if (IsWindow(hwnd) == FALSE) {
			break;
		}
		// 最初に見つかった可視状態のウインドウを直前に全面にいたウインドウとみなす
		if (IsWindowVisible(hwnd) == FALSE) {
			continue;
		}
		break;
	}
	return hwnd;
}

/**
 	コマンド(ウインドウ整列処理)を実行する
 	@return TRUE:成功 FALSE:失敗
 	@param[in] param コマンド実行時パラメータ
*/
BOOL AlignWindowCommand::Execute(
	const Parameter& param
)
{
	CString missingTitles;

	ScopeAttachThreadInput scope;

	HWND hwndForeground = GetNextHwnd();

	std::vector<HWND> targets;
	for (auto& item : in->mParam.mItems) {

		targets.clear();
		item.FindHwnd(targets);

		if (targets.empty()) {
			if (missingTitles.IsEmpty() == FALSE) {
				missingTitles += _T(" / ");
			}
			missingTitles += item.mCaptionStr;
			continue;
		}

		for (auto& hwnd : targets) {
			const WINDOWPLACEMENT& placement = item.mPlacement;
			SetWindowPlacement(hwnd, &placement);
		}
	}

	if (in->mParam.mIsNotifyIfWindowNotFound && missingTitles.IsEmpty() == FALSE) {
		CString msg;
		msg.Format(_T("以下のウインドウは見つかりませんでした。\n%s"), missingTitles);
		soyokaze::commands::common::PopupMessage(msg);
	}

	if (in->mParam.mIsKeepActiveWindow) {
		SetForegroundWindow(hwndForeground);
	}
	else {
		if (in->mParam.mItems.size() > 0) {
			std::vector<HWND> targets;
			in->mParam.mItems.back().FindHwnd(targets);
			if (targets.size() > 0) {
				SetForegroundWindow(targets[0]);
			}
		}
	}

	return TRUE;
}

CString AlignWindowCommand::GetErrorString()
{
	return _T("");
}

HICON AlignWindowCommand::GetIcon()
{
	//return IconLoader::Get()->GetImageResIcon(-5310);
	return IconLoader::Get()->GetImageResIcon(-5357);
}

int AlignWindowCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool AlignWindowCommand::IsEditable()
{
	return true;
}

int AlignWindowCommand::EditDialog(const Parameter*)
{
	SettingDialog dlg;

	auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
	auto param = in->mParam;

	HOTKEY_ATTR hotKeyAttr;
	bool isGlobal = false;
	if (hotKeyManager->HasKeyBinding(param.mName, &hotKeyAttr, &isGlobal)) {
		param.mHotKeyAttr = hotKeyAttr;
		param.mIsGlobal = isGlobal;
	}

	dlg.SetParam(param);
	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	auto cmdNew = std::make_unique<AlignWindowCommand>();

	param = dlg.GetParam();
	cmdNew->in->mParam = param;

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->UnregisterCommand(this);
	cmdRepo->RegisterCommand(cmdNew.release());

	// ホットキー設定を更新
	CommandHotKeyMappings hotKeyMap;
	hotKeyManager->GetMappings(hotKeyMap);

	hotKeyMap.RemoveItem(hotKeyAttr);
	if (param.mHotKeyAttr.IsValid()) {
		hotKeyMap.AddItem(param.mName, param.mHotKeyAttr, param.mIsGlobal);
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
bool AlignWindowCommand::IsPriorityRankEnabled()
{
	// 基本は重みづけをする
	return true;
}

soyokaze::core::Command*
AlignWindowCommand::Clone()
{
	auto clonedCmd = std::make_unique<AlignWindowCommand>();

	clonedCmd->in->mParam = in->mParam;

	return clonedCmd.release();
}

bool AlignWindowCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());

	cmdFile->Set(entry, _T("description"), GetDescription());

	cmdFile->Set(entry, _T("IsNotifyIfWindowNotExist"), in->mParam.mIsNotifyIfWindowNotFound != FALSE);
	cmdFile->Set(entry, _T("IsKeepActiveWindow"), in->mParam.mIsKeepActiveWindow != FALSE);
	cmdFile->Set(entry, _T("ItemCount"), (int)in->mParam.mItems.size());

	CString key;

	int index = 1;
	for (auto& item : in->mParam.mItems) {

		key.Format(_T("CaptionStr%d"), index);
		cmdFile->Set(entry, key, item.mCaptionStr);
		key.Format(_T("ClassStr%d"), index);
		cmdFile->Set(entry, key, item.mClassStr);
		key.Format(_T("IsUseRegExp%d"), index);
		cmdFile->Set(entry, key, item.mIsUseRegExp != FALSE);
		key.Format(_T("IsApplyAll%d"), index);
		cmdFile->Set(entry, key, item.mIsApplyAll != FALSE);

		key.Format(_T("Action%d"), index);
		cmdFile->Set(entry, key, item.mAction);

		key.Format(_T("Placement%d"), index);
		std::vector<uint8_t> data(sizeof(item.mPlacement));
		memcpy(&data.front(), &item.mPlacement, sizeof(item.mPlacement));
		cmdFile->Set(entry, key, data);

		index++;
	}

	return true;
}

uint32_t AlignWindowCommand::AddRef()
{
	return ++in->mRefCount;
}

uint32_t AlignWindowCommand::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

bool AlignWindowCommand::NewDialog(
	const Parameter* param,
	AlignWindowCommand** newCmdPtr
)
{
	// パラメータ指定には対応していない
	// param;

	// 新規作成ダイアログを表示
	SettingDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = dlg.GetParam();
	auto newCmd = std::make_unique<AlignWindowCommand>();
	newCmd->in->mParam = commandParam;

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	// ホットキー設定を更新
	if (commandParam.mHotKeyAttr.IsValid()) {

		auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
		CommandHotKeyMappings hotKeyMap;
		hotKeyManager->GetMappings(hotKeyMap);

		hotKeyMap.AddItem(commandParam.mName, commandParam.mHotKeyAttr, commandParam.mIsGlobal);

		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		pref->Save();
	}

	return true;
}

bool AlignWindowCommand::LoadFrom(CommandFile* cmdFile, void* e, AlignWindowCommand** newCmdPtr)
{
	ASSERT(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;
	CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != AlignWindowCommand::GetType()) {
		return false;
	}

	CString name = cmdFile->GetName(entry);
	CString descriptionStr = cmdFile->Get(entry, _T("description"), _T(""));

	BOOL isNotify = cmdFile->Get(entry, _T("IsNotifyIfWindowNotExist"), false) ? TRUE : FALSE;
	BOOL isKeepActive = cmdFile->Get(entry, _T("IsKeepActiveWindow"), true) ? TRUE : FALSE;

	CString key;

	std::vector<CommandParam::ITEM> items;

	std::vector<uint8_t> placement;

	int itemCount = cmdFile->Get(entry, _T("ItemCount"), 0);
	for (int i = 1; i <= itemCount; ++i) {

		CommandParam::ITEM item;

		key.Format(_T("CaptionStr%d"), i);
		item.mCaptionStr = cmdFile->Get(entry, key, _T(""));
		key.Format(_T("ClassStr%d"), i);
		item.mClassStr = cmdFile->Get(entry, key, _T(""));
		key.Format(_T("IsUseRegExp%d"), i);
		item.mIsUseRegExp = cmdFile->Get(entry, key, false) ? TRUE : FALSE;
		key.Format(_T("IsApplyAll%d"), i);
		item.mIsApplyAll = cmdFile->Get(entry, key, false) ? TRUE : FALSE;

		key.Format(_T("Action%d"), i);
		item.mAction = cmdFile->Get(entry, key, 0);

		key.Format(_T("Placement%d"), i);
		if (cmdFile->Get(entry, key, placement) == false) {
			continue;
		}
		if (placement.size() != sizeof(WINDOWPLACEMENT)) {
			continue;
		}
		memcpy(&item.mPlacement, &placement.front(), placement.size());

		item.BuildRegExp();

		items.push_back(item);
	}

	auto command = std::make_unique<AlignWindowCommand>();

	command->in->mParam.mName = name;
	command->in->mParam.mDescription = descriptionStr;
	command->in->mParam.mItems.swap(items);
	command->in->mParam.mIsNotifyIfWindowNotFound = isNotify;
	command->in->mParam.mIsKeepActiveWindow = isKeepActive;

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

} // end of namespace align_window
} // end of namespace commands
} // end of namespace soyokaze

