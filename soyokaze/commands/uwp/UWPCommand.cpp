#include "pch.h"
#include "framework.h"
#include "UWPCommand.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace uwp {


struct UWPCommand::PImpl
{
	ITEM mItem;
};


UWPCommand::UWPCommand(const ITEM& item) : 
	AdhocCommandBase(item.mName, item.mDescription),
	in(std::make_unique<PImpl>())
{
	in->mItem = item;
}

UWPCommand::~UWPCommand()
{
}


CString UWPCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_UWP);
	return TEXT_TYPE;
}

BOOL UWPCommand::Execute(const Parameter& param)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_HIDE;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = _T("cmd.exe");

	CString cmdline;
	cmdline.Format(_T("/c start %s:"), in->mItem.mScheme);
	si.lpParameters = cmdline;

	ShellExecuteEx(&si);
	CloseHandle(si.hProcess);

	return TRUE;
}

HICON UWPCommand::GetIcon()
{
	return IconLoader::Get()->GetDefaultIcon(in->mItem.mIconPath);
}

soyokaze::core::Command*
UWPCommand::Clone()
{
	return new UWPCommand(in->mItem);
}

} // end of namespace uwp
} // end of namespace commands
} // end of namespace soyokaze

