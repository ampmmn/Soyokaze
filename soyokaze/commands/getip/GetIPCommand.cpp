#include "pch.h"
#include "framework.h"
#include "GetIPCommand.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace getip {


struct GetIPCommand::PImpl
{
	CString mDisplayName;
	CString mAddress;
};


GetIPCommand::GetIPCommand(const CString& displayName, const CString& addr) : in(std::make_unique<PImpl>())
{
	in->mDisplayName = displayName;
	in->mAddress = addr;
}

GetIPCommand::~GetIPCommand()
{
}

CString GetIPCommand::GetName()
{
	return in->mAddress;
}

CString GetIPCommand::GetDescription()
{
	return in->mDisplayName + _T(" : ") + in->mAddress;
}

CString GetIPCommand::GetGuideString()
{
	return _T("Enter:IPアドレスをコピー");
}

CString GetIPCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE(_T("IP Address"));
	return TEXT_TYPE;
}

BOOL GetIPCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	Clipboard::Copy(in->mAddress);
	return TRUE;
}


HICON GetIPCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-74);
}

launcherapp::core::Command*
GetIPCommand::Clone()
{
	return new GetIPCommand(in->mDisplayName, in->mAddress);
}

} // end of namespace getip
} // end of namespace commands
} // end of namespace launcherapp

