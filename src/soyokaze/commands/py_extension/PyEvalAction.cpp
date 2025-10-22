#include "pch.h"
#include "PyEvalAction.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/Message.h"
#include "python/PythonDLLLoader.h"
#include <map>
#include <thread>

namespace launcherapp { namespace commands { namespace py_extension  {

using namespace launcherapp::commands::common;

struct PyEvalAction::PImpl
{
	const CommandParam* mParam{nullptr};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


PyEvalAction::PyEvalAction(const CommandParam* param) : in(new PImpl)
{
	in->mParam = param;
}

PyEvalAction::~PyEvalAction()
{
}

// Action
// アクションの内容を示す名称
CString PyEvalAction::GetDisplayName()
{
	return _T("実行");
}

// アクションを実行する
bool PyEvalAction::Perform(Parameter* args_, String* errMsg)
{
	UNREFERENCED_PARAMETER(args_);

	auto dllLoader = PythonDLLLoader::Get();
	if (dllLoader->IsAvailable() == false) {
		if (errMsg) { *errMsg = _T("Pythonを利用できません"); }
		return false;
	}

	auto proxy = dllLoader->GetLibrary();
	if (proxy->IsPyCmdAvailable() == false) {
		if (errMsg) { *errMsg = _T("コマンドを実行できません。\n(Python3.12以降が必要です)"); }
		return false;
	}

	String name;
	UTF2UTF(in->mParam->mName, name);
	String script;
	UTF2UTF(in->mParam->mScript, script);

	// 引数用の配列を生成
	CString tmpW;
	std::string tmpA;

	std::vector<char*> args;
	int paramCount = args_->GetParamCount();
	args.reserve(paramCount + 1);
	for (int i = 0; i < paramCount; ++i) {
		tmpW = args_->GetParam(i);
		UTF2UTF(tmpW, tmpA);
		auto p = new char[tmpA.size()+1];
		memcpy(p, tmpA.data(), tmpA.size()+1);
		args.push_back(p);
	}
	// 終端としての目印となるnullを追加
	args.push_back(nullptr);

	std::thread th([proxy, name, script, args]() {

		char* pyErrMsg{nullptr};

		auto argsPtr = &(*(args.begin()));
		bool result = proxy->Evaluate(script.c_str(), (const char**)argsPtr, &pyErrMsg);

		// ラムダ関数の外で生成したメモリを解放
		for (auto p : args) {
			delete [] p;
		}

 		if (result == false) {
			if (pyErrMsg) {
				String error(name);
				error += "\r\n";
				error += pyErrMsg;
				PopupMessage(error);
			}
			proxy->ReleaseBuffer(pyErrMsg);
			return false;
		}
		return true;
	});

	th.detach();

	return true;
}

}}}

