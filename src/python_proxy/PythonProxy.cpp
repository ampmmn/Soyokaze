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

	PyObject* mModule{nullptr};
	PyObject* mDict{nullptr};

	PyThreadState* mBaseThreadState{nullptr};
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
	mDict = PyModule_GetDict(mModule);

	PyObject* pyMathModule = PyImport_ImportModule("math");
	PyObject* matchDict = PyModule_GetDict(pyMathModule);
	PyDict_Merge(mDict, matchDict, 1);

	Py_DecRef(pyMathModule);

	// サブインタープリターを作成する
	mBaseThreadState = Py_NewInterpreter();

	// GILを解放し、threadstateをnullにしておく
	PyEval_SaveThread();

	return true;
}

void PythonProxy::PImpl::Finalize()
{
	// GILを取得する
	PyEval_RestoreThread(mBaseThreadState);
	mBaseThreadState = nullptr;
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

bool PythonProxy::Evaluate(const wchar_t* src, char** result)
{
	// GILを取得する
	PyEval_RestoreThread(in->mBaseThreadState);

	CStringA srcA(src);
	PyObject* codeObj = Py_CompileString((LPCSTR)srcA, "<string>", Py_eval_input);
	if (codeObj == nullptr) {
		PyErr_Clear();
		PyEval_SaveThread();
		return false;
	}

	PyObject* pyObject = PyEval_EvalCode(codeObj, in->mDict, in->mDict);
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

