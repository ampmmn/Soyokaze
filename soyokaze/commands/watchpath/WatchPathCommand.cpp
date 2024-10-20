#include "pch.h"
#include "framework.h"
#include "WatchPathCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/watchpath/WatchPathCommandEditor.h"
#include "commands/watchpath/PathWatcher.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace watchpath {

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;
using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


struct WatchPathCommand::PImpl
{
	CommandParam mParam;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString WatchPathCommand::GetType() { return _T("WatchPath"); }

WatchPathCommand::WatchPathCommand() : in(std::make_unique<PImpl>())
{
}

WatchPathCommand::~WatchPathCommand()
{
}


void WatchPathCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

CString WatchPathCommand::GetName()
{
	return in->mParam.mName;
}


CString WatchPathCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString WatchPathCommand::GetGuideString()
{
	return _T("Enter:更新検知対象パスを開く");
}

CString WatchPathCommand::GetTypeDisplayName()
{
	return _T("フォルダ更新検知");
}

BOOL WatchPathCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (in->mParam.mIsDisabled) {
		return TRUE;
	}

	auto path = in->mParam.mPath;
	path += _T("\\");

	ShellExecCommand cmd;
	cmd.SetPath(path);

	return cmd.Execute(CommandParameterBuilder::EmptyParam());
}

CString WatchPathCommand::GetErrorString()
{
	return _T("");
}

HICON WatchPathCommand::GetIcon()
{
	CString path = in->mParam.mPath;
	ExpandMacros(path);

	return IconLoader::Get()->LoadIconFromPath(path);
}

int WatchPathCommand::Match(Pattern* pattern)
{
	return pattern->Match(in->mParam.mName);
}

bool WatchPathCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	UNREFERENCED_PARAMETER(attr);

	return false;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool WatchPathCommand::IsPriorityRankEnabled()
{
	return false;
}

launcherapp::core::Command*
WatchPathCommand::Clone()
{
	auto clonedObj = std::make_unique<WatchPathCommand>();
	clonedObj->SetParam(in->mParam);
	return clonedObj.release();
}

bool WatchPathCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());

	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("path"), in->mParam.mPath);
	entry->Set(_T("message"), in->mParam.mNotifyMessage);
	entry->Set(_T("isDisabled"), in->mParam.mIsDisabled);

	return true;
}

bool WatchPathCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));
	in->mParam.mPath = entry->Get(_T("path"), _T(""));
	in->mParam.mNotifyMessage = entry->Get(_T("message"), _T(""));
	in->mParam.mIsDisabled = entry->Get(_T("isDisabled"), false);

	// 監視対象に登録
	PathWatcher::ITEM item;
	item.mPath = in->mParam.mPath;
	item.mMessage = in->mParam.mNotifyMessage;
	PathWatcher::Get()->RegisterPath(in->mParam.mName, item);

	return true;
}

bool WatchPathCommand::NewDialog(Parameter* param)
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
	auto newCmd = std::make_unique<WatchPathCommand>();

	const auto& paramNew = cmdEditor->GetParam();
	newCmd->SetParam(paramNew);

	// 監視対象に登録
	if (paramNew.mIsDisabled == false) {
		PathWatcher::ITEM item;
		item.mPath = paramNew.mPath;
		item.mMessage = paramNew.mNotifyMessage;
		PathWatcher::Get()->RegisterPath(paramNew.mName, item);
	}

	bool isReloadHotKey = false;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);

	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool WatchPathCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool WatchPathCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_WATCHPATHCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();

	// 登録しなおす
	auto watcher = PathWatcher::Get();

	CString orgName = cmdEditor->GetOriginalName();
	watcher->UnregisterPath(orgName);

	if (in->mParam.mIsDisabled == false) {
		PathWatcher::ITEM item;
		item.mPath = in->mParam.mPath;
		item.mMessage = in->mParam.mNotifyMessage;
		watcher->RegisterPath(GetName(), item);
	}

	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool WatchPathCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_WATCHPATHCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	auto paramNew = cmdEditor->GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<WatchPathCommand>();
	newCmd->SetParam(paramNew);

	// 監視対象に登録
	if (paramNew.mIsDisabled == false) {
		PathWatcher::ITEM item;
		item.mPath = paramNew.mPath;
		item.mMessage = paramNew.mNotifyMessage;
		PathWatcher::Get()->RegisterPath(paramNew.mName, item);
	}


	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

}
}
}

