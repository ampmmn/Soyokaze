#include "pch.h"
#include "PythonProxy.h"
#include <atlstr.h>
#include <string>
#include <regex>
#include <mutex>

#ifdef _DEBUG
#undef _DEBUG
#define Py_LIMITED_API 0x030c0000
#include <Python.h>
#undef _DEBUG
#else
#define Py_LIMITED_API 0x030c0000
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

	bool InitializeForPyCmd(PyObject* globalDict);

	bool IsBusy() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsBusy;
	}
	void SetBusy(bool busy) {
		std::lock_guard<std::mutex> lock(mMutex);
		mIsBusy = busy;
	}

	PyObject* mModule{nullptr};
	PyObject* mGlobalDictForCalc{nullptr};

	PyThreadState* mThreadStateForCalc{nullptr};
	void* mInterpreter{nullptr};
	std::mutex mMutex;
	bool mIsBusy{false};
};

bool PythonProxy::PImpl::Initialize()
{
	// 初期化
	Py_Initialize();

	// 辞書生成
	mModule = PyImport_AddModule("__main__");
	mGlobalDictForCalc = PyModule_GetDict(mModule);

// 電卓機能向けの初期化
	PyObject* pyMathModule = PyImport_ImportModule("math");
	PyObject* matchDict = PyModule_GetDict(pyMathModule);
	PyDict_Merge(mGlobalDictForCalc, matchDict, 1);

	Py_DecRef(pyMathModule);

	// サブインタープリターを作成する
	mThreadStateForCalc = Py_NewInterpreter();

	// GILを解放し、threadstateをnullにしておく
	PyEval_SaveThread();

	return true;
}

// Python拡張コマンド向けの初期化
bool PythonProxy::PImpl::InitializeForPyCmd(PyObject* globalDict)
{
	// exeがあるディレクトリを検索パスとして追加
	std::vector<wchar_t> exeDir(MAX_PATH_NTFS);
	GetModuleFileName(nullptr, exeDir.data(), MAX_PATH_NTFS);
	PathRemoveFileSpec(exeDir.data());
	PyObject* libPath = PyUnicode_FromWideChar(exeDir.data(), -1);
	PyObject* sysPath = PySys_GetObject("path");
	PyList_Append(sysPath, libPath);
	DecRef(libPath);
	auto len = PyList_Size(sysPath);

	auto builtins = PyEval_GetBuiltins();
	PyDict_SetItemString(globalDict, "__builtins__", builtins);

	// アプリ専用の組み込みライブラリを暗黙的にインポート
	PyObject* key_module = PyImport_ImportModule("key");
	if (key_module != nullptr) {
		PyDict_SetItemString(globalDict, "key", key_module);
		DecRef(key_module);
	}
	PyObject* win_module = PyImport_ImportModule("win");
	if (win_module != nullptr) {
		PyDict_SetItemString(globalDict, "win", win_module);
		DecRef(win_module);
	}

	return key_module != nullptr && win_module != nullptr;
}

void PythonProxy::PImpl::Finalize()
{
	if (mThreadStateForCalc == nullptr) {
		return;
	}
	PyEval_RestoreThread(mThreadStateForCalc);
	// Initializeの取り消し(Initializeで作成したサブインタープリタも破棄する) 
	Py_Finalize();

	mThreadStateForCalc = nullptr;
	mGlobalDictForCalc = nullptr;
	mModule = nullptr;
}


PythonProxy::PythonProxy() : in(new PImpl)
{
	in->Initialize();
}

PythonProxy::~PythonProxy()
{
	in->Finalize();
}

void PythonProxy::Finalize()
{
	in->Finalize();
}

static std::string py2str(PyObject* obj)
{
	PyObject* repr = PyObject_Repr(obj);
	PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");

	std::string ret(PyBytes_AsString(str));

	DecRef(str);
	DecRef(repr);

	return ret;
}

