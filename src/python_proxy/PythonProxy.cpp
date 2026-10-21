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

// 電卓機能での評価のタイムアウト
constexpr uint64_t CALC_TIMEOUT_IN_MSEC = 3 * 1000; // 3秒

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

	PyObject* release() {
		auto p = mObj;
		mObj = nullptr;
		return p;
	}

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

	PyObject* mGlobalDictForCalc{nullptr};

	PyThreadState* mThreadStateForPyCmd{nullptr};
	void* mInterpreter{nullptr};
	std::atomic<bool> mIsBusy{false};
};

using scope_state = PythonProxy::PImpl::scope_state;

// 電卓機能で使用を許可する組み込み関数の一覧
static const char* const CALC_ALLOWED_BUILTINS[] = {
	"abs", "bin", "float", "hex", "int", "max", "min", "oct", "round"
};

/**
  電卓機能用のグローバル辞書を作成する
  mathモジュールの関数・定数と、電卓機能で使用する組み込み関数のみを登録する
  それ以外の組み込み関数は使用できないようにするため、
  辞書の__(builtins__)は空の辞書に置き換える
  @return 作成した辞書
*/
static PyObject* CreateCalcGlobalDict()
{
	PyObject* globals = PyDict_New();
	if (globals == nullptr) {
		return nullptr;
	}

	// mathモジュールの関数・定数を登録する
	// (__(name__)などのモジュール属性は登録しない)
	scope_pyobject mathModule = PyImport_ImportModule("math");
	if (mathModule != nullptr) {
		PyObject* mathDict = PyModule_GetDict(mathModule);
		PyObject* key = nullptr;
		PyObject* value = nullptr;
		Py_ssize_t pos = 0;
		while (PyDict_Next(mathDict, &pos, &key, &value)) {
			if (PyUnicode_Check(key) == false) {
				continue;
			}
			Py_ssize_t nameLen = 0;
			const char* name = PyUnicode_AsUTF8AndSize(key, &nameLen);
			if (name == nullptr || nameLen == 0 || name[0] == '_') {
				continue;
			}
			PyDict_SetItemString(globals, name, value);
		}
		// math.xxx() の形式でも参照できるようにモジュール自身を登録する
		PyDict_SetItemString(globals, "math", mathModule);
	}

	// 電卓機能で使用する組み込み関数のみを登録する
	PyObject* builtins = PyEval_GetBuiltins();
	if (builtins != nullptr) {
		for (auto name : CALC_ALLOWED_BUILTINS) {
			PyObject* func = PyDict_GetItemString(builtins, name);
			if (func != nullptr) {
				PyDict_SetItemString(globals, name, func);
			}
		}
	}

	// 組み込み関数を呼び出せないようにする
	// (グローバル辞書に__(builtins__)がない場合、インタープリタ側の組み込み関数が使用される)
	scope_pyobject emptyBuiltins = PyDict_New();
	if (emptyBuiltins != nullptr) {
		PyDict_SetItemString(globals, "__builtins__", emptyBuiltins);
	}

	return globals;
}

