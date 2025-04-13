#include "AfxWFunctions.h"
#include <windows.h>
#include <atlcomcli.h>

static IDispatch* GetDispatch()
{
	CLSID clsId;
	HRESULT hr = CLSIDFromProgID(L"afxw.obj", &clsId);
	if (FAILED(hr)) {
		return nullptr;
	}

	IDispatch* dispPtr = nullptr;
	hr = CoCreateInstance(clsId, NULL, CLSCTX_ALL, IID_IDispatch, (void**)&dispPtr);
	if (FAILED(hr)) {
		return nullptr;
	}

	return dispPtr;
}

bool AfxW_GetCurrentDir(std::wstring& curDir)
{
	IDispatch* pDisp = GetDispatch();
	if (pDisp == nullptr) {
		return false;
	}

	CComBSTR extractStr(L"Extract");
	OLECHAR* p = extractStr;

	DISPID methodId;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &p, 1, LOCALE_USER_DEFAULT, &methodId);
	if (FAILED(hr)) {
		pDisp->Release();
		return false;
	}

	
	CComBSTR cmdlineBstr(L"$P");

	DISPPARAMS params = {};
	params.cArgs = 1;

	VARIANT args[1];
	params.rgvarg = args;
	args[0].vt = VT_BSTR;
	args[0].bstrVal = cmdlineBstr;

	CComVariant vResult;
	hr = pDisp->Invoke(methodId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, 
	                   &params, &vResult, NULL, NULL);
	if (FAILED(hr)) {
		pDisp->Release();
		return false;
	}

	pDisp->Release();

	curDir = (LPWSTR)vResult.bstrVal;
	return true;
}

bool AfxW_SetCurrentDir(const std::wstring& path)
{
	IDispatch* pDisp = GetDispatch();
	if (pDisp == nullptr) {
		return false;
	}

	CComBSTR execStr(L"Exec");
	OLECHAR* p = execStr;

	DISPID methodId;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &p, 1, LOCALE_USER_DEFAULT, &methodId);
	if (FAILED(hr)) {
		pDisp->Release();
		return false;
	}

	std::wstring cmdline(L"&EXCD -P\"");
	cmdline += path;
	cmdline += L"\"";

	CComBSTR cmdlineBstr(cmdline.c_str());

	DISPPARAMS params = {};
	params.cArgs = 1;

	VARIANT args[1];
	params.rgvarg = args;
	args[0].vt = VT_BSTR;
	args[0].bstrVal = cmdlineBstr;

	CComVariant vResult;
	hr = pDisp->Invoke(methodId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, 
	                   &params, &vResult, NULL, NULL);
	pDisp->Release();

	if (FAILED(hr)) {
		return false;
	}
	return true;
}

