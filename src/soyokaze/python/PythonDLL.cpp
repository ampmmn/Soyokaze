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
typedef void* (*PY_NEWINTERPRETER)(void);
typedef void (*PYEVAL_INITTHREADS)(void);
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
typedef int (*PYDICT_MERGE)(void*, void*, int);
typedef void* (*PYEVAL_GetGlobals)(void);

struct PythonDLL::PImpl
{
	bool Initialize();
	void Finalize();

	CString mDllPath;

	HMODULE mDll{nullptr};
	void* mModule{nullptr};
	void* mDict{nullptr};

	PY_INITIALIZE mPy_Initialize{nullptr};
	PY_FINALIZEEX mPy_FinalizeEx{nullptr};
	PY_NEWINTERPRETER mPy_NewInterpreter{nullptr};
	PYEVAL_INITTHREADS mPyEval_InitThreads{nullptr};
	PYGILSTATE_ENSURE mPyGILState_Ensure{nullptr};
	PYRUN_STRING mPyRun_String{nullptr};
	PYSTRING_ASSTRING mPyString_AsString{nullptr};
	PYERR_OCCURRED mPyErr_Occurred{nullptr};
	PY_DECREF mPy_DecRef{nullptr};
	PYGILSTATE_RELEASE mPyGILState_Release{nullptr};
	PYIMPORT_ADDMODULE mPyImport_AddModule{nullptr};
	PYMODULE_GETDICT mPyModule_GetDict{nullptr};
	PYERR_PRINT mPyErr_Print{nullptr};
	PYOBJECT_REPR mPyObject_Repr{nullptr};
	PYUNICODE_ASENCODEDSTRING mPyUnicode_AsEncodedString{nullptr};
	PYIMPORT_IMPORTMODULE mPyImport_ImportModule{nullptr};
	PYMAPPING_SETITEMSTRING mPyMapping_SetItemString{nullptr};
	PY_COMPILESTRING mPy_CompileString{nullptr};
	PYDICT_MERGE mPyDict_Merge{nullptr};
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
	mPy_NewInterpreter = (PY_NEWINTERPRETER)GetProcAddress(mDll, "Py_NewInterpreter");
	mPyEval_InitThreads = (PYEVAL_INITTHREADS)GetProcAddress(mDll, "PyEval_InitThreads");
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

	// 初期化
	mPy_Initialize();
	mPyEval_InitThreads();

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
	return in->Initialize();
}

bool PythonDLL::Evaluate(const CString& src, CString& result)
{
	int gstate = EnsureGILState();

	CStringA srcA(src);
	void* pyObject = RunString((LPCSTR)srcA);

	// 文をインタープリタ側で評価した結果エラーだった場合
	if (IsErrorOccurred()) {

		// for debug
		PrintError();

		DecRef(pyObject);

		ReleaseGILState(gstate);
		return false;
	}

	if (pyObject == nullptr) {
		ReleaseGILState(gstate);
		return false;
	}

	void* repr = ReprObject(pyObject);
	void* str = AsEncodedString(repr, "utf-8", "~E~");
	const char* s = AsString(str);
	result = CStringA(s);

	DecRef(repr);
	DecRef(str);
	DecRef(pyObject);
	ReleaseGILState(gstate);

	return true;
}


bool PythonDLL::IsErrorOccurred()
{
	return in->mPyErr_Occurred() != nullptr;
}

void PythonDLL::PrintError()
{
	in->mPyErr_Print();
}

void PythonDLL::DecRef(void* obj)
{
	if (obj) {
		in->mPy_DecRef(obj);
	}
}

int PythonDLL::EnsureGILState()
{
	return in->mPyGILState_Ensure();
}

void PythonDLL::ReleaseGILState(int gstate)
{
	in->mPyGILState_Release(gstate);
}

void* PythonDLL::RunString(LPCSTR script)
{
	return in->mPyRun_String(script, Py_eval_input, in->mDict, in->mDict);
}

void* PythonDLL::ReprObject(void* obj)
{
	return in->mPyObject_Repr(obj);
}

void* PythonDLL::AsEncodedString(void* obj, const char* encoding, const char* errors)
{
	return in->mPyUnicode_AsEncodedString(obj, encoding, errors);
}

const char* PythonDLL::AsString(void* obj)
{
	return in->mPyString_AsString(obj);
}