bool PythonProxy::PImpl::Initialize()
{
	// 初期化
	Py_Initialize();

	// 電卓機能向けのグローバル辞書を作成する
	mGlobalDictForCalc = CreateCalcGlobalDict();

	mThreadStateForPyCmd = PyThreadState_Get();

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
	if (mThreadStateForPyCmd == nullptr) {
		return;
	}

	PyEval_RestoreThread(mThreadStateForPyCmd);

	// 電卓機能用のグローバル辞書を解放する
	Py_XDECREF(mGlobalDictForCalc);
	mGlobalDictForCalc = nullptr;

	// Initializeの取り消し
	Py_Finalize();

	mThreadStateForPyCmd = nullptr;
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

/**
  Pythonオブジェクトの文字列表現を取得する
  文字列表現を取得できない場合(例: 桁数の多い整数)は空文字列を返す
  @return 文字列表現
  @param[in] obj 対象のオブジェクト
*/
static std::string py2str(PyObject* obj)
{
	if (obj == nullptr) {
		return std::string();
	}

	scope_pyobject repr = PyObject_Repr(obj);
	if ((PyObject*)repr == nullptr) {
		// 文字列表現の取得に失敗した場合
		PyErr_Clear();
		return std::string();
	}

	scope_pyobject str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
	if ((PyObject*)str == nullptr) {
		PyErr_Clear();
		return std::string();
	}

	const char* raw = PyBytes_AsString(str);
	if (raw == nullptr) {
		PyErr_Clear();
		return std::string();
	}
	return std::string(raw);
}

static void MakeErrMsg(char** errMsg, const std::string& msgstr)
{
	if (errMsg == nullptr) {
		return;
	}
	*errMsg = new char[msgstr.size()+1];
	memcpy(*errMsg, msgstr.data(), msgstr.size() + 1);
}

static void MakeErrMsg(char** errMsg, const std::u8string& msgstr)
{
	if (errMsg == nullptr) {
		return;
	}
	*errMsg = new char[msgstr.size() + 1];
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

bool PythonProxy::Evaluate(const char* src, const char** args, char** errMsg)
{
	// ウインドウを作成した側のスレッドでラムダ関数を同期実行する
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
			// (カーネルで待機することで、スクリプト終了と同時に監視が解除される)
			auto s = GetTickCount64();
			while (true) {

				// 10ms単位で分割して待機する
				// (アプリの中止要求を定期的に確認するため)
				if (WaitForSingleObject(eval_event, 10) == WAIT_OBJECT_0) {
					// スクリプト側の評価が終わったので監視をやめる
					return;
				}

				if (window->IsAbort()) {
					// 中止するために例外を発行する
					PyErr_SetInterrupt();
					return;
				}

				if (GetTickCount64() - s >= TIMEOUT_IN_MSEC) {
					// 中止するために例外を発行する
					PyErr_SetInterrupt();

					if (window->IsAbort() == false) {
						isTimeout = true;
					}
					return;
				}
			}
		});

		// スクリプトを実行
		scope_pyobject pyRetObject = PyEval_EvalCode(codeMain, globalDict, globalDict);

		// funcObjは借用参照
		PyObject* pyFuncObj = PyDict_GetItemString(globalDict, "call");

		// スクリプトがcallという名前の定義を定義していたらそれを呼ぶ
		if (pyFuncObj && PyCallable_Check(pyFuncObj)) {

			// call関数に対して渡す引数の数をカウントする
			auto a = args;
			size_t nArgs = 0;
			while(a && *a != nullptr) {
				nArgs++;
				a++;
			}

			// call関数に渡すための引数オブジェクトを生成する
			scope_pyobject pyArgsTuple(PyTuple_New(nArgs));
			for (int i = 0; i < nArgs; ++i) {
				PyTuple_SetItem(pyArgsTuple, i, PyUnicode_FromString(args[i]));
			}

			scope_pyobject pyArgs(PyTuple_Pack(1, pyArgsTuple.release()));

			// call関数をよぶ
			scope_pyobject result = PyObject_CallObject(pyFuncObj, pyArgs);
		}

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

		scope_state _scope_state(in.get(), in->mThreadStateForPyCmd);

		scope_pyobject codeObj = Py_CompileString((LPCSTR)src, "<string>", Py_eval_input);
		if (codeObj == nullptr) {
			PyErr_Clear();
			return false;
		}

		// タイムアウト監視スレッド側で評価の完了を検知するためのイベント
		HANDLE eval_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

		// 一定時間経っても評価が完了しない場合は中断する
		// (カーネルで待機することで、評価終了と同時に監視が解除される)
		std::thread th([eval_event]() {

			if (WaitForSingleObject(eval_event, CALC_TIMEOUT_IN_MSEC) == WAIT_TIMEOUT) {

				// 評価が終わった直後だった場合は中断しない
				if (WaitForSingleObject(eval_event, 0) == WAIT_OBJECT_0) {
					return;
				}

				// 中止するために例外を発行する
				PyErr_SetInterrupt();
			}
		});

		scope_pyobject pyObject = PyEval_EvalCode(codeObj, in->mGlobalDictForCalc, in->mGlobalDictForCalc);

		SetEvent(eval_event);

		// タイムアウト監視スレッドの完了を待つ
		th.join();
		CloseHandle(eval_event);

		// 文をインタープリタ側で評価した結果エラーだった場合
		if (PyErr_Occurred() != nullptr) {
			PyErr_Clear();
			return false;
		}

		if (pyObject == nullptr) {
			return false;
		}

		// 数値(または文字列)以外の結果が返された場合は採用しない
		if (PyLong_Check(pyObject) == 0 &&
			PyFloat_Check(pyObject) == 0 &&
			PyUnicode_Check(pyObject) == 0) {
			return false;
		}

		std::string s(py2str(pyObject));
		if (s.empty()) {
			// 文字列表現を取得できない場合は採用しない
			return false;
		}

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

