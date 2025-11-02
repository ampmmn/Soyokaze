#include "OneNoteNavigateToProxyCommand.h"
#include <windows.h>
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy { 


struct OneNoteNavigateToProxyCommand::PImpl
{
	bool InitializeApp() {

		if (!(mApp == nullptr)) {
			// 初期化済
			return true;
		}

		CLSID CLSID_OneNote;
		HRESULT hr = CLSIDFromProgID(L"OneNote.Application.12", &CLSID_OneNote);
		if (FAILED(hr)) {
			return false;
		}
		hr = CoCreateInstance(CLSID_OneNote, nullptr, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&mApp);
		if (FAILED(hr)) {
			return false;
		}
		return true;
	}
	CComPtr<IDispatch> mApp;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



REGISTER_PROXYCOMMAND(OneNoteNavigateToProxyCommand)

OneNoteNavigateToProxyCommand::OneNoteNavigateToProxyCommand() : in(new PImpl)
{
}

OneNoteNavigateToProxyCommand::~OneNoteNavigateToProxyCommand()
{
}

std::string OneNoteNavigateToProxyCommand::GetName()
{
	return "onenote_navigateto";
}

bool OneNoteNavigateToProxyCommand::Execute(json& json_req, json& json_res)
{
	// mAppが未初期化なら初期化する
	if (in->InitializeApp() == false) {
		json_res["reason"] = "Failed to Initialize OneNote App.";
		return false;
	}

	std::wstring id;
	utf2utf(json_req["page_id"].get<std::string>().c_str(), id);

	VARIANT result;
	VariantInit(&result);

	VARIANT argEmptyStr;
	VariantInit(&argEmptyStr);
	argEmptyStr.vt = VT_BSTR;
	CComBSTR argEmptyVal(L"");
	argEmptyStr.bstrVal = argEmptyVal;

	VARIANT argPage;
	VariantInit(&argPage);
	argPage.vt = VT_BSTR;
	CComBSTR argPageVal(id.c_str());
	argPage.bstrVal = argPageVal;

	HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, in->mApp, L"NavigateTo", 2, &argEmptyStr, &argPage);
	if (FAILED(hr)) {
		spdlog::error("OneNoteAppProxy::NavigateTo failed hr={}", hr);
	}

	json_res["result"] = (SUCCEEDED(hr) != FALSE);
	return true;
}

} //"sheet does not found."end of namespace 



