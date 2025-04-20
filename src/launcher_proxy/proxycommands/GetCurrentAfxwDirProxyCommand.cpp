#include "GetCurrentAfxwDirProxyCommand.h"
#include <windows.h>
#include "processproxy/share/AfxWFunctions.h"
#include "StringUtil.h"
#include <spdlog/spdlog.h>

using json = nlohmann::json;



namespace launcherproxy { 

REGISTER_PROXYCOMMAND(GetCurrentAfxwDirProxyCommand)

GetCurrentAfxwDirProxyCommand::GetCurrentAfxwDirProxyCommand()
{
}

GetCurrentAfxwDirProxyCommand::~GetCurrentAfxwDirProxyCommand()
{
}

std::string GetCurrentAfxwDirProxyCommand::GetName()
{
	return "getcurrentafxwdir";
}

bool GetCurrentAfxwDirProxyCommand::Execute(json& json_req, json& json_res)
{
	std::wstring curDir;
	bool result = AfxW_GetCurrentDir(curDir);

	std::string tmp;
	json_res["path"] = utf2utf(curDir, tmp);
	json_res["result"] = result;

	return true;
}

} // end of namespace 



