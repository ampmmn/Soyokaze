#include "pch.h"
#include "framework.h"
#include "WatchPathCommand.h"
#include "commands/watchpath/WatchPathCommandEditDialog.h"
#include "commands/watchpath/PathWatcher.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "core/CommandRepository.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace watchpath {

using namespace soyokaze::commands::common;

using CommandRepository = soyokaze::core::CommandRepository;
using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

struct WatchPathCommand::PImpl
{
	PImpl() : mRefCount(1)
	{
	}
	~PImpl()
	{
	}

	CString mName;
	CString mDescription;
	CString mPath;

	// 参照カウント
	uint32_t mRefCount;
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

CString WatchPathCommand::GetName()
{
	return in->mName;
}


CString WatchPathCommand::GetDescription()
{
	return in->mDescription;
}

CString WatchPathCommand::GetGuideString()
{
	return _T("Enter:更新検知対象パスを開く");
}

CString WatchPathCommand::GetTypeDisplayName()
{
	return _T("フォルダ更新検知");
}

BOOL WatchPathCommand::Execute(const Parameter& param)
{
	auto path = in->mPath;
	path += _T("\\");

	ShellExecCommand cmd;
	cmd.SetPath(path);

	Parameter paramEmpty;
	return cmd.Execute(paramEmpty);
}

CString WatchPathCommand::GetErrorString()
{
	return _T("");
}

HICON WatchPathCommand::GetIcon()
{
	CString path = in->mPath;
	ExpandEnv(path);

	return IconLoader::Get()->LoadIconFromPath(path);
}

int WatchPathCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool WatchPathCommand::IsEditable()
{
	return true;
}

int WatchPathCommand::EditDialog(const Parameter* param)
{
	auto oldPath = in->mPath;

	CommandEditDialog dlg;
	dlg.SetOrgName(in->mName);

	dlg.mName = in->mName;
	dlg.mDescription = in->mDescription;
	dlg.mPath = in->mPath;

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	// 名前が変わっている可能性があるため、いったん削除して再登録する #addref
	auto cmdRepo = CommandRepository::GetInstance();

	AddRef();  // UnregisterCommandで削除されるのを防ぐため+1しておく
	cmdRepo->UnregisterCommand(this);

	CString orgName = in->mName;

	in->mName = dlg.mName;
	in->mDescription = dlg.mDescription;
	in->mPath = dlg.mPath;

	// RegisterCommandはrefCountを+1しないので、#addrefで上げたカウントをさげる必要はない
	cmdRepo->RegisterCommand(this);

	// パスが変わったら登録しなおす
	if (orgName != in->mName) {
		auto watcher = PathWatcher::Get();
		watcher->UnregisterPath(orgName);
		watcher->RegisterPath(in->mName, dlg.mPath);
	}

	return 0;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool WatchPathCommand::IsPriorityRankEnabled()
{
	return false;
}

soyokaze::core::Command*
WatchPathCommand::Clone()
{
	auto clonedObj = std::make_unique<WatchPathCommand>();

	clonedObj->in->mName = in->mName;
	clonedObj->in->mDescription = in->mDescription;
	clonedObj->in->mPath = in->mPath;

	return clonedObj.release();
}

bool WatchPathCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());

	cmdFile->Set(entry, _T("description"), GetDescription());
	cmdFile->Set(entry, _T("path"), in->mPath);

	return true;
}

bool WatchPathCommand::Load(CommandFile* cmdFile, void* entry_)
{
	auto entry = (CommandFile::Entry*)entry_;

	in->mName = cmdFile->GetName(entry);
	in->mDescription = cmdFile->Get(entry, _T("description"), _T(""));
	in->mPath = cmdFile->Get(entry, _T("path"), _T(""));

	// 監視対象に登録
	PathWatcher::Get()->RegisterPath(in->mName, in->mPath);

	return true;
}

uint32_t WatchPathCommand::AddRef()
{
	return ++in->mRefCount;
}

uint32_t WatchPathCommand::Release()
{
	auto n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

bool WatchPathCommand::NewDialog(const Parameter* param)
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
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<WatchPathCommand>();
	newCmd->in->mName = dlg.mName;
	newCmd->in->mDescription = dlg.mDescription;
	newCmd->in->mPath = dlg.mPath;

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());

	// 監視対象に登録
	PathWatcher::Get()->RegisterPath(dlg.mName, dlg.mPath);

	return true;

}

}
}
}

