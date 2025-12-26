#include "pch.h"
#include "VSCodeFileCommand.h"
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

struct VSCodeFileCommand::PImpl
{
	CString mFilePath;
	const CommandParam* mParam{nullptr};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(VSCodeFileCommand)

VSCodeFileCommand::VSCodeFileCommand(
	const CommandParam* param,
 	const CString& displayName,
 	const CString& localPath
) : 
	AdhocCommandBase(_T(""), localPath),
	in(std::make_unique<PImpl>())
{
	mName = param->HasPrefix() ? param->mPrefix + _T(" ") + displayName : displayName;
	in->mFilePath = localPath;
	in->mParam = param;
}

VSCodeFileCommand::~VSCodeFileCommand()
{
}

bool VSCodeFileCommand::Create(nlohmann::json& json, CommandParam* param, Command** cmd)
{
	// 	{"fileUri":"file:///path/to/file"},
	try {
		auto uri = json.find("fileUri");
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

		*cmd = new VSCodeFileCommand(param, displayName, (LPCTSTR)path);
		return true;
	}
	catch(json::exception& e) {
		spdlog::warn("failed to parse fileUri entry. err:{}", e.what());
		return false;
	}
	return true;
}

CString VSCodeFileCommand::GetTypeDisplayName()
{
	return VSCodeFileCommand::TypeDisplayName();
}

bool VSCodeFileCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags == 0) {
		*action = new VSCodeExecuteAction(in->mParam, in->mFilePath, false);
		return true;
	}
	else if (modifierFlags == MOD_SHIFT) {
		*action = new VSCodeExecuteAction(in->mParam, in->mFilePath, true);
		return true;
	}
	else if (modifierFlags == MOD_CONTROL) {
		*action = new actions::clipboard::CopyTextAction(in->mFilePath);
		return true;
	}
	return false;
}


HICON VSCodeFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mParam->GetVSCodeExePath());
}

int VSCodeFileCommand::Match(Pattern* pattern)
{
	return pattern->Match(PathFindFileName(in->mFilePath), in->mParam->HasPrefix() ? 1 : 0);
}

launcherapp::core::Command*
VSCodeFileCommand::Clone()
{
	return new VSCodeFileCommand(in->mParam, mName, in->mFilePath);
}

CString VSCodeFileCommand::GetSourceName()
{
	return mName;
}

bool VSCodeFileCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString VSCodeFileCommand::TypeDisplayName()
{
	return _T("履歴(ファイル)");
}


} // end of namespace vscode
} // end of namespace commands
} // end of namespace launcherapp

