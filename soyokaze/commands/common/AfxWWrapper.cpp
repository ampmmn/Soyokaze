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
AfxWWrapper::AfxWWrapper() : in(new PImpl)
{
	CoInitialize(NULL);

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

	CComBSTR hisDirStr(L"HisDir");
	OLECHAR* p = hisDirStr;

	DISPID methodId;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &p, 1, LOCALE_USER_DEFAULT, &methodId);
	if (FAILED(hr)) {
		return _T("");
	}


	DISPPARAMS params = {};
	params.cArgs = 2;

	VARIANT args[2];
	params.rgvarg = args;
	args[0].intVal = 0;
	args[0].vt = VT_INT;
	args[1].intVal = in->GetCurrentWinNo();
	args[1].vt = VT_INT;

	CComVariant vResult;
	hr = pDisp->Invoke(methodId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, 
	                   &params, &vResult, NULL, NULL);
	if (FAILED(hr)) {
		return _T("");
	}

	pDisp->Release();

	return CString(CStringW((LPWSTR)vResult.bstrVal));
}

