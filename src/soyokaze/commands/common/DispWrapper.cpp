#include "pch.h"
#include "DispWrapper.h"
#include "commands/common/AutoWrap.h"

namespace launcherapp { namespace commands { namespace common {

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

void DispWrapper::Release()
{
	mDispPtr.Release();
}

IDispatch** DispWrapper::operator &()
{
	return &mDispPtr;
}

DispWrapper::operator IDispatch*()
{
	return mDispPtr;
}


void DispWrapper::GetPropertyVariant(LPCOLESTR name, VARIANT& value)
{
	if (mDispPtr == nullptr) {
		return ;
	}

	VariantInit(&value);
	AutoWrap(DISPATCH_PROPERTYGET, &value, mDispPtr, name, 0);
}

int DispWrapper::GetPropertyInt(
		LPCOLESTR name
)
{
	if (mDispPtr == nullptr) {
		return 0;
	}
		
	VARIANT result;
	VariantInit(&result);

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 0);
	return result.intVal;
}

int64_t DispWrapper::GetPropertyInt64(
		LPCOLESTR name
)
{
	if (mDispPtr == nullptr) {
		return 0;
	}

	VARIANT result;
	VariantInit(&result);

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 0);
	return result.llVal;
}

CString DispWrapper::GetPropertyString(LPCOLESTR name)
{
	if (mDispPtr == nullptr) {
		return _T("");
	}

	VARIANT result;
	VariantInit(&result);

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 0);
	return CString(result.bstrVal);
}

CString DispWrapper::GetPropertyString(LPCOLESTR name, int index)
{
	if (mDispPtr == nullptr) {
		return _T("");
	}

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_INT;
	arg1.intVal = index;

	AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 1, &arg1);
	return CString(result.bstrVal);
}


bool DispWrapper::GetPropertyObject(LPCOLESTR name, DispWrapper& object)
{
	if (mDispPtr == nullptr) {
		return false;
	}

	VARIANT result;
	VariantInit(&result);

	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, mDispPtr, name, 0);
	if (FAILED(hr)) {
		return false;
	}
	object = result.pdispVal;	

	return true;
}

bool DispWrapper::GetPropertyObject(LPCOLESTR name, int16_t index, DispWrapper& object)
{
	if (mDispPtr == nullptr) {
		return false;
	}

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

bool DispWrapper::GetPropertyObject(LPCOLESTR name, int32_t index, DispWrapper& object)
{
	if (mDispPtr == nullptr) {
		return false;
	}

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

bool DispWrapper::GetPropertyObject(LPCOLESTR name, LPCOLESTR argName, DispWrapper& object)
{
	if (mDispPtr == nullptr) {
		return false;
	}

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


int DispWrapper::CallIntMethod(LPCOLESTR methodName, int defValue)
{
	if (mDispPtr == nullptr) {
		return defValue;
	}

	VARIANT result;
	VariantInit(&result);

	VariantInit(&result);
	HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 0);

	if (FAILED(hr)) {
		return defValue;
	}
	return result.intVal;
}

bool DispWrapper::CallBooleanMethod(LPCOLESTR methodName, bool defValue)
{
	if (mDispPtr == nullptr) {
		return defValue;
	}

	VARIANT result;
	VariantInit(&result);

	VariantInit(&result);
	HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 0);

	if (FAILED(hr)) {
		return defValue;
	}
	return result.boolVal;
}

CString DispWrapper::CallStringMethod(LPCOLESTR methodName, const CString& defValue)
{
	if (mDispPtr == nullptr) {
		return defValue;
	}

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

bool DispWrapper::CallObjectMethod(LPCOLESTR methodName, DispWrapper& object)
{
	if (mDispPtr == nullptr) {
		return false;
	}

	VARIANT result;
	VariantInit(&result);

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 0);
	object = result.pdispVal;

	return  (result.pdispVal != nullptr);
}

bool DispWrapper::CallObjectMethod(LPCOLESTR methodName, DispWrapper& param1, DispWrapper& object)
{
	if (mDispPtr == nullptr) {
		return false;
	}

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

bool DispWrapper::CallObjectMethod(LPCOLESTR methodName, LPCOLESTR param1, DispWrapper& object)
{
	if (mDispPtr == nullptr) {
		return false;
	}

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

bool DispWrapper::CallObjectMethod(LPCOLESTR methodName, int32_t param1, DispWrapper& object)
{
	if (mDispPtr == nullptr) {
		return false;
	}

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

bool DispWrapper::CallObjectMethod(LPCOLESTR methodName, int16_t param1, DispWrapper& object)
{
	if (mDispPtr == nullptr) {
		return false;
	}

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

void DispWrapper::CallVoidMethod(LPCOLESTR methodName)
{
	if (mDispPtr == nullptr) {
		return;
	}

	VARIANT result;
	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 0);
}

void DispWrapper::CallVoidMethod(LPCOLESTR methodName, IDispatch* param1)
{
	if (mDispPtr == nullptr) {
		return;
	}

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_DISPATCH;
	arg1.pdispVal = param1;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
}

void DispWrapper::CallVoidMethod(LPCOLESTR methodName, int16_t param1)
{
	if (mDispPtr == nullptr) {
		return ;
	}

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_I2;
	arg1.intVal = param1;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
}

void DispWrapper::CallVoidMethod(LPCOLESTR methodName, bool param1)
{
	if (mDispPtr == nullptr) {
		return;
	}

	VARIANT result;
	VariantInit(&result);

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_BOOL;
	arg1.boolVal = param1 ? VARIANT_TRUE : VARIANT_FALSE;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, mDispPtr, methodName, 1, &arg1);
}


}}}
