#pragma once

namespace launcherapp { namespace commands { namespace common {

class DispWrapper
{
public:
	DispWrapper();
	DispWrapper(IDispatch* disp);
	~DispWrapper();

	void Release();

	IDispatch** operator &();
	operator IDispatch*();

	void GetPropertyVariant(LPOLESTR name, VARIANT& value);

	int GetPropertyInt(LPOLESTR name);
	int64_t GetPropertyInt64(LPOLESTR name);
	CString GetPropertyString(LPOLESTR name);
	CString GetPropertyString(LPOLESTR name, int index);
	bool GetPropertyObject(LPOLESTR name, DispWrapper& object);
	bool GetPropertyObject(LPOLESTR name, int16_t index, DispWrapper& object);
	bool GetPropertyObject(LPOLESTR name, int32_t index, DispWrapper& object);
	bool GetPropertyObject(LPOLESTR name, LPOLESTR argName, DispWrapper& object);

	int CallIntMethod(LPOLESTR methodName, int defValue);

	bool CallBooleanMethod(LPOLESTR methodName, bool defValue);

	CString CallStringMethod(LPOLESTR methodName, const CString& defValue);

	bool CallObjectMethod(LPOLESTR methodName, DispWrapper& object);
	bool CallObjectMethod(LPOLESTR methodName, DispWrapper& param1, DispWrapper& object);
	bool CallObjectMethod(LPOLESTR methodName, LPOLESTR param1, DispWrapper& object);
	bool CallObjectMethod(LPOLESTR methodName, int32_t param1, DispWrapper& object);
	bool CallObjectMethod(LPOLESTR methodName, int16_t param1, DispWrapper& object);

	void CallVoidMethod(LPOLESTR methodName);
	void CallVoidMethod(LPOLESTR methodName, IDispatch* param1);
	void CallVoidMethod(LPOLESTR methodName, int16_t param1);
	void CallVoidMethod(LPOLESTR methodName, bool param1);

private:
	CComPtr<IDispatch> mDispPtr;
};

}}}

