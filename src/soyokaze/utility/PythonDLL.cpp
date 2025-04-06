#include "pch.h"
#include "PythonDLL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//static const int Py_single_input = 256;    // defined in compile.h
static const int Py_file_input = 257;      // defined in compile.h
static const int Py_eval_input = 258;    // defined in compile.h

typedef void (*PY_INITIALIZE)(void);
typedef int (*PY_FINALIZEEX)(void);
typedef int (*PYGILSTATE_ENSURE)(void);
typedef void* (*PYRUN_STRING)(const char*, int, void*, void*);
typedef char* (*PYSTRING_ASSTRING)(void*);
typedef void* (*PYERR_OCCURRED)(void);
typedef void (*PY_DECREF)(void*);
typedef void (*PYGILSTATE_RELEASE)(int);
typedef void* (*PYIMPORT_ADDMODULE)(const char*);
typedef void* (*PYMODULE_GETDICT)(void*);
typedef void (*PYERR_PRINT)(void);
typedef void* (*PYOBJECT_REPR)(void*);
typedef void* (*PYUNICODE_ASENCODEDSTRING)(void*, const char*, const char*);
typedef void* (*PYIMPORT_IMPORTMODULE)(const char*);
typedef int (*PYMAPPING_SETITEMSTRING)(void*, const char*, void*);
typedef void* (*PY_COMPILESTRING)(const char *, const char *, int);
typedef void* (*PYOBJECT_GETATTRSTRING)(void* o, const char *attr_name);
typedef int (*PYDICT_MERGE)(void*, void*, int);
typedef void* (*PYEVAL_GetGlobals)(void);

struct PythonDLL::PImpl
{
	bool Initialize();
	void Finalize();

	CString mDllPath;

	HMODULE mDll = nullptr;
	void* mModule = nullptr;
	void* mDict = nullptr;

	PY_INITIALIZE mPy_Initialize = nullptr;
	PY_FINALIZEEX mPy_FinalizeEx = nullptr;
	PYGILSTATE_ENSURE mPyGILState_Ensure = nullptr;
	PYRUN_STRING mPyRun_String = nullptr;
	PYSTRING_ASSTRING mPyString_AsString = nullptr;
	PYERR_OCCURRED mPyErr_Occurred = nullptr;
	PY_DECREF mPy_DecRef = nullptr;
	PYGILSTATE_RELEASE mPyGILState_Release = nullptr;
	PYIMPORT_ADDMODULE mPyImport_AddModule = nullptr;
	PYMODULE_GETDICT mPyModule_GetDict = nullptr;
	PYERR_PRINT mPyErr_Print = nullptr;
	PYOBJECT_REPR mPyObject_Repr = nullptr;
	PYUNICODE_ASENCODEDSTRING mPyUnicode_AsEncodedString = nullptr;
	PYIMPORT_IMPORTMODULE mPyImport_ImportModule = nullptr;
	PYMAPPING_SETITEMSTRING mPyMapping_SetItemString = nullptr;
	PY_COMPILESTRING mPy_CompileString = nullptr;
  PYOBJECT_GETATTRSTRING mPyObject_GetAttrString = nullptr;
	PYDICT_MERGE mPyDict_Merge = nullptr;
	PYEVAL_GetGlobals mPyEval_GetGlobals = nullptr;
};

