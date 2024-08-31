#include "pch.h"
#include "framework.h"
#include "commands/builtin/CopyClipboardCommand.h"
#include "commands/common/Clipboard.h"
#include "icon/IconLoader.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

using namespace launcherapp::commands::common;


CString CopyClipboardCommand::TYPE(_T("Builtin-Clipcopy"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(CopyClipboardCommand)


CString CopyClipboardCommand::GetType()
{
	return TYPE;
}

CopyClipboardCommand::CopyClipboardCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("copyclip"))
{
	mDescription = _T("【文字列をクリップボードにコピー】");
	mCanSetConfirm = false;
	mCanDisable = true;
}

CopyClipboardCommand::CopyClipboardCommand(const CopyClipboardCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

CopyClipboardCommand::~CopyClipboardCommand()
{
}

BOOL CopyClipboardCommand::Execute(const Parameter& param)
{
	std::vector<CString> args;
	param.GetParameters(args);

	// 引数が空なら何もしない
	if (args.empty()) {
		return TRUE;
	}

	// クリップボードにコピー
	OnCopy(param.GetParameterString());

	return TRUE;
}

HICON CopyClipboardCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5301);
}

launcherapp::core::Command* CopyClipboardCommand::Clone()
{
	return new CopyClipboardCommand(*this);
}


void CopyClipboardCommand::OnCopy(const CString& str)
{
	Clipboard::Copy(str);
}

}
}
}
