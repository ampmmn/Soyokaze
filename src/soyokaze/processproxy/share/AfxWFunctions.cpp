#include "AfxWFunctions.h"
#include <windows.h>
#include <atlcomcli.h>
#include <regex>
#include <algorithm>

/**
	あふwのオートメーションオブジェクトを取得する
	@return true:取得成功 false:失敗
	@param[out] dispPtr 取得したIDispatch
*/
static bool GetDispatch(
	IDispatch** dispPtr
)
{
	CLSID clsId;
	HRESULT hr = CLSIDFromProgID(L"afxw.obj", &clsId);
	if (FAILED(hr)) {
		return false;
	}

	hr = CoCreateInstance(clsId, NULL, CLSCTX_ALL, IID_IDispatch, (void**)&dispPtr);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

/**
	あふwのカレントディレクトリを取得する
	@return true:取得成功  false:取得失敗
	@param[out] curDir カレントディレクトリ
*/
bool AfxW_GetCurrentDir(
	std::wstring& curDir
)
{
	CComPtr<IDispatch> pDisp;
	if (GetDispatch(&pDisp) == false) {
		return false;
	}
	if (pDisp == nullptr) {
		return false;
	}

	// "Extract $P"で現在の窓のパスを取得する
	CComBSTR extractStr(L"Extract");
	OLECHAR* p = extractStr;

	DISPID methodId;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &p, 1, LOCALE_USER_DEFAULT, &methodId);
	if (FAILED(hr)) {
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

	if (FAILED(hr) || vResult.bstrVal == nullptr) {
		return false;
	}

	curDir = (LPWSTR)vResult.bstrVal;
	return true;
}

/**
	あふwのカレントディレクトリを設定する
	@return true:成功  false:失敗
	@param[in] path 設定するカレントディレクトリ
*/
bool AfxW_SetCurrentDir(
	const std::wstring& path
)
{
	CComPtr<IDispatch> pDisp;
	if (GetDispatch(&pDisp) == false) {
		return false;
	}

	CComBSTR execStr(L"Exec");
	OLECHAR* p = execStr;

	DISPID methodId;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &p, 1, LOCALE_USER_DEFAULT, &methodId);
	if (FAILED(hr)) {
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
	if (FAILED(hr)) {
		return false;
	}
	return true;
}

// あふwの選択中のファイルパスを取得(単一)
bool AfxW_GetSelectionPath(std::wstring& selPath, int index)
{
	std::vector<std::wstring> paths;
	Afxw_GetAllSelectionPath(paths);
	if (index < 0 || index <= paths.size()) {
		return false;
	}
	selPath = paths[index];
	return true;
}

// あふwの選択中のファイルパスを取得(すべて)
bool Afxw_GetAllSelectionPath(std::vector<std::wstring>& paths)
{
	CComPtr<IDispatch> pDisp;
	if (GetDispatch(&pDisp) == false) {
		return false;
	}

	CComBSTR extractStr(L"Extract");
	OLECHAR* p = extractStr;

	DISPID methodId;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &p, 1, LOCALE_USER_DEFAULT, &methodId);
	if (FAILED(hr)) {
		return false;
	}

	
	CComBSTR cmdlineBstr(L"$MF");

	DISPPARAMS params = {};
	params.cArgs = 1;

	VARIANT args[1];
	params.rgvarg = args;
	args[0].vt = VT_BSTR;
	args[0].bstrVal = cmdlineBstr;

	CComVariant vResult;
	hr = pDisp->Invoke(methodId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, 
	                   &params, &vResult, NULL, NULL);

	if (FAILED(hr) || vResult.bstrVal == nullptr) {
		return false;
	}

	std::wstring pathStr((LPWSTR)vResult.bstrVal);

	// 個々の要素に分解する
	std::vector<std::wstring> result;

	std::wregex pattern(L"(\"([^\"]*)\")");
	std::wsmatch matches;

	std::wstring::const_iterator searchStart(pathStr.cbegin());
	while (std::regex_search(searchStart, pathStr.cend(), matches, pattern)) {
		auto path = matches[1].str();
		// ダブルクォーテーションを除去
    path.erase(std::remove(path.begin(), path.end(), L'"'), path.end());

		// キャプチャした内容を格納
		result.push_back(path); 
		// 次の検索位置に更新
		searchStart = matches.suffix().first; 
	}

	paths.swap(result);

	return true;
}