static void FetchErrMsg(char** errMsg)
{
	if (errMsg == nullptr) {
		return;
	}

	PyObject* ptype;
	PyObject* pvalue;
	PyObject* ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);

	std::string exc_type(py2str(ptype));
	std::string exc_value(py2str(pvalue));

	std::string buf(exc_type + "\r\n" + exc_value);
	*errMsg = new char[buf.size()+1];
	memcpy(*errMsg, buf.data(), buf.size() + 1);

	Py_DecRef(ptraceback);
	Py_DecRef(pvalue);
	Py_DecRef(ptype);

}


bool PythonProxy::CompileTest(const char* src, char** errMsg)
{
	if (in->IsBusy()) {
		return false;
	}
	in->SetBusy(true);

	PyThreadState* state = Py_NewInterpreter();

	PyObject* codeObj = Py_CompileString((LPCSTR)src, "<string>", Py_file_input);
	if (codeObj == nullptr) {
		// エラー詳細をerrMsgにつめて返す
		FetchErrMsg(errMsg);
		PyErr_Clear();
	}

	DecRef(codeObj);
	Py_EndInterpreter(state);

	in->SetBusy(false);
	return true;

}

bool PythonProxy::Evaluate(const char* src, char** errMsg)
{
	if (in->IsBusy()) {
		return false;
	}
	in->SetBusy(true);

	PyThreadState* state = Py_NewInterpreter();

	// GILを取得する
	PyObject* codeObj = Py_CompileString((LPCSTR)src, "<string>", Py_file_input);
	if (codeObj == nullptr) {
		// エラー詳細をerrMsgにつめて返す
		FetchErrMsg(errMsg);

		PyErr_Clear();

		Py_EndInterpreter(state);

		in->SetBusy(false);
		return false;
	}



	PyObject* globalDict = PyDict_New();
	in->InitializeForPyCmd(globalDict);

	PyObject* localDict = PyDict_New();

	PyObject* pyObject = PyEval_EvalCode(codeObj, globalDict, localDict);

	DecRef(codeObj);
	DecRef(localDict);
	DecRef(globalDict);
	DecRef(pyObject);

	bool isOK = PyErr_Occurred() == nullptr;
	if (isOK == false) {
		// エラー詳細をerrMsgにつめて返す
		FetchErrMsg(errMsg);
		PyErr_Clear();
	}


	Py_EndInterpreter(state);

	in->SetBusy(false);
	return isOK;
}

bool PythonProxy::EvalForCalculate(const char* src, char** result)
{
	if (in->IsBusy()) {
		return false;
	}
	in->SetBusy(true);

	// GILを取得する
	PyEval_RestoreThread(in->mThreadStateForCalc);

	PyObject* codeObj = Py_CompileString((LPCSTR)src, "<string>", Py_eval_input);
	if (codeObj == nullptr) {
		PyErr_Clear();
		PyEval_SaveThread();
		in->SetBusy(false);
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

		PyErr_Clear();
		PyEval_SaveThread();
		in->SetBusy(false);
		return false;
	}

	if (pyObject == nullptr) {
		PyEval_SaveThread();
		in->SetBusy(false);
		return false;
	}

	std::string s(py2str(pyObject));

	size_t len = s.size() + 1;
	*result = new char[len];
	memcpy(*result, s.c_str(), len);

	DecRef(pyObject);

	// GILを解放する
	PyEval_SaveThread();

	in->SetBusy(false);
	return true;
}

void PythonProxy::ReleaseBuffer(char* result)
{
	delete [] result;
}

bool PythonProxy::IsPyCmdAvailable()
{
	const char* ver = Py_GetVersion();
	static const std::regex pat("^(\\d+)\\.(\\d+).+$");
	if (std::regex_match(ver, pat) == false) {
		return false;
	}
	auto major = std::stoi(std::regex_replace(ver, pat, "$1"));
	auto minor = std::stoi(std::regex_replace(ver, pat, "$2"));
	if (major < 3 || minor < 12) {
		return false;
	}
	return true;
}

bool PythonProxy::IsBusy()
{
	return in->IsBusy();
}

void DecRef(PyObject* obj)
{
	if (obj) {
		Py_DecRef(obj);
	}
}

