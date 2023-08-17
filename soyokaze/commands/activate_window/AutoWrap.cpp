#include "pch.h"
#include "AutoWrap.h"

namespace soyokaze {
namespace commands {
namespace activate_window {

// 
/**
 	IDispatchのプロパティ/メソッドにアクセスする
 	
 	このメソッドは可変長引数をうけとる。
 	メソッドに渡す引数がある場合は、cArgsの後ろに、cArgsで指定した数ぶんのVARIANTを指定する。

 	下記URLに掲載されているコードをベースにしている。
 	https://learn.microsoft.com/en-us/previous-versions/office/troubleshoot/office-developer/automate-excel-from-c

 	@return 処理結果HRESULT
 	@param[in]     autoType 種別(DISPATCH_PROPERTYGET or DISPATCH_PROPERTYPUT)
 	@param[out]    pvResult 呼び出し結果
 	@param[in,out] pDisp    IDispatchオブジェクト
 	@param[in]     ptName   プロパティ/メソッドの名前
 	@param[in]     cArgs    引数の数
*/
HRESULT AutoWrap(
	int autoType,
	VARIANT* pvResult,
	IDispatch* pDisp,
	LPOLESTR ptName,
	int cArgs...
)
{
	if (!pDisp) {
		return E_FAIL;
	}

	va_list marker;
	va_start(marker, cArgs);

	DISPID dispID;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
	if (FAILED(hr)) {
		return hr;
	}

	std::vector<VARIANT> args(cArgs + 1);
	for (int i = 0; i < cArgs; i++) {
		args[i] = va_arg(marker, VARIANT);
	}
	va_end(marker);

	DISPPARAMS dp = { NULL, NULL, 0, 0 };
	dp.cArgs = cArgs;
	dp.rgvarg = &args.front();

	DISPID dispidNamed = DISPID_PROPERTYPUT;
	if (autoType & DISPATCH_PROPERTYPUT) {
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}

	return pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL);
}



}
}
}
