#include "pch.h"
#include "framework.h"
#include "EverythingAdhocCommand.h"
#include "commands/everything/EverythingCommandParam.h"
#include "commands/everything/EverythingResult.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/core/CommandRepository.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/builtin/ShowPropertiesAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using OpenPathInFilerAction = launcherapp::actions::builtin::OpenPathInFilerAction;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;
using ShowPropertiesAction = launcherapp::actions::builtin::ShowPropertiesAction;

namespace launcherapp {
namespace commands {
namespace everything {

struct EverythingAdhocCommand::PImpl
{
	CommandParam mParam;
	EverythingResult mResult;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(EverythingAdhocCommand)

EverythingAdhocCommand::EverythingAdhocCommand() : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
}


EverythingAdhocCommand::EverythingAdhocCommand(
	const CommandParam& param,
 	const EverythingResult& result
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mParam = param;
	in->mResult = result;
}

EverythingAdhocCommand::~EverythingAdhocCommand()
{
}

CString EverythingAdhocCommand::GetName()
{
	return in->mResult.mFullPath;
}

CString EverythingAdhocCommand::GetDescription()
{
	CString str;
	str.Format(_T("%s"), (LPCTSTR)PathFindFileName(in->mResult.mFullPath));
	return str;

}

CString EverythingAdhocCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool EverythingAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		// 実行
		*action = new ExecuteAction(in->mResult.mFullPath);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_SHIFT) {
		// パスをコピー
		*action = new CopyTextAction(in->mResult.mFullPath);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_CTRL) {
		// フォルダを開く
		*action = new OpenPathInFilerAction(in->mResult.mFullPath);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_ALT) {
		// フォルダを開く
		*action = new ShowPropertiesAction(in->mResult.mFullPath);
		return true;
	}
	else if (modifierFlags == (Command::MODIFIER_CTRL|Command::MODIFIER_SHIFT)) {
		// 管理者権限で実行
		auto a = new ExecuteAction(in->mResult.mFullPath);
		a->SetRunAsAdmin();
		*action = a;
		return true;
	}
	return false;
}

HICON EverythingAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mResult.mFullPath);
}

launcherapp::core::Command*
EverythingAdhocCommand::Clone()
{
	return new EverythingAdhocCommand(in->mParam, in->mResult);
}

CString EverythingAdhocCommand::GetSourceName()
{
	return in->mParam.mPrefix;
}

bool EverythingAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_EXTRACANDIDATE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidate*)this;
		return true;
	}
	return false;
}

CString EverythingAdhocCommand::TypeDisplayName()
{
	return _T("Everything検索");
}

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

