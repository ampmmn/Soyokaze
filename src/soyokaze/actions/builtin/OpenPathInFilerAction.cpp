#include "pch.h"
#include "OpenPathInFilerAction.h"
#include "setting/AppPreference.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "actions/core/ActionParameter.h"
#include "utility/Path.h"

namespace launcherapp { namespace actions { namespace builtin {


using namespace launcherapp::commands::common;
using namespace launcherapp::actions::core;


OpenPathInFilerAction::OpenPathInFilerAction(const CString fullPath) :
 	mFullPath(fullPath)
{
}

OpenPathInFilerAction::~OpenPathInFilerAction()
{
}

// Action
// アクションの内容を示す名称
CString OpenPathInFilerAction::GetDisplayName()
{
	if (Path::IsDirectory(mFullPath)) {
		return _T("フォルダを開く");
	}
	else {
		return _T("パスを開く");
	}
}

// アクションを実行する
bool OpenPathInFilerAction::Perform(Parameter* args_, String* errMsg)
{
	CString path;
	CString param;

	// 与えられた実行時引数を配列にコピー
	int argCount = args_->GetParamCount();
	std::vector<CString> args;
	args.reserve(argCount);
	for (int i = 0; i < argCount; ++i) {
		args.push_back(args_->GetParam(i));
	}

	bool isFilerAvailable = false;

	auto pref = AppPreference::Get();
	if (pref->IsUseFiler()) {
		// 外部ファイラを使う場合はファイラ経由でパスを表示する形に差し替える
		param = pref->GetFilerParam();
		param.Replace(_T("$target"), path);

		auto filerPath = pref->GetFilerPath();
		ExpandArguments(filerPath, args);
		ExpandMacros(filerPath);

		isFilerAvailable = Path::FileExists(filerPath);
		if (isFilerAvailable) {
			path = filerPath;
		}
		else {
			// ファイラーが見つからない旨をログにだす
			spdlog::warn(_T("Failed to locate the specified file manager. {}"), (LPCTSTR)filerPath);
		}
	}

	// 登録されたファイラーがない、または、利用できない場合はエクスプローラで開く
	if (isFilerAvailable == false) {
		if (Path::IsDirectory(mFullPath) == false) {
			PathRemoveFileSpec(path.GetBuffer(MAX_PATH_NTFS));
			path.ReleaseBuffer();
		}
		else {
			path = mFullPath;
		}
		param = _T("open");
	}

	SubProcess exec(ParameterBuilder::EmptyParam());
	SubProcess::ProcessPtr process;
	if (exec.Run(path, param, process) == false) {
		if (errMsg) {
			UTF2UTF(process->GetErrorMessage(), *errMsg);
		}
		return false;
	}
	return true;
}


}}}

