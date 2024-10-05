#include "pch.h"
#include "framework.h"
#include "SpecialFolderFileCommand.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"
#include "setting/AppPreference.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {


struct SpecialFolderFileCommand::PImpl
{
	ITEM mItem;
};


SpecialFolderFileCommand::SpecialFolderFileCommand(const ITEM& item) : 
	AdhocCommandBase(item.mName, item.mDescription),
	in(std::make_unique<PImpl>())
{
	in->mItem = item;
	if (item.mDescription.IsEmpty()) {
		this->mDescription = item.mFullPath;
	}
}

SpecialFolderFileCommand::~SpecialFolderFileCommand()
{
}

CString SpecialFolderFileCommand::GetGuideString()
{
	return _T("Enter:開く Ctrl-Enter:フォルダを開く");
}


CString SpecialFolderFileCommand::GetTypeDisplayName()
{
	if (in->mItem.mType == TYPE_RECENT) {
		static CString TEXT_TYPE_RECENT((LPCTSTR)IDS_COMMAND_RECENTFILES);
		return TEXT_TYPE_RECENT;
	}
	//else if (in->mItem.mType == TYPE_STARTMENU) {
		static CString TEXT_TYPE_STARTMENU((LPCTSTR)IDS_COMMAND_STARTMENU);
		return TEXT_TYPE_STARTMENU;
	//}
}

BOOL SpecialFolderFileCommand::Execute(Parameter* param)
{
	SubProcess::ProcessPtr process;

	SubProcess exec(param);
	if (exec.Run(in->mItem.mFullPath, process) == false) {
		this->mErrMsg = process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON SpecialFolderFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mItem.mFullPath);
}

launcherapp::core::Command*
SpecialFolderFileCommand::Clone()
{
	return new SpecialFolderFileCommand(in->mItem);
}

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

