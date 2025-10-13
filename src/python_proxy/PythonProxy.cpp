#include "pch.h"
#include "PythonProxy.h"
#include <atlstr.h>

#ifdef _DEBUG
#undef _DEBUG
#define Py_LIMITED_API
#include <Python.h>
#undef _DEBUG
#else
#define Py_LIMITED_API
#include <Python.h>
#endif

using CStringA = ATL::CStringA;
using CStringW = ATL::CStringW;
using CString = ATL::CString;

static void DecRef(PyObject* obj);

struct PythonProxy::PImpl
{
	bool Initialize();
	void Finalize();

	bool InitializeForCalc();
	bool InitializeForPyCmd();

	PyObject* mModule{nullptr};
	PyObject* mGlobalDictForCalc{nullptr};
	PyObject* mGlobalDictForPyCmd{nullptr};

	PyThreadState* mThreadStateForCalc{nullptr};
	PyThreadState* mThreadStateForPyCmd{nullptr};
	void* mInterpreter{nullptr};
};

bool PythonProxy::PImpl::Initialize()
{
	// 初期化
	Py_Initialize();
#if PY_VERSION_HEX < 0x030A0000
	PyEval_InitThreads();
#endif

	// 辞書生成
	mModule = PyImport_AddModule("__main__");
	mGlobalDictForCalc = PyModule_GetDict(mModule);
	mGlobalDictForPyCmd = PyDict_Copy(mGlobalDictForCalc);

	InitializeForCalc();
	InitializeForPyCmd();

	// サブインタープリターを作成する
	mThreadStateForCalc = Py_NewInterpreter();
	mThreadStateForPyCmd = Py_NewInterpreter();

	// GILを解放し、threadstateをnullにしておく
	PyEval_SaveThread();

	return true;
}

// 電卓機能向けの初期化
bool PythonProxy::PImpl::InitializeForCalc()
{
	PyObject* pyMathModule = PyImport_ImportModule("math");
	PyObject* matchDict = PyModule_GetDict(pyMathModule);
	PyDict_Merge(mGlobalDictForCalc, matchDict, 1);

	Py_DecRef(pyMathModule);
	return true;
}

// Python拡張コマンド向けの初期化
bool PythonProxy::PImpl::InitializeForPyCmd()
{
	PyObject* key_module = PyImport_ImportModule("key");
	if (key_module == nullptr) {
		//spdlog::error("pycmd : Failed to load key module.");
		return false;
	}

	PyDict_SetItemString(mGlobalDictForPyCmd, "__builtins__", PyEval_GetBuiltins());
	PyDict_SetItemString(mGlobalDictForPyCmd, "key", key_module);

	return true;
}

void PythonProxy::PImpl::Finalize()
{
	// GILを取得する
	PyEval_RestoreThread(mThreadStateForCalc);
	mThreadStateForCalc = nullptr;
	PyEval_RestoreThread(mThreadStateForPyCmd);
	mThreadStateForPyCmd = nullptr;
	// Initializeの取り消し(Initializeで作成したサブインタープリタも破棄する) 
	Py_Finalize();
}


PythonProxy::PythonProxy() : in(new PImpl)
{
	in->Initialize();
}

PythonProxy::~PythonProxy()
{
	in->Finalize();
}

bool PythonProxy::CompileTest(const char* src, char** errMsg)
{
	// GILを取得する
	PyEval_RestoreThread(in->mThreadStateForPyCmd);

	PyObject* codeObj = Py_CompileString((LPCSTR)src, "<string>", Py_file_input);
	if (codeObj == nullptr) {
		// FIXME: エラー詳細をerrMsgにつめて返す
		PyErr_Clear();
		PyEval_SaveThread();
		return false;
	}

	Py_DecRef(codeObj);
	PyEval_SaveThread();

	return true;

}

bool PythonProxy::Evaluate(const char* src, char** errMsg)
{
	// GILを取得する
	PyEval_RestoreThread(in->mThreadStateForPyCmd);

	PyObject* codeObj = Py_CompileString((LPCSTR)src, "<string>", Py_file_input);
	if (codeObj == nullptr) {
		// FIXME: エラー詳細をerrMsgにつめて返す
		PyErr_Clear();
		PyEval_SaveThread();
		return false;
	}
	PyObject* localDict = PyDict_New();

	PyObject* pyObject = PyEval_EvalCode(codeObj, in->mGlobalDictForPyCmd, localDict);

	Py_DecRef(localDict);
	Py_DecRef(codeObj);

	if (pyObject) {
		Py_DecRef(pyObject);
	}

	PyEval_SaveThread();

	return true;
}

bool PythonProxy::EvalForCalculate(const char* src, char** result)
{
	// GILを取得する
	PyEval_RestoreThread(in->mThreadStateForCalc);

	PyObject* codeObj = Py_CompileString((LPCSTR)src, "<string>", Py_eval_input);
	if (codeObj == nullptr) {
		PyErr_Clear();
		PyEval_SaveThread();
		return false;
	}

	PyObject* localDict = PyDict_New();

	PyObject* pyObject = PyEval_EvalCode(codeObj, in->mGlobalDictForCalc, localDict);

	Py_DecRef(localDict);
	Py_DecRef(codeObj);

	// 文をインタープリタ側で評価した結果エラーだった場合
	if (PyErr_Occurred() != nullptr) {
		// for debug
		PyErr_Print();

		DecRef(pyObject);

		PyEval_SaveThread();
		return false;
	}

	if (pyObject == nullptr) {
		PyEval_SaveThread();
		return false;
	}

	PyObject* repr = PyObject_Repr(pyObject);
	PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
	const char* s = PyBytes_AsString(str);

	size_t len = strlen(s) + 1;
	*result = new char[len];
	memcpy(*result, s, len);

	DecRef(repr);
	DecRef(str);
	DecRef(pyObject);

	// GILを解放する
	PyEval_SaveThread();

	return true;
}

void PythonProxy::ReleaseBuffer(char* result)
{
	delete [] result;
}

void DecRef(PyObject* obj)
{
	if (obj) {
		Py_DecRef(obj);
	}
}

