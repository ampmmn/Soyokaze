#include "pch.h"
#include "framework.h"
#include "ClipboardHistoryCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

struct ClipboardHistoryCommand::PImpl
{
	uint64_t mAppendDate = 0;
	CString mPrefix;
	CString mData;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(ClipboardHistoryCommand)

ClipboardHistoryCommand::ClipboardHistoryCommand(
 	const CString& prefix,
	uint64_t appendDate,
 	const CString& data
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mAppendDate = appendDate;
	in->mPrefix = prefix;
	in->mData = data;
}

ClipboardHistoryCommand::~ClipboardHistoryCommand()
{
}

CString ClipboardHistoryCommand::GetName()
{
	return in->mPrefix + _T(" ") + in->mData;
}

CString ClipboardHistoryCommand::GetDescription()
{
	return in->mData;

}

CString ClipboardHistoryCommand::GetGuideString()
{
	return _T("Enter:コピー");
}

CString ClipboardHistoryCommand::GetTypeDisplayName()
{
	return _T("クリップボード履歴");
}

BOOL ClipboardHistoryCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// 値をコピー
	Clipboard::Copy(in->mData);

	return TRUE;
}

HICON ClipboardHistoryCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5301);
}

launcherapp::core::Command*
ClipboardHistoryCommand::Clone()
{
	return new ClipboardHistoryCommand(in->mPrefix, in->mAppendDate, in->mData);
}


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

