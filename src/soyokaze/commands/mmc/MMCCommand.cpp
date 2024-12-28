#include "pch.h"
#include "framework.h"
#include "MMCCommand.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace mmc {

struct MMCCommand::PImpl
{
	MMCSnapin mSnapin;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(MMCCommand)

MMCCommand::MMCCommand(const MMCSnapin& snapin) : 
	AdhocCommandBase(snapin.mDisplayName, snapin.mDescription),
	in(std::make_unique<PImpl>())
{
	in->mSnapin = snapin;
}

MMCCommand::~MMCCommand()
{
}

CString MMCCommand::GetGuideString()
{
	return _T("Enter:実行");
}

CString MMCCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE(_T("MMCスナップイン"));
	return TEXT_TYPE;
}

BOOL MMCCommand::Execute(Parameter* param)
{
	CString paramStr;
	paramStr.Format(_T("/c start \"\" \"%s\""), (LPCTSTR)in->mSnapin.mMscFilePath);

	SubProcess::ProcessPtr process;

	SubProcess exec(param);
	exec.SetShowType(SW_HIDE);
	if (exec.Run(_T("cmd.exe"), paramStr, process) == false) {
		mErrMsg = process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON MMCCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconResource(in->mSnapin.mIconFilePath, in->mSnapin.mIconIndex);
}

launcherapp::core::Command*
MMCCommand::Clone()
{
	return new MMCCommand(in->mSnapin);
}

} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

