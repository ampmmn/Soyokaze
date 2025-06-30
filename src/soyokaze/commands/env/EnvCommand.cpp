#include "pch.h"
#include "framework.h"
#include "EnvCommand.h"
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
namespace env {

struct EnvCommand::PImpl
{
	CString mValue;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(EnvCommand)

EnvCommand::EnvCommand(const CString& name, const CString& value) : 
	AdhocCommandBase(name, value),
	in(std::make_unique<PImpl>())
{
	in->mValue = value;
}

EnvCommand::~EnvCommand()
{
}

CString EnvCommand::GetGuideString()
{
	return _T("⏎:クリップボードにコピー");
}

CString EnvCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL EnvCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// クリップボードにコピー
	Clipboard::Copy(in->mValue);
	return TRUE;
}

HICON EnvCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-149);
}

launcherapp::core::Command*
EnvCommand::Clone()
{
	return new EnvCommand(this->mName, in->mValue);
}

CString EnvCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_ENVIRON);
	return TEXT_TYPE;
}

} // end of namespace env
} // end of namespace commands
} // end of namespace launcherapp

