#include "OutlookIsAppRunningProxyCommand.h"
#include <windows.h>
#include "StringUtil.h"
#include <atlbase.h>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy { 


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



REGISTER_PROXYCOMMAND(OutlookIsAppRunningProxyCommand)

OutlookIsAppRunningProxyCommand::OutlookIsAppRunningProxyCommand()
{
}

OutlookIsAppRunningProxyCommand::~OutlookIsAppRunningProxyCommand()
{
}

std::string OutlookIsAppRunningProxyCommand::GetName()
{
	return "outlook_isapprunning";
}

bool OutlookIsAppRunningProxyCommand::Execute(json& json_req, json& json_res)
{
	// OutlookのCLSIDを取得
	CLSID CLSID_Outlook;
	HRESULT hr = CLSIDFromProgID(L"Outlook.Application", &CLSID_Outlook);
	if (FAILED(hr)) {
		return false;
	}

	// 起動中のOutlook.Applicationインスタンスを取得する
	CComPtr<IUnknown> unk;
	hr = GetActiveObject(CLSID_Outlook, nullptr, &unk);
	if (FAILED(hr)) {
		return false;
	}

	json_res["result"] = (SUCCEEDED(hr) != FALSE);
	return true;
}

} //"sheet does not found."end of namespace 



