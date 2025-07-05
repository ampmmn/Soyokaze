#include "GetExplorerCurrentPathProxyCommand.h"
#include "ExplorerFunctions.h"
#include <windows.h>
#include "StringUtil.h"
#include <spdlog/spdlog.h>

using json = nlohmann::json;


namespace launcherproxy { 

REGISTER_PROXYCOMMAND(GetExplorerCurrentPathProxyCommand)

GetExplorerCurrentPathProxyCommand::GetExplorerCurrentPathProxyCommand()
{
}

GetExplorerCurrentPathProxyCommand::~GetExplorerCurrentPathProxyCommand()
{
}

std::string GetExplorerCurrentPathProxyCommand::GetName()
{
	return "getexplorercurrentdir";
}

bool GetExplorerCurrentPathProxyCommand::Execute(json& json_req, json& json_res)
{
	std::wstring curDir;
	Expr_GetCurrentDir(curDir);


	std::string tmp;
	json_res["path"] = utf2utf(curDir, tmp);
	json_res["result"] = std::wstring();

	return true;
}

} // end of namespace 



