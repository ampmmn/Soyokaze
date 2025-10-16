#include "pch.h"
#include "PyEvalAction.h"
#include "commands/common/CommandParameterFunctions.h"
#include "python/PythonDLLLoader.h"
#include <map>

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

	if (proxy->IsBusy()) {
		if (errMsg) { *errMsg = _T("コマンドを実行できません。\n(先に実行したPython拡張コマンドが完了していない)"); }
		return false;
	}


	std::string tmpSrc;

	char* pyErrMsg{nullptr};
	if (proxy->Evaluate(UTF2UTF(in->mParam->mScript, tmpSrc).c_str(), &pyErrMsg) == false) {
		if (errMsg && pyErrMsg) { *errMsg = pyErrMsg; }
		proxy->ReleaseBuffer(pyErrMsg);
		return false;
	}
	return true;
}

}}}

