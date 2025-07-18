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


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(GetIPCommand)

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
	return _T("⏎:IPアドレスをコピー");
}

CString GetIPCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL GetIPCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

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

CString GetIPCommand::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("IP Address"));
	return TEXT_TYPE;
}

} // end of namespace getip
} // end of namespace commands
} // end of namespace launcherapp

