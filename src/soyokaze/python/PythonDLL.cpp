#include "pch.h"
#include "PythonDLL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//static const int Py_single_input = 256;    // defined in compile.h
static const int Py_file_input = 257;      // defined in compile.h
static const int Py_eval_input = 258;    // defined in compile.h

using PYBYTES_ASSTRING = char*(*)(void*);
using PYDICT_MERGE = int (*)(void*, void*, int);
using PYERR_OCCURRED = void*(*)(void);
using PYERR_PRINT = void (*)(void);
using PYEVAL_INITTHREADS = void(*)(void);
using PYEVAL_RESTORETHREAD = void(*)(void*);
using PYEVAL_SAVETHREAD = void*(*)(void);
using PYIMPORT_ADDMODULE = void* (*)(const char*);
using PYIMPORT_IMPORTMODULE = void* (*)(const char*);
using PYMAPPING_SETITEMSTRING = int (*)(void*, const char*, void*);
using PYMODULE_GETDICT = void* (*)(void*);
using PYOBJECT_REPR = void* (*)(void*);
using PY_COMPILESTRING = void*(*)(const char *, const char *, int);
using PYEVAL_EVALCODE = void*(*)(void*, void*,void*);
using PYUNICODE_ASENCODEDSTRING = void* (*)(void*, const char*, const char*);
using PY_DECREF = void(*)(void*);
using PY_FINALIZEEX = int(*)(void);
using PY_INITIALIZE = void(*)(void);
using PY_NEWINTERPRETER = void*(*)(void); 

struct PythonDLL::PImpl
{
	bool Initialize();
	void Finalize();

	CString mDllPath;

	HMODULE mDll{nullptr};
	void* mModule{nullptr};
	void* mDict{nullptr};

	void* mBaseThreadState{nullptr};
	void* mInterpreter{nullptr};

	PYBYTES_ASSTRING mPyBytes_AsString{nullptr};
	PYDICT_MERGE mPyDict_Merge{nullptr};
	PYERR_OCCURRED mPyErr_Occurred{nullptr};
	PYERR_PRINT mPyErr_Print{nullptr};
	PYEVAL_INITTHREADS mPyEval_InitThreads{nullptr};
	PYEVAL_RESTORETHREAD mPyEval_RestoreThread{nullptr};
	PYEVAL_SAVETHREAD mPyEval_SaveThread{nullptr};
	PYIMPORT_ADDMODULE mPyImport_AddModule{nullptr};
	PYIMPORT_IMPORTMODULE mPyImport_ImportModule{nullptr};
	PYMAPPING_SETITEMSTRING mPyMapping_SetItemString{nullptr};
	PYMODULE_GETDICT mPyModule_GetDict{nullptr};
	PYOBJECT_REPR mPyObject_Repr{nullptr};
	PY_COMPILESTRING mPy_CompileString{nullptr};
	PYEVAL_EVALCODE mPyEval_EvalCode{nullptr};
	PYUNICODE_ASENCODEDSTRING mPyUnicode_AsEncodedString{nullptr};
	PY_DECREF mPy_DecRef{nullptr};
	PY_FINALIZEEX mPy_FinalizeEx{nullptr};
	PY_INITIALIZE mPy_Initialize{nullptr};
	PY_NEWINTERPRETER mPy_NewInterpreter{nullptr};
};

