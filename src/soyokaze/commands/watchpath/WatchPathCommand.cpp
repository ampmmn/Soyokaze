#include "pch.h"
#include "framework.h"
#include "WatchPathCommand.h"
#include "core/IFIDDefine.h"
#include "commands/watchpath/WatchPathCommandEditor.h"
#include "commands/watchpath/PathWatcher.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/ExpandFunctions.h"
#include "actions/builtin/CallbackAction.h"
#include "actions/core/ActionParameter.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "commands/common/SubProcess.h"
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
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;


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
	return _T("⏎:更新検知対象パスを開く");
}

CString WatchPathCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool WatchPathCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	UNREFERENCED_PARAMETER(modifierFlags);

	*action = new CallbackAction(_T("更新検知対象パスを開く"), [&](Parameter*,String* errMsg) -> bool {

		if (in->mParam.mIsDisabled) {
			return true;
		}
	
		auto path = in->mParam.mPath;
		path += _T("\\");

		SubProcess exec(ParameterBuilder::EmptyParam());
		SubProcess::ProcessPtr process;
		if (exec.Run(path, process) == FALSE) {
			if (errMsg) {
				UTF2UTF(process->GetErrorMessage(), *errMsg);
			}
			return false;
		}
		return true;
	});

	return true;
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

launcherapp::core::Command*
WatchPathCommand::Clone()
{
	auto clonedObj = make_refptr<WatchPathCommand>();
	clonedObj->SetParam(in->mParam);
	return clonedObj.release();
}

bool WatchPathCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);
	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);

}

bool WatchPathCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}

	CommandParam paramTmp;
	paramTmp.Load(entry);

	if (paramTmp == in->mParam) {
		// 変化がなければ何もしない
		SPDLOG_DEBUG(_T("skip loading"));
		return true;
	}

	in->mParam.swap(paramTmp);

	// 監視対象に登録
	PathWatcher::ITEM item;
	item.mPath = in->mParam.mPath;
	item.mMessage = in->mParam.mNotifyMessage;
	item.mInterval = in->mParam.mWatchInterval;
	item.mExcludeFilter = in->mParam.mExcludeFilter;
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
	auto newCmd = make_refptr<WatchPathCommand>();

	const auto& paramNew = cmdEditor->GetParam();
	newCmd->SetParam(paramNew);

	// 監視対象に登録
	if (paramNew.mIsDisabled == false) {
		PathWatcher::ITEM item;
		item.mPath = paramNew.mPath;
		item.mMessage = paramNew.mNotifyMessage;
		item.mInterval = paramNew.mWatchInterval;
		item.mExcludeFilter = paramNew.mExcludeFilter;
		PathWatcher::Get()->RegisterPath(paramNew.mName, item);
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());

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
		item.mInterval = in->mParam.mWatchInterval;
		item.mExcludeFilter = in->mParam.mExcludeFilter;
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
	auto newCmd = make_refptr<WatchPathCommand>();
	newCmd->SetParam(paramNew);

	// 監視対象に登録
	if (paramNew.mIsDisabled == false) {
		PathWatcher::ITEM item;
		item.mPath = paramNew.mPath;
		item.mMessage = paramNew.mNotifyMessage;
		item.mInterval = paramNew.mWatchInterval;
		item.mExcludeFilter = paramNew.mExcludeFilter;
		PathWatcher::Get()->RegisterPath(paramNew.mName, item);
	}


	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

CString WatchPathCommand::TypeDisplayName()
{
	return _T("フォルダ更新検知");
}

}
}
}

