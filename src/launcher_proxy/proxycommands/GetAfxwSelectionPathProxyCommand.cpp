#include "GetAfxwSelectionPathProxyCommand.h"
#include <windows.h>
#include "processproxy/share/AfxWFunctions.h"
#include "StringUtil.h"
#include <spdlog/spdlog.h>

using json = nlohmann::json;



namespace launcherproxy { 

REGISTER_PROXYCOMMAND(GetAfxwSelectionPathProxyCommand)

GetAfxwSelectionPathProxyCommand::GetAfxwSelectionPathProxyCommand()
{
}

GetAfxwSelectionPathProxyCommand::~GetAfxwSelectionPathProxyCommand()
{
}

std::string GetAfxwSelectionPathProxyCommand::GetName()
{
	return "getafxselectionpath";
}

bool GetAfxwSelectionPathProxyCommand::Execute(json& json_req, json& json_res)
{
	int index = json_req["index"].get<int>();

	std::vector<std::wstring> paths;

	if (index >= 0) {
		std::wstring selPath;
		if (AfxW_GetSelectionPath(selPath, index)) {
			paths.push_back(selPath);
		}
	}
	else {
		Afxw_GetAllSelectionPath(paths);
	}

	std::string tmp;

	auto json_items = json::array();
	for (auto& path : paths)  {
		auto json_item = json::object();
		json_item["path"] = utf2utf(path, tmp);
		json_items.push_back(json_item);
	}
	json_res["items"] = json_items;
	json_res["result"] = std::wstring();

	return true;
}

} // end of namespace 



