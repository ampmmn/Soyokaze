#include "pch.h"
#include "AfxWWrapper.h"


struct AfxWWrapper::PImpl
{
	IDispatch* GetDispatch();
	int GetCurrentWinNo();


	IDispatch* mDispPtr;
};

/**
  あふのIDispatchオブジェクトを取得する
 	@return IDispatchオブジェクト
*/
IDispatch* AfxWWrapper::PImpl::GetDispatch()
{
	if (mDispPtr) {
		return mDispPtr;
	}

	CLSID clsId;
	HRESULT hr = CLSIDFromProgID(L"afxw.obj", &clsId);
	if (FAILED(hr)) {
		return nullptr;
	}

	hr = CoCreateInstance(clsId, NULL, CLSCTX_ALL, IID_IDispatch, (void**)&mDispPtr);
	if (FAILED(hr)) {
		return nullptr;
	}

	return mDispPtr;
}

/**
  あふの自窓の番号を取得(0:Left 1:Right)
 	@return 窓番号
*/
int AfxWWrapper::PImpl::GetCurrentWinNo()
{
	CComBSTR srcWinName(L"SrcWin");
	OLECHAR* p = srcWinName;

	IDispatch* pDisp = GetDispatch();
	if (pDisp == nullptr) {
		return -1;
	}

	DISPID methodId;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &p, 1, LOCALE_USER_DEFAULT, &methodId);
	if (FAILED(hr)) {
		return -1;
	}

	DISPPARAMS params = {};
	params.cArgs = 0;

	CComVariant vResult;

	hr = pDisp->Invoke(methodId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, 
	                   &params, &vResult, NULL, NULL);

	return vResult.intVal;
}


// コンストラクタ
AfxWWrapper::AfxWWrapper() : in(std::make_unique<PImpl>())
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		SPDLOG_ERROR(_T("Failed to CoInitialize!"));
	}

	in->mDispPtr = nullptr;
}

// デストラクタ
AfxWWrapper::~AfxWWrapper()
{
	if (in->mDispPtr) {
		in->mDispPtr->Release();
	}

	CoUninitialize();
}

/**
  自窓のディレクトリパスを取得
 	@return 自窓のディレクトリパス
*/
CString AfxWWrapper::GetCurrentDir()
{
	IDispatch* pDisp = in->GetDispatch();
	if (pDisp == nullptr) {
		return _T("");
	}

	CComBSTR extractStr(L"Extract");
	OLECHAR* p = extractStr;

	DISPID methodId;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &p, 1, LOCALE_USER_DEFAULT, &methodId);
	if (FAILED(hr)) {
		return _T("");
	}

	CStringW cmdline(L"$P");
	CComBSTR cmdlineBstr(cmdline);

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
		return _T("");
	}

	pDisp->Release();

	return CString(CStringW((LPWSTR)vResult.bstrVal));
}

// 自窓のカレントディレクトリを移動
bool AfxWWrapper::SetCurrentDir(const CString& path)
{
	IDispatch* pDisp = in->GetDispatch();
	if (pDisp == nullptr) {
		return false;
	}

	CComBSTR execStr(L"Exec");
	OLECHAR* p = execStr;

	DISPID methodId;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &p, 1, LOCALE_USER_DEFAULT, &methodId);
	if (FAILED(hr)) {
		return false;
	}

	CStringW cmdline;
#ifdef UNICODE
	cmdline.Format(L"&EXCD -P\"%s\"", (LPCWSTR)path);
#else
	cmdline.Format(L"&EXCD -P\"%s\"", (LPCWSTR)(CStringW)path);
#endif

	CComBSTR cmdlineBstr(cmdline);

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
	pDisp->Release();

	return true;
}


