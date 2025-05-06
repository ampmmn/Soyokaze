#include "pch.h"
#include "framework.h"
#include "RemoteAdhocCommand.h"
#include "commands/remote/RemoteClient.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>
#include <nlohmann/json.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using json = nlohmann::json;

namespace launcherapp { namespace commands { namespace remote {

struct RemoteAdhocCommand::PImpl
{
	RemoteClient* mClient{nullptr};
	CString mTypeName;
	CString mQueryString;
	int mIndex{0};
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(RemoteAdhocCommand)

RemoteAdhocCommand::RemoteAdhocCommand(
	RemoteClient* client,
 	LPCWSTR name,
 	LPCWSTR description,
 	LPCWSTR typeName,
	LPCWSTR queryString,
	int index
) : 
	AdhocCommandBase(name, description),
	in(std::make_unique<PImpl>())
{
	in->mClient = client;
	in->mTypeName = typeName;
	in->mQueryString = queryString;
	in->mIndex = index;
}

RemoteAdhocCommand::~RemoteAdhocCommand()
{
}

RemoteAdhocCommand* RemoteAdhocCommand::CreateInstance(
	RemoteClient* client,
 	LPCWSTR name,
 	LPCWSTR description,
 	LPCWSTR typeName,
	LPCWSTR queryString,
 	int index
)
{
	return new RemoteAdhocCommand(client, name, description, typeName, queryString, index);
}

CString RemoteAdhocCommand::GetGuideString()
{
	CString guideStr(_T("⏎:実行"));
	return guideStr;
}

CString RemoteAdhocCommand::GetTypeDisplayName()
{
	return in->mTypeName;
}

BOOL RemoteAdhocCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	auto& client = in->mClient;

	std::string dst;

	json json_req = json::object();
	json_req["command"] = "run";
	json_req["query"] = UTF2UTF(in->mQueryString, dst);
	json_req["index"] = in->mIndex;

	if (client->SendRequest(json_req) == false) {
		return FALSE;
	}

	json json_res = json::object();
	if (client->ReceiveResponse(json_res) == false) {
		return FALSE;
	}

	if (json_res.find("result") == json_res.end()) {
		return FALSE;
	}
	if (json_res["result"].get<bool>() == false) {
		auto reason = json_res["reason"].get<std::string>();
		spdlog::error("[remote] server returned error err:{}", reason);
		return FALSE;
	}

	return TRUE;
}

HICON RemoteAdhocCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-171);
}

launcherapp::core::Command*
RemoteAdhocCommand::Clone()
{
	return new RemoteAdhocCommand(in->mClient, this->mName, this->mDescription, in->mTypeName,
	                              in->mQueryString, in->mIndex);
}

}}} // end of namespace launcherapp::commands::remote