#define TRY_GET_PROC(dll_handle_ref, type_name, function_name) \
	m##function_name = (type_name)GetProcAddress(dll_handle_ref, #function_name); \
	if (m##function_name == nullptr) { \
		FreeLibrary(dll_handle_ref); dll_handle_ref = nullptr; \
		m##function_name = nullptr; \
		spdlog::error("Failed to get proc {}", #function_name); \
		return false; \
	}

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
		// ロード済
		return true;
	}

	// 使用するAPIを取得
	TRY_GET_PROC(mDll, PYBYTES_ASSTRING, PyBytes_AsString);
	TRY_GET_PROC(mDll, PYDICT_MERGE, PyDict_Merge);
	TRY_GET_PROC(mDll, PYERR_OCCURRED, PyErr_Occurred);
	TRY_GET_PROC(mDll, PYERR_PRINT, PyErr_Print);
	TRY_GET_PROC(mDll, PYEVAL_INITTHREADS, PyEval_InitThreads);
	TRY_GET_PROC(mDll, PYEVAL_RESTORETHREAD, PyEval_RestoreThread);
	TRY_GET_PROC(mDll, PYEVAL_SAVETHREAD, PyEval_SaveThread);
	TRY_GET_PROC(mDll, PYIMPORT_ADDMODULE, PyImport_AddModule);
	TRY_GET_PROC(mDll, PYIMPORT_IMPORTMODULE, PyImport_ImportModule);
	TRY_GET_PROC(mDll, PYMAPPING_SETITEMSTRING, PyMapping_SetItemString);
	TRY_GET_PROC(mDll, PYMODULE_GETDICT, PyModule_GetDict);
	TRY_GET_PROC(mDll, PYOBJECT_REPR, PyObject_Repr);
	TRY_GET_PROC(mDll, PY_COMPILESTRING, Py_CompileString);
	TRY_GET_PROC(mDll, PYEVAL_EVALCODE, PyEval_EvalCode);
	TRY_GET_PROC(mDll, PYUNICODE_ASENCODEDSTRING, PyUnicode_AsEncodedString);
	TRY_GET_PROC(mDll, PY_DECREF, Py_DecRef);
	TRY_GET_PROC(mDll, PY_FINALIZEEX, Py_FinalizeEx);
	TRY_GET_PROC(mDll, PY_INITIALIZE, Py_Initialize);
	TRY_GET_PROC(mDll, PY_NEWINTERPRETER, Py_NewInterpreter);

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

	// サブインタープリターを作成する
	mBaseThreadState = mPy_NewInterpreter();

	// GILを解放し、threadstateをnullにしておく
	mPyEval_SaveThread();

	return true;
}

void PythonDLL::PImpl::Finalize()
{
	if (mDll == nullptr) {
		return;
	}

	// GILを取得する
	mPyEval_RestoreThread(mBaseThreadState);
	// Initializeの取り消し(Initializeで作成したサブインタープリタも破棄する) 
	mPy_FinalizeEx();

	// ライブラリをアンロード
	FreeLibrary(mDll);
	mDll = nullptr;

	mPyBytes_AsString = nullptr;
	mPyDict_Merge = nullptr;
	mPyErr_Occurred = nullptr;
	mPyErr_Print = nullptr;
	mPyEval_InitThreads = nullptr;
	mPyEval_RestoreThread = nullptr;
	mPyEval_SaveThread = nullptr;
	mPyImport_AddModule = nullptr;
	mPyImport_ImportModule = nullptr;
	mPyMapping_SetItemString = nullptr;
	mPyModule_GetDict = nullptr;
	mPyObject_Repr = nullptr;
	mPy_CompileString = nullptr;
	mPyEval_EvalCode = nullptr;
	mPyUnicode_AsEncodedString = nullptr;
	mPy_DecRef = nullptr;
	mPy_FinalizeEx = nullptr;
	mPy_Initialize = nullptr;
	mPy_NewInterpreter = nullptr;
}


PythonDLL::PythonDLL() : in(new PImpl)
{
}

PythonDLL::~PythonDLL()
{
	in->Finalize();
}

bool PythonDLL::LoadDLL(const CString& dllPath)
{
	in->mDllPath = dllPath;
	return in->Initialize();
}

bool PythonDLL::Evaluate(const CString& src, CString& result)
{
	if (in->mDll == nullptr) {
		return false;
	}
	// GILを取得する
	in->mPyEval_RestoreThread(in->mBaseThreadState);

	CStringA srcA(src);
	void* pyObject = RunString((LPCSTR)srcA);

	// 文をインタープリタ側で評価した結果エラーだった場合
	if (IsErrorOccurred()) {
		// for debug
		PrintError();

		DecRef(pyObject);

		in->mPyEval_SaveThread();
		return false;
	}

	if (pyObject == nullptr) {
		in->mPyEval_SaveThread();
		return false;
	}

	void* repr = ReprObject(pyObject);
	void* str = AsEncodedString(repr, "utf-8", "~E~");
	const char* s = AsString(str);
	result = CStringA(s);

	DecRef(repr);
	DecRef(str);
	DecRef(pyObject);

	// GILを解放する
	in->mPyEval_SaveThread();

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

void* PythonDLL::RunString(LPCSTR script)
{
	void* codeObj = in->mPy_CompileString(script, "<string>", Py_eval_input);
	if (codeObj == nullptr) {
		return nullptr;
	}

	void* ret = in->mPyEval_EvalCode(codeObj, in->mDict, in->mDict);
	in->mPy_DecRef(codeObj);

	return ret;
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
	return in->mPyBytes_AsString(obj);
}


