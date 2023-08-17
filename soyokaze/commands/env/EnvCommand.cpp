#include "pch.h"
#include "framework.h"
#include "EnvCommand.h"
#include "IconLoader.h"
#include "commands/common/Clipboard.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace soyokaze::commands::common;

namespace soyokaze {
namespace commands {
namespace env {


struct EnvCommand::PImpl
{
	CString mValue;
};


EnvCommand::EnvCommand(const CString& name, const CString& value) : 
	AdhocCommandBase(name, value),
	in(std::make_unique<PImpl>())
{
	in->mValue = value;
}

EnvCommand::~EnvCommand()
{
}

CString EnvCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_ENVIRON);
	return TEXT_TYPE;
}

BOOL EnvCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	Clipboard::Copy(in->mValue);
	return TRUE;
}

HICON EnvCommand::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

soyokaze::core::Command*
EnvCommand::Clone()
{
	return new EnvCommand(this->mName, in->mValue);
}

} // end of namespace env
} // end of namespace commands
} // end of namespace soyokaze

