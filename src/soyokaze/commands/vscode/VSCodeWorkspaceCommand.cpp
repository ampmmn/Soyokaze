#include "pch.h"
#include "VSCodeWorkspaceCommand.h"
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

struct VSCodeWorkspaceCommand::PImpl
{
	CString mID;
	CString mConfigPath;
	const CommandParam* mParam{nullptr};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(VSCodeWorkspaceCommand)

VSCodeWorkspaceCommand::VSCodeWorkspaceCommand(
	const CommandParam* param,
	const CString& id,
 	const CString& displayName,
 	const CString& localPath
) : 
	AdhocCommandBase(_T(""), localPath),
	in(std::make_unique<PImpl>())
{
	mName = param->HasPrefix() ? param->mPrefix + _T(" ") + displayName : displayName;

	in->mID = id;
	in->mConfigPath = localPath;
	in->mParam = param;
}

VSCodeWorkspaceCommand::~VSCodeWorkspaceCommand()
{
}

bool VSCodeWorkspaceCommand::Create(nlohmann::json& json, CommandParam* param, Command** cmd)
{
	// 	{"workspace":{"id":"2db1ad0c811e159b30b119a0aef8ce79","configPath":"file:///c%3A/path/to/file.code-workspace"}},

	try {
		auto ws = json.find("workspace");
		if (ws == json.end()) {
			return false;
		}

		CString id;
		UTF2UTF((*ws)["id"], id);

		Path path;
		path.FileUriToLocalPath((*ws)["configPath"]);
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

		*cmd = new VSCodeWorkspaceCommand(param, id, displayName, (LPCTSTR)path);
		return true;
	}
	catch(json::exception& e) {
		spdlog::warn("failed to parse workspace entry. err:{}", e.what());
		return false;
	}

	return true;
}

CString VSCodeWorkspaceCommand::GetTypeDisplayName()
{
	return VSCodeWorkspaceCommand::TypeDisplayName();
}

bool VSCodeWorkspaceCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		*action = new VSCodeExecuteAction(in->mParam, in->mConfigPath, false);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_SHIFT) {
		*action = new VSCodeExecuteAction(in->mParam, in->mConfigPath, true);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_CTRL) {
		*action = new actions::clipboard::CopyTextAction(in->mConfigPath);
		return true;
	}
	return false;
}


HICON VSCodeWorkspaceCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mParam->GetVSCodeExePath());
}

int VSCodeWorkspaceCommand::Match(Pattern* pattern)
{
	return pattern->Match(PathFindFileName(in->mConfigPath), in->mParam->HasPrefix() ? 1 : 0);
}

launcherapp::core::Command*
VSCodeWorkspaceCommand::Clone()
{
	return new VSCodeWorkspaceCommand(in->mParam, in->mID, mName, in->mConfigPath);
}

CString VSCodeWorkspaceCommand::GetSourceName()
{
	return mName;
}

bool VSCodeWorkspaceCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString VSCodeWorkspaceCommand::TypeDisplayName()
{
	return _T("履歴(ワークスペース)");
}


} // end of namespace vscode
} // end of namespace commands
} // end of namespace launcherapp

