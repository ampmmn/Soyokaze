#include "pch.h"
#include "PlaceWindowInRegionCommand.h"
#include "commands/place_window_in_region/PlaceWindowInRegionAdhocCommand.h"
#include "commands/place_window_in_region/PlaceWindowInRegionParameter.h"
#include "commands/place_window_in_region/PlaceWindowInRegionCommandEditor.h"
//#include "commands/place_window_in_region/ScreenRegionSelectorWindow.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/activate_window/WindowList.h"
#include "commands/core/CommandRepository.h"
#include "core/IFIDDefine.h"
#include "hotkey/CommandHotKeyManager.h"
#include "utility/ScopeAttachThreadInput.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "commands/common/Message.h"
#include "actions/mainwindow/MainWindowSetTextAction.h"

using namespace launcherapp::commands::common;
using SetTextAction = launcherapp::actions::mainwindow::SetTextAction;

namespace launcherapp {
namespace commands {
namespace place_window_in_region {

struct TARGET
{
public:
	HWND mHwnd;
	CString mName;
};


struct PlaceWindowInRegionCommand::PImpl
{
	void EnumTargets();

	std::vector<TARGET> mTargets;
	CommandParam mParam;
};

void PlaceWindowInRegionCommand::PImpl::EnumTargets()
{
	std::vector<HWND> handles;
	launcherapp::commands::activate_window::WindowList windowList;
	windowList.EnumWindowHandles(handles);

	std::vector<TARGET> targets;
	for (auto& hwnd : handles) {
		TCHAR caption[256];
		::GetWindowText(hwnd, caption, 256);
		targets.push_back(TARGET{hwnd, caption});
	}
	mTargets.swap(targets);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CString PlaceWindowInRegionCommand::GetType() { return _T("PlaceWindowInRegion"); }

CString PlaceWindowInRegionCommand::TypeDisplayName()
{
	return _T("ウインドウ配置コマンド");
}

PlaceWindowInRegionCommand::PlaceWindowInRegionCommand() : in(std::make_unique<PImpl>())
{
}

PlaceWindowInRegionCommand::~PlaceWindowInRegionCommand()
{
}

bool PlaceWindowInRegionCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

void PlaceWindowInRegionCommand::SetParam(const CommandParam& param)
{
	// パラメータを上書き
	in->mParam = param;
}

CString PlaceWindowInRegionCommand::GetName()
{
	return in->mParam.mName;
}

CString PlaceWindowInRegionCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString PlaceWindowInRegionCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

// 修飾キー押下状態に対応した実行アクションを取得する
bool PlaceWindowInRegionCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() == 0) {
		// コマンド名単体(後続のパラメータなし)で実行したときはウインドウ候補一覧を列挙させる
		LPCTSTR guideStr = _T("キーワード入力すると候補を絞り込むことができます");
		*action = new SetTextAction(guideStr, GetName() + _T(" "));
		return true;
	}
	return false;
}

HICON PlaceWindowInRegionCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-16757);
}

int PlaceWindowInRegionCommand::Match(Pattern* pattern)
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

bool PlaceWindowInRegionCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
PlaceWindowInRegionCommand::Clone()
{
	auto clonedCmd = make_refptr<PlaceWindowInRegionCommand>();

	clonedCmd->SetParam(in->mParam);

	return clonedCmd.release();
}

bool PlaceWindowInRegionCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);
}

bool PlaceWindowInRegionCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != PlaceWindowInRegionCommand::GetType()) {
		return false;
	}

	CommandParam param;
	if (param.Load(entry) == false) {
		return false;
	}

	SetParam(param);

	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool PlaceWindowInRegionCommand::NewInstance(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<PlaceWindowInRegionCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_PLACEWINDOWINREGIONCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<PlaceWindowInRegionCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool PlaceWindowInRegionCommand::NewDialog(
	Parameter* param,
	PlaceWindowInRegionCommand** newCmdPtr
)
{
	// パラメータ指定には対応していない
	CString value;
	CommandParam paramTmp;

	if (GetNamedParamString(param, _T("COMMAND"), value)) {
		paramTmp.mName = value;
	}
	if (GetNamedParamString(param, _T("DESCRIPTION"), value)) {
		paramTmp.mDescription = value;
	}

	RefPtr<PlaceWindowInRegionCommandEditor> cmdEditor(new PlaceWindowInRegionCommandEditor);
	cmdEditor->SetParam(paramTmp);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = cmdEditor->GetParam();
	auto newCmd = make_refptr<PlaceWindowInRegionCommand>();
	newCmd->SetParam(commandParam);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool PlaceWindowInRegionCommand::LoadFrom(CommandFile* cmdFile, void* e, PlaceWindowInRegionCommand** newCmdPtr)
{
	UNREFERENCED_PARAMETER(cmdFile);

	ASSERT(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;

	auto command = make_refptr<PlaceWindowInRegionCommand>();
	if (command->Load(entry) == false) {
		return false;
	}

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool PlaceWindowInRegionCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new PlaceWindowInRegionCommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool PlaceWindowInRegionCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<PlaceWindowInRegionCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_PLACEWINDOWINREGIONCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	SetParam(cmdEditor->GetParam());
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool PlaceWindowInRegionCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<PlaceWindowInRegionCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_PLACEWINDOWINREGIONCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<PlaceWindowInRegionCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

/**
 	検索処理(コマンド名が入力していなければ候補を表示しない)
 	@return        true: 検索結果あり   false:検索結果なし
 	@param[in]     pattern  検索パターン
 	@param[out]    commands 検索して見つかった候補を入れる入れ物
 	@param[in]     tm       タイムアウト判定用
*/
bool PlaceWindowInRegionCommand::QueryCandidates(Pattern* pattern, CommandQueryItemList& commands)
{
	if (in->mParam.mName.CompareNoCase(pattern->GetFirstWord()) != 0) {
		// コマンド名が一致しない場合は検索対象外
		return false;
	}

	if (in->mTargets.size() == 0) {
		// ToDo: ウインドウを列挙する
		in->EnumTargets();
	}

	bool hasCandidate = false;

	auto it = in->mTargets.begin();
	while (it != in->mTargets.end()) {
		const auto& name = it->mName;
		int level = pattern->Match(name, 1);
		if (level == Pattern::Mismatch) {
			it++;
			continue;
		}

		// コマンド名が一致しているので最低でも前方一致扱いとする
		if (level == Pattern::PartialMatch) {
			level = Pattern::FrontMatch;
		}

		auto hwnd = it->mHwnd;
		if (IsWindow(hwnd) == FALSE) {
			it = in->mTargets.erase(it);
			continue;
		}

		auto cmd = new PlaceWindowInRegionAdhocCommand(hwnd, name, in->mParam);
		commands.Add(CommandQueryItem(level, cmd));
		it++;
		hasCandidate = true;
	}

	return hasCandidate;
}

void PlaceWindowInRegionCommand::ClearCache()
{
	in->mTargets.clear();
}

} // end of namespace place_window_in_region
} // end of namespace commands
} // end of namespace launcherapp

