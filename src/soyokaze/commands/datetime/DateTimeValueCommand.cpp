#include "pch.h"
#include "framework.h"
#include "DateTimeValueCommand.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
namespace commands {
namespace datetime {

struct DateTimeValueCommand::PImpl
{
	// 表示・コピーする文字列
	CString mText;
	// 種別の表示名
	CString mTypeName;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(DateTimeValueCommand)

DateTimeValueCommand::DateTimeValueCommand(const CString& text, const CString& typeName) : in(std::make_unique<PImpl>())
{
	in->mText = text;
	in->mTypeName = typeName;

	// 候補名と説明は表示する文字列と同じにする
	this->mName = text;
	this->mDescription = text;
}

DateTimeValueCommand::~DateTimeValueCommand()
{
}

CString DateTimeValueCommand::GetTypeDisplayName()
{
	return in->mTypeName;
}

bool DateTimeValueCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	// 修飾キーなしの場合のみ、文字列をクリップボードにコピーする
	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags != 0) {
		return false;
	}

	auto a = new actions::clipboard::CopyTextAction(in->mText);
	a->SetDisplayName(_T("コピー"));
	*action = a;
	return true;
}


HICON DateTimeValueCommand::GetIcon()
{
	// 時計のアイコン
	return IconLoader::Get()->GetShell32Icon(-16752);
}

launcherapp::core::Command*
DateTimeValueCommand::Clone()
{
	return new DateTimeValueCommand(in->mText, in->mTypeName);
}

} // end of namespace datetime
} // end of namespace commands
} // end of namespace launcherapp
