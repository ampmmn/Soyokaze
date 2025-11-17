#include "GetExplorerSelectionPathProxyCommand.h"
#include "ExplorerFunctions.h"
#include <windows.h>
#include "StringUtil.h"
#include <spdlog/spdlog.h>

using json = nlohmann::json;



namespace launcherproxy { 

REGISTER_PROXYCOMMAND(GetExplorerSelectionPathProxyCommand)

GetExplorerSelectionPathProxyCommand::GetExplorerSelectionPathProxyCommand()
{
}

GetExplorerSelectionPathProxyCommand::~GetExplorerSelectionPathProxyCommand()
{
}

std::string GetExplorerSelectionPathProxyCommand::GetName()
{
	return "getexplorerselectiondir";
}

bool GetExplorerSelectionPathProxyCommand::Execute(json& json_req, json& json_res)
{
	int index = json_req["index"].get<int>();

	std::vector<std::wstring> paths;

	if (index >= 0) {
		std::wstring selPath;
		if (Expr_GetSelectionPath(selPath, index)) {
			paths.push_back(selPath);
		}
	}
	else {
		Expr_GetAllSelectionPath(paths);
	}

	std::string tmp;

	auto json_items = json::array();
	for (auto& path : paths)  {
		auto json_item = json::object();
		json_item["path"] = utf2utf(path, tmp);
		json_items.push_back(json_item);
	}
	json_res["items"] = json_items;
	json_res["result"] = true;

	return true;
}

} // end of namespace 