bool PythonDLL::PImpl::Initialize()
{
	// DLLのロード
	if (mDll == nullptr) {
		mDll = LoadLibraryEx(mDllPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

		if (mDll == nullptr) {
			return false;
		}
	}

	if (mPy_Initialize) {
		// Py_Initializeロード墨
		return true;
	}

	// クラス内で使用するAPIを取得

	mPy_Initialize = (PY_INITIALIZE)GetProcAddress(mDll, "Py_Initialize");
	if (mPy_Initialize == nullptr) {
		FreeLibrary(mDll);
		mPy_Initialize = nullptr;
		return false;
	}
	mPy_FinalizeEx = (PY_FINALIZEEX)GetProcAddress(mDll, "Py_FinalizeEx");
	mPyGILState_Ensure = (PYGILSTATE_ENSURE)GetProcAddress(mDll, "PyGILState_Ensure");
	mPyRun_String = (PYRUN_STRING)GetProcAddress(mDll, "PyRun_String");
	mPyString_AsString = (PYSTRING_ASSTRING)GetProcAddress(mDll, "PyBytes_AsString");
	mPyErr_Occurred = (PYERR_OCCURRED)GetProcAddress(mDll, "PyErr_Occurred");
	mPy_DecRef = (PY_DECREF)GetProcAddress(mDll, "Py_DecRef");
	mPyGILState_Release = (PYGILSTATE_RELEASE)GetProcAddress(mDll, "PyGILState_Release");
	mPyImport_AddModule = (PYIMPORT_ADDMODULE)GetProcAddress(mDll, "PyImport_AddModule");
	mPyModule_GetDict = (PYMODULE_GETDICT)GetProcAddress(mDll, "PyModule_GetDict");
	mPyErr_Print = (PYERR_PRINT)GetProcAddress(mDll, "PyErr_Print");
	mPyObject_Repr = (PYOBJECT_REPR)GetProcAddress(mDll, "PyObject_Repr");
	mPyUnicode_AsEncodedString = (PYUNICODE_ASENCODEDSTRING)GetProcAddress(mDll, "PyUnicode_AsEncodedString");
	mPyImport_ImportModule = (PYIMPORT_IMPORTMODULE)GetProcAddress(mDll, "PyImport_ImportModule");
	mPyMapping_SetItemString = (PYMAPPING_SETITEMSTRING)GetProcAddress(mDll, "PyMapping_SetItemString");
	mPy_CompileString = (PY_COMPILESTRING)GetProcAddress(mDll, "Py_CompileString");
	mPyDict_Merge = (PYDICT_MERGE)GetProcAddress(mDll, "PyDict_Merge");
	mPyEval_GetGlobals = (PYEVAL_GetGlobals)GetProcAddress(mDll, "PyEval_GetGlobals");

	// 初期化
	mPy_Initialize();

	// 辞書生成
	mModule = mPyImport_AddModule("__main__");
  mDict = mPyModule_GetDict(mModule);

	void* pyMathModule = mPyImport_ImportModule("math");
	void* matchDict = mPyModule_GetDict(pyMathModule);
	mPyDict_Merge(mDict, matchDict, 1);

	mPy_DecRef(pyMathModule);
	return true;
}

void PythonDLL::PImpl::Finalize()
{
	if (mDict) {
		mPy_DecRef(mDict);
		mDict = nullptr;
	}
	if (mModule) {
		mPy_DecRef(mModule);
		mModule = nullptr;
	}
	if (mPy_FinalizeEx) {
		mPy_FinalizeEx();
	}

	if (mDll) {
		FreeLibrary(mDll);
		mDll = nullptr;

		mPy_Initialize = nullptr;
		mPy_FinalizeEx = nullptr;
		mPyGILState_Ensure = nullptr;
		mPyRun_String = nullptr;
		mPyString_AsString = nullptr;
		mPyErr_Occurred = nullptr;
		mPy_DecRef = nullptr;
		mPyGILState_Release = nullptr;
		mPyImport_AddModule = nullptr;
		mPyModule_GetDict = nullptr;
		mPyErr_Print = nullptr;
		mPyObject_Repr = nullptr;
		mPyUnicode_AsEncodedString = nullptr;
	}
}


PythonDLL::PythonDLL() : in(new PImpl)
{
	in->mDll = nullptr;
	in->mModule = nullptr;
	in->mDict = nullptr;

	in->mPy_Initialize = nullptr;
	in->mPy_FinalizeEx = nullptr;
	in->mPyGILState_Ensure = nullptr;
	in->mPyRun_String = nullptr;
	in->mPyString_AsString = nullptr;
	in->mPyErr_Occurred = nullptr;
	in->mPy_DecRef = nullptr;
	in->mPyGILState_Release = nullptr;
	in->mPyImport_AddModule = nullptr;
	in->mPyModule_GetDict = nullptr;
	in->mPyErr_Print = nullptr;
	in->mPyObject_Repr = nullptr;
	in->mPyUnicode_AsEncodedString = nullptr;
}

PythonDLL::~PythonDLL()
{
	in->Finalize();
}

bool PythonDLL::SetDLLPath(const CString& dllPath)
{
	in->mDllPath = dllPath;
	in->Finalize();
	return true;
}

bool PythonDLL::Evaluate(const CString& src, CString& result)
{
	if (in->Initialize() == false) {
		return false;
	}

	int gstate = in->mPyGILState_Ensure();

	CStringA srcA(src);
	void* pyObject = in->mPyRun_String((LPCSTR)srcA, Py_eval_input, in->mDict, in->mDict);

	// 文をインタープリタ側で評価した結果エラーだった場合
	void* errObj = in->mPyErr_Occurred();
	if (errObj != nullptr) {

		// for debug
		in->mPyErr_Print();

		if (pyObject) {
			in->mPy_DecRef(pyObject);
		}

		in->mPyGILState_Release(gstate);
		return false;
	}

	if (pyObject == nullptr) {
		in->mPyGILState_Release(gstate);
		return false;
	}

	void* repr = in->mPyObject_Repr(pyObject);
	void* str = in->mPyUnicode_AsEncodedString(repr, "utf-8", "~E~");
	const char* s = in->mPyString_AsString(str);
	result = CStringA(s);

	if (repr) {
		in->mPy_DecRef(repr);
	}
	if (str) {
		in->mPy_DecRef(str);
	}
	if (pyObject) {
		in->mPy_DecRef(pyObject);
	}

	in->mPyGILState_Release(gstate);

	return true;
}


bool PythonDLL::SingleInput(const CString& src, CString& result)
{
	if (in->Initialize() == false) {
		return false;
	}

	int gstate = in->mPyGILState_Ensure();

	CStringA srcA(src);
	void* pyObject = in->mPyRun_String((LPCSTR)srcA, Py_file_input, in->mDict, in->mDict);

	// 文をインタープリタ側で評価した結果エラーだった場合
	void* errObj = in->mPyErr_Occurred();
	if (errObj != nullptr) {

		// for debug
		in->mPyErr_Print();

		if (pyObject) {
			in->mPy_DecRef(pyObject);
		}

		in->mPyGILState_Release(gstate);
		return false;
	}

	if (pyObject == nullptr) {
		in->mPyGILState_Release(gstate);
		return false;
	}

	void* pyObj2 = in->mPyObject_GetAttrString(in->mModule, "_");

	void* repr = in->mPyObject_Repr(pyObj2);
	void* str = in->mPyUnicode_AsEncodedString(repr, "utf-8", "~E~");

	const char* s = in->mPyString_AsString(str);
	result = CStringA(s);

	if (repr) {
		in->mPy_DecRef(repr);
	}
	if (str) {
		in->mPy_DecRef(str);
	}
	if (pyObject) {
		in->mPy_DecRef(pyObject);
	}

	in->mPyGILState_Release(gstate);

	return true;
}

bool PythonDLL::CanCompile(const CString& src)
{
	if (in->Initialize() == false) {
		return false;
	}

	int gstate = in->mPyGILState_Ensure();

	CStringA srcA(src);

	void* pyObject = in->mPy_CompileString(srcA, "", Py_file_input);
	if (pyObject) {
		in->mPy_DecRef(pyObject);
	}

	in->mPyGILState_Release(gstate);

	return pyObject != nullptr;
}

