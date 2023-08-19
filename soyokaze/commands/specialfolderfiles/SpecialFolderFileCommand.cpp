#include "pch.h"
#include "framework.h"
#include "SpecialFolderFileCommand.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
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


CString SpecialFolderFileCommand::GetTypeDisplayName()
{
	if (in->mItem.mType == TYPE_RECENT) {
		static CString TEXT_TYPE_RECENT((LPCTSTR)IDS_COMMAND_RECENTFILES);
		return TEXT_TYPE_RECENT;
	}
	else {
		static CString TEXT_TYPE_STARTMENU((LPCTSTR)IDS_COMMAND_STARTMENU);
		return TEXT_TYPE_STARTMENU;
	}
}

BOOL SpecialFolderFileCommand::Execute(const Parameter& param)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = in->mItem.mFullPath;
	si.lpParameters = nullptr;

	ShellExecuteEx(&si);
	CloseHandle(si.hProcess);

	return TRUE;
}

HICON SpecialFolderFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mItem.mFullPath);
}

soyokaze::core::Command*
SpecialFolderFileCommand::Clone()
{
	return new SpecialFolderFileCommand(in->mItem);
}

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace soyokaze

