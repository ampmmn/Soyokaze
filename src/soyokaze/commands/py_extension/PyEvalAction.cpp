#include "pch.h"
#include "PyEvalAction.h"
#include "commands/py_extension/PyEvalArgument.h"
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

	PyEvalArgument args(args_);
	char** argsPtr = args.release();
	std::thread th([proxy, name, script, argsPtr]() {

		PyEvalArgument args(argsPtr);

		char* pyErrMsg{nullptr};
 		if (proxy->Evaluate(script.c_str(), (const char**)args.get(), &pyErrMsg) == false) {
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

