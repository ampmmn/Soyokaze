#include "pch.h"
#include "PythonProxy.h"
#include "ProxyWindow.h"
#include <atlstr.h>
#include <string>
#include <regex>
#include <atomic>
#include <thread>

#ifdef _DEBUG
#undef _DEBUG
#define Py_LIMITED_API 0x030c0000
#include <Python.h>
#undef _DEBUG
#else
#define Py_LIMITED_API 0x030c0000
#include <Python.h>
#endif

// Python拡張コマンドでの実行時間のタイムアウト
constexpr uint64_t TIMEOUT_IN_MSEC = 10 * 1000; // 10秒

using CStringA = ATL::CStringA;
using CStringW = ATL::CStringW;
using CString = ATL::CString;

struct scope_pyobject
{
	scope_pyobject() {}
	scope_pyobject(PyObject* obj) : mObj(obj) {}
	~scope_pyobject() {
		if (mObj) {
			Py_DecRef(mObj);
			mObj = nullptr;
		}
	}
	bool operator == (void* p) const { return mObj == p; }
	operator PyObject*() { return mObj; }
	PyObject** operator &() { return &mObj; }

	PyObject* mObj{nullptr};
};

struct PythonProxy::PImpl
{
	struct scope_state
	{
		scope_state(PImpl* in_, PyThreadState* state_) : in(in_), state(state_) {
			in->SetBusy(true);
			// GILを取得する
			PyEval_RestoreThread(state);
	 	}
		~scope_state() {
			// GILを解放する
			PyEval_SaveThread();
			in->SetBusy(false);
		}
		PImpl* in;
		PyThreadState* state;
	};

	bool Initialize();
	void Finalize();

	bool InitializeForPyCmd(PyObject* globalDict);

	bool IsBusy() {
		return mIsBusy.load();
	}
	void SetBusy(bool busy) {
		mIsBusy = busy;
	}

	PyObject* mModule{nullptr};
	PyObject* mGlobalDictForCalc{nullptr};

	PyThreadState* mThreadStateForPyCmd{nullptr};
	PyThreadState* mThreadStateForCalc{nullptr};
	void* mInterpreter{nullptr};
	std::atomic<bool> mIsBusy{false};
};

using scope_state = PythonProxy::PImpl::scope_state;

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

	mThreadStateForPyCmd = PyThreadState_Get();

	// サブインタープリターを作成する
	mThreadStateForCalc = Py_NewInterpreter();

	// GILを解放し、threadstateをnullにしておく
	PyEval_SaveThread();

	return true;
}

// Python拡張コマンド向けの初期化
bool PythonProxy::PImpl::InitializeForPyCmd(PyObject* globalDict)
{
	// app.pyd, key.pyd, win.pydを読み込むためのディレクトリパス生成
	std::vector<wchar_t> exeDir(MAX_PATH_NTFS);
	GetModuleFileName(nullptr, exeDir.data(), MAX_PATH_NTFS);
	PathRemoveFileSpec(exeDir.data());
	//PathAppend(exeDir.data(), _T("pylib"));

	// ディレクトリパスをモジュール検索対象として追加
	scope_pyobject libPath = PyUnicode_FromWideChar(exeDir.data(), -1);
	PyObject* sysPath = PySys_GetObject("path");
	PyList_Append(sysPath, libPath);

	auto builtins = PyEval_GetBuiltins();
	PyDict_SetItemString(globalDict, "__builtins__", builtins);

	// アプリ専用の組み込みライブラリを暗黙的にインポート
	scope_pyobject app_module = PyImport_ImportModule("app");
	if (app_module != nullptr) {
		PyDict_SetItemString(globalDict, "app", app_module);
	}
	scope_pyobject key_module = PyImport_ImportModule("key");
	if (key_module != nullptr) {
		PyDict_SetItemString(globalDict, "key", key_module);
	}
	scope_pyobject win_module = PyImport_ImportModule("win");
	if (win_module != nullptr) {
		PyDict_SetItemString(globalDict, "win", win_module);
	}

	return app_module != nullptr && key_module != nullptr && win_module != nullptr;
}

void PythonProxy::PImpl::Finalize()
{
	if (mThreadStateForCalc == nullptr) {
		return;
	}

	PyEval_RestoreThread(mThreadStateForCalc);
	// Initializeの取り消し(Initializeで作成したサブインタープリタも破棄する)
	Py_Finalize();

	mThreadStateForPyCmd = nullptr;
	mThreadStateForCalc = nullptr;
	mGlobalDictForCalc = nullptr;
	mModule = nullptr;
}


PythonProxy::PythonProxy() : in(new PImpl)
{
	ProxyWindow::GetInstance()->RequestCallback([&]() {
		return in->Initialize();
	});
}

PythonProxy::~PythonProxy()
{
	Finalize();
}

void PythonProxy::Finalize()
{
	ProxyWindow::GetInstance()->RequestCallback([&]() {
		in->Finalize();
		return true;
	});
}

static std::string py2str(PyObject* obj)
{
	scope_pyobject repr = PyObject_Repr(obj);
	scope_pyobject str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
	return std::string(PyBytes_AsString(str));
}

