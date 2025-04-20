#include "SetCurrentAfxwDirProxyCommand.h"
#include <windows.h>
#include "processproxy/share/AfxWFunctions.h"
#include "StringUtil.h"
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy { 

REGISTER_PROXYCOMMAND(SetCurrentAfxwDirProxyCommand)

SetCurrentAfxwDirProxyCommand::SetCurrentAfxwDirProxyCommand()
{
}

SetCurrentAfxwDirProxyCommand::~SetCurrentAfxwDirProxyCommand()
{
}

std::string SetCurrentAfxwDirProxyCommand::GetName()
{
	return "setcurrentafxwdir";
}

bool SetCurrentAfxwDirProxyCommand::Execute(json& json_req, json& json_res)
{
	try {
		std::string path_str = json_req["path"]; 
		std::wstring path;
		utf2utf(path_str, path);

		bool result = AfxW_SetCurrentDir(path);
		json_res["result"] = result;
		return true;
	}
	catch(...) {
		return false;
 	}
}

} // end of namespace 



