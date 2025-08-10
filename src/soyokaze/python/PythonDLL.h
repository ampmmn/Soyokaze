#pragma once

#include <memory>

class PythonDLL
{
public:
	PythonDLL();
	~PythonDLL();

	bool SetDLLPath(const CString& dllPath);
	bool Evaluate(const CString& src, CString& result);

	void* NewInterpreter();
	void ReleaseInterpreter();

	bool IsErrorOccurred();
	void PrintError();
	void DecRef(void* obj);
	int EnsureGILState();
	void ReleaseGILState(int gstate);
	void* RunString(LPCSTR script);
	void* ReprObject(void* obj);
	void* AsEncodedString(void* obj, const char* encoding, const char* errors);
	const char* AsString(void* obj);


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

