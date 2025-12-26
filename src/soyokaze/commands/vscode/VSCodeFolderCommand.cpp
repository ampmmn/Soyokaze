#include "pch.h"
#include "VSCodeFolderCommand.h"
#include "commands/vscode/VSCodeCommandParam.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/vscode/VSCodeExecuteAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "utility/Path.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "framework.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace vscode {

struct VSCodeFolderCommand::PImpl
{
	CString mFolderPath;
	const CommandParam* mParam{nullptr};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(VSCodeFolderCommand)

VSCodeFolderCommand::VSCodeFolderCommand(
	const CommandParam* param,
 	const CString& displayName,
 	const CString& localPath
) : 
	AdhocCommandBase(_T(""), localPath),
	in(std::make_unique<PImpl>())
{
	mName = param->HasPrefix() ? param->mPrefix + _T(" ") + displayName : displayName;

	in->mFolderPath = localPath;
	in->mParam = param;
}

VSCodeFolderCommand::~VSCodeFolderCommand()
{
}

bool VSCodeFolderCommand::Create(nlohmann::json& json, CommandParam* param, Command** cmd)
{
	//   {"folderUri":"file:///path/to/folder"},
	try {
		auto uri = json.find("folderUri");
		if (uri == json.end()) {
			return false;
		}

		Path path;
		path.FileUriToLocalPath(*uri);
		if (path.FileExists() == false) {
			return false;
		}

		CString displayName;
		if (param->mIsShowFullPath) {
			displayName = path;
		}
		else {
			displayName = path.FindFileName();
		}

		*cmd = new VSCodeFolderCommand(param, displayName, (LPCTSTR)path);
		return true;
	}
	catch(json::exception& e) {
		spdlog::warn("failed to parse folderUri entry. err:{}", e.what());
		return false;
	}
	return true;
}

CString VSCodeFolderCommand::GetTypeDisplayName()
{
	return VSCodeFolderCommand::TypeDisplayName();
}

bool VSCodeFolderCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags == 0) {
		*action = new VSCodeExecuteAction(in->mParam, in->mFolderPath, false);
		return true;
	}
	else if (modifierFlags == MOD_SHIFT) {
		*action = new VSCodeExecuteAction(in->mParam, in->mFolderPath, true);
		return true;
	}
	else if (modifierFlags == MOD_CONTROL) {
		*action = new actions::clipboard::CopyTextAction(in->mFolderPath);
		return true;
	}
	return false;
}


HICON VSCodeFolderCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mParam->GetVSCodeExePath());
}

int VSCodeFolderCommand::Match(Pattern* pattern)
{
	return pattern->Match(PathFindFileName(in->mFolderPath), in->mParam->HasPrefix() ? 1 : 0);
}

launcherapp::core::Command*
VSCodeFolderCommand::Clone()
{
	return new VSCodeFolderCommand(in->mParam, mName, in->mFolderPath);
}

CString VSCodeFolderCommand::GetSourceName()
{
	return mName;
}

bool VSCodeFolderCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString VSCodeFolderCommand::TypeDisplayName()
{
	return _T("履歴(フォルダ)");
}


} // end of namespace vscode
} // end of namespace commands
} // end of namespace launcherapp

