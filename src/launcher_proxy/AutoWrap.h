#pragma once

#include <stdint.h>
#include <atlbase.h>
#include <string>

HRESULT AutoWrap(int autoType, VARIANT* pvResult, IDispatch* pDisp, const wchar_t* ptName, int cArgs...);


class DispWrapper
{
public:
	DispWrapper();
	DispWrapper(IDispatch* disp);
	~DispWrapper();

	void Release();

	IDispatch** operator &();
	operator IDispatch*();

	void GetPropertyVariant(LPCOLESTR name, VARIANT& value);

	int GetPropertyInt(LPCOLESTR name);
	int64_t GetPropertyInt64(LPCOLESTR name);
	std::wstring GetPropertyString(LPCOLESTR name);
	std::wstring GetPropertyString(LPCOLESTR name, int index);
	bool GetPropertyObject(LPCOLESTR name, DispWrapper& object);
	bool GetPropertyObject(LPCOLESTR name, int16_t index, DispWrapper& object);
	bool GetPropertyObject(LPCOLESTR name, int32_t index, DispWrapper& object);
	bool GetPropertyObject(LPCOLESTR name, LPCOLESTR argName, DispWrapper& object);

	int CallIntMethod(LPCOLESTR methodName, int defValue);

	bool CallBooleanMethod(LPCOLESTR methodName, bool defValue);

	std::wstring CallStringMethod(LPCOLESTR methodName, const std::wstring& defValue);

	bool CallObjectMethod(LPCOLESTR methodName, DispWrapper& object);
	bool CallObjectMethod(LPCOLESTR methodName, DispWrapper& param1, DispWrapper& object);
	bool CallObjectMethod(LPCOLESTR methodName, LPCOLESTR param1, DispWrapper& object);
	bool CallObjectMethod(LPCOLESTR methodName, int32_t param1, DispWrapper& object);
	bool CallObjectMethod(LPCOLESTR methodName, int16_t param1, DispWrapper& object);

	void CallVoidMethod(LPCOLESTR methodName);
	void CallVoidMethod(LPCOLESTR methodName, IDispatch* param1);
	void CallVoidMethod(LPCOLESTR methodName, int16_t param1);
	void CallVoidMethod(LPCOLESTR methodName, bool param1);

private:
	CComPtr<IDispatch> mDispPtr;
};

