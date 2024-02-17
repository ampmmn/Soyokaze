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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DispWrapper::DispWrapper()
{
}

DispWrapper::DispWrapper(IDispatch* disp) : mDispPtr(disp)
{
	if (disp) {
		disp->AddRef();
	}
}

DispWrapper::~DispWrapper()
{
}

IDispatch** DispWrapper::operator &()
{
	return &mDispPtr;
}

DispWrapper::operator IDispatch*()
{
	return mDispPtr;
}


void DispWrapper::GetPropertyVariant(LPOLESTR name, VARIANT& value)
{
	ASSERT(mDispPtr);

	VariantInit(&value);
	AutoWrap(DISPATCH_PROPERTYGET, &value, mDispPtr, name, 0);
}

int DispWrapper::GetPropertyInt(
		LPOLESTR name
)
{
	ASSERT(mDispPtr);

	int val = 0;

	VARIANT result;
	VariantInit(&result);

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 0);
	return result.intVal;
}

CString DispWrapper::GetPropertyString(LPOLESTR name)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 0);
	return CString(result.bstrVal);
}

CString DispWrapper::GetPropertyString(LPOLESTR name, int index)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_INT;
	arg1.intVal = index;

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 1, &arg1);
	return CString(result.bstrVal);
}


bool DispWrapper::GetPropertyObject(LPOLESTR name, DispWrapper& object)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 0);
	if (FAILED(hr)) {
		return false;
	}
	object = result.pdispVal;	

	return true;
}

bool DispWrapper::GetPropertyObject(LPOLESTR name, int16_t index, DispWrapper& object)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_I2;
	arg1.intVal = index;

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 1, &arg1);
	object = result.pdispVal;

	return true;
}

bool DispWrapper::GetPropertyObject(LPOLESTR name, int32_t index, DispWrapper& object)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_INT;
	arg1.intVal = index;

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 1, &arg1);
	object = result.pdispVal;

	return true;
}

bool DispWrapper::GetPropertyObject(LPOLESTR name, LPOLESTR argName, DispWrapper& object)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	CComBSTR nameStr(argName);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_BSTR;
	arg1.bstrVal = nameStr;

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 1, &arg1);
	object = result.pdispVal;

	return true;
}


int DispWrapper::CallIntMethod(LPOLESTR methodName, int defValue)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VariantInit(&result);
	HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 0);

	if (FAILED(hr)) {
		return defValue;
	}
	return result.intVal;
}

bool DispWrapper::CallBooleanMethod(LPOLESTR methodName, bool defValue)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VariantInit(&result);
	HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 0);

	if (FAILED(hr)) {
		return defValue;
	}
	return result.boolVal;
}

CString DispWrapper::CallStringMethod(LPOLESTR methodName, const CString& defValue)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VariantInit(&result);
	HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 0);

	if (FAILED(hr)) {
		return defValue;
	}

	CComBSTR bstrVal;
	bstrVal = result.bstrVal;
	return CString(bstrVal);
}

bool DispWrapper::CallObjectMethod(LPOLESTR methodName, DispWrapper& object)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 0);
	object = result.pdispVal;

	return  (result.pdispVal != nullptr);
}

bool DispWrapper::CallObjectMethod(LPOLESTR methodName, DispWrapper& param1, DispWrapper& object)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_DISPATCH;
	arg1.pdispVal = param1;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
	object = result.pdispVal;

	return  (result.pdispVal != nullptr);
}

bool DispWrapper::CallObjectMethod(LPOLESTR methodName, LPOLESTR param1, DispWrapper& object)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	CComBSTR argVal(param1);
	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_BSTR;
	arg1.bstrVal = argVal;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
	object = result.pdispVal;

	return  (result.pdispVal != nullptr);
}

bool DispWrapper::CallObjectMethod(LPOLESTR methodName, int32_t param1, DispWrapper& object)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_INT;
	arg1.intVal = param1;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
	object = result.pdispVal;

	return  (result.pdispVal != nullptr);
}

bool DispWrapper::CallObjectMethod(LPOLESTR methodName, int16_t param1, DispWrapper& object)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_I2;
	arg1.intVal = param1;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
	object = result.pdispVal;

	return  (result.pdispVal != nullptr);
}

void DispWrapper::CallVoidMethod(LPOLESTR methodName)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 0);
}

void DispWrapper::CallVoidMethod(LPOLESTR methodName, IDispatch* param1)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_DISPATCH;
	arg1.pdispVal = param1;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
}

void DispWrapper::CallVoidMethod(LPOLESTR methodName, int16_t param1)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_I2;
	arg1.intVal = param1;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
}

void DispWrapper::CallVoidMethod(LPOLESTR methodName, bool param1)
{
	ASSERT(mDispPtr);

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_BOOL;
	arg1.boolVal = param1 ? VARIANT_TRUE : VARIANT_FALSE;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
}


}
}
}