static void MakeErrMsg(char** errMsg, const std::string& msgstr)
{
	if (errMsg == nullptr) {
		return;
	}
	*errMsg = new char[msgstr.size()+1];
	memcpy(*errMsg, msgstr.data(), msgstr.size() + 1);
}


static void FetchErrMsg(char** errMsg)
{
	if (errMsg == nullptr) {
		return;
	}

	scope_pyobject ptype;
	scope_pyobject pvalue;
	scope_pyobject ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);

	std::string exc_type(py2str(ptype));
	std::string exc_value(py2str(pvalue));

	std::string buf(exc_type + "\r\n" + exc_value);
	MakeErrMsg(errMsg, buf);
}

bool PythonProxy::CompileTest(const char* src, char** errMsg)
{
	return ProxyWindow::GetInstance()->RequestCallback([&]() {

		scope_state _scope_state(in.get(), in->mThreadStateForPyCmd);

		scope_pyobject codeObj = Py_CompileString((LPCSTR)src, "<string>", Py_file_input);
		if (codeObj == nullptr) {
			// エラー詳細をerrMsgにつめて返す
			FetchErrMsg(errMsg);
			PyErr_Clear();
			return false;
		}
		return true;
	});
}

bool PythonProxy::Evaluate(const char* src, char** errMsg)
{
	return ProxyWindow::GetInstance()->RequestCallback([&]() {

		// 実行中である状態にセットする
		// (電卓機能の方から利用できなくするため)
		scope_state _scope_state(in.get(), in->mThreadStateForPyCmd);

		// スクリプトをコンパイル
		scope_pyobject codeMain = Py_CompileString((LPCSTR)src, "<string>", Py_file_input);
		if (codeMain == nullptr) {
			// コンパイルに失敗したら、エラー詳細をerrMsgにつめて返す
			FetchErrMsg(errMsg);
			PyErr_Clear();
			return false;
		}

		auto window = ProxyWindow::GetInstance();

		scope_pyobject globalDict = PyDict_New();
		in->InitializeForPyCmd(globalDict);

		// タイムアウト監視スレッド側でスクリプト実行完了を検知するためのイベント
		HANDLE eval_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

		bool isTimeout = false;
		std::thread th([&isTimeout, window, eval_event]() {

			// 一定時間待ってもスクリプト評価が完了しない場合は強制終了させる
			auto s = GetTickCount64();
			while(GetTickCount64() - s < TIMEOUT_IN_MSEC && window->IsAbort() == false) {

				Sleep(50);

				if (WaitForSingleObject(eval_event, 0) == WAIT_OBJECT_0) {
					// スクリプト側の評価が終わったので監視をやめる
					return;
				}
			}

			// 中止するために例外を発行する
			PyErr_SetInterrupt();

			if (window->IsAbort() == false) {
				isTimeout = true;
			}
		});

		// スクリプトを実行
		scope_pyobject localDict = PyDict_New();
		scope_pyobject pyRetObject = PyEval_EvalCode(codeMain, globalDict, localDict);
		SetEvent(eval_event);

		// タイムアウト監視スレッドの完了を待つ
		th.join();
		CloseHandle(eval_event);

		bool isOK = PyErr_Occurred() == nullptr;
		if (isOK == false) {

			// IsAbort() == アプリ終了時はエラー状態を返さないようにする
			// (proxyインスタンスが破棄されてReleaseBufferをよべない可能性があるため)
			if (window->IsAbort() == false) {
				if (isTimeout == false) {
					// エラー詳細をerrMsgにつめて返す
					FetchErrMsg(errMsg);
				}
				else {
					// 時間がかかりすぎ
					MakeErrMsg(errMsg, u8"時間がかかるためスクリプトの実行を停止しました");
				}
			}
			PyErr_Clear();
		}
		return isOK;
	});
}

bool PythonProxy::EvalForCalculate(const char* src, char** result)
{
	if (in->IsBusy()) {
		return false;
	}

	return ProxyWindow::GetInstance()->RequestCallback([&]() {

		if (in->IsBusy()) {
			// Py拡張コマンドの方でPython実行中
			// (ブロックするのを避けるため即時リターンする)
			return false;
		}

		scope_state _scope_state(in.get(), in->mThreadStateForCalc);

		scope_pyobject codeObj = Py_CompileString((LPCSTR)src, "<string>", Py_eval_input);
		if (codeObj == nullptr) {
			PyErr_Clear();
			return false;
		}

		scope_pyobject localDict = PyDict_New();
		scope_pyobject pyObject = PyEval_EvalCode(codeObj, in->mGlobalDictForCalc, localDict);

		// 文をインタープリタ側で評価した結果エラーだった場合
		if (PyErr_Occurred() != nullptr) {
			PyErr_Clear();
			return false;
		}

		if (pyObject == nullptr) {
			return false;
		}

		std::string s(py2str(pyObject));

		size_t len = s.size() + 1;
		*result = new char[len];
		memcpy(*result, s.c_str(), len);

		return true;
	});
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
	if (major != 3 || minor < 12) {
		return false;
	}
	return true;
}

bool PythonProxy::IsBusy()
{
	return in->IsBusy();
}

