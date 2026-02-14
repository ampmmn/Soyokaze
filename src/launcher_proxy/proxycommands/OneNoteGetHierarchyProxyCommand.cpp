#include "OneNoteGetHierarchyProxyCommand.h"
#include "onenote/OneNoteAppProxy.h"
#include <windows.h>
#include "StringUtil.h"

using json = nlohmann::json;

namespace launcherproxy { 

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


REGISTER_PROXYCOMMAND(OneNoteGetHierarchyProxyCommand)

OneNoteGetHierarchyProxyCommand::OneNoteGetHierarchyProxyCommand()
{
}

OneNoteGetHierarchyProxyCommand::~OneNoteGetHierarchyProxyCommand()
{
}

std::string OneNoteGetHierarchyProxyCommand::GetName()
{
	return "onenote_gethierarchy";
}

bool OneNoteGetHierarchyProxyCommand::Execute(json& json_req, json& json_res)
{
	auto appProxy = OneNoteAppProxy::GetInstance();

	json books = {};
	if (appProxy->GetHierarchy(books) == false) {
		json_res["reason"] = appProxy->GetErrorMessage();
		json_res["result"] = false;
		return false;
	}

	json_res["result"] = true;
	json_res["books"] = books;

	return true;
}

} //"sheet does not found."end of namespace 



