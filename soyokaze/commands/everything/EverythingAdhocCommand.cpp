#include "pch.h"
#include "framework.h"
#include "EverythingAdhocCommand.h"
#include "commands/everything/EverythingCommandParam.h"
#include "commands/everything/EverythingResult.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace everything {

struct EverythingAdhocCommand::PImpl
{
	CommandParam mParam;
	EverythingResult mResult;
};


EverythingAdhocCommand::EverythingAdhocCommand(
	const CommandParam& param,
 	const EverythingResult& result
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mParam = param;
	in->mResult = result;
}

EverythingAdhocCommand::~EverythingAdhocCommand()
{
}

CString EverythingAdhocCommand::GetName()
{
	return in->mParam.mName + _T(" ") + PathFindFileName(in->mResult.mFullPath);
}

CString EverythingAdhocCommand::GetDescription()
{
	CString str;
	str.Format(_T("%s"), (LPCTSTR)in->mResult.mFullPath);
	return str;

}

CString EverythingAdhocCommand::GetGuideString()
{
	CString guideStr(_T("Enter:実行"));
	guideStr += _T(" Shift-Enter:パスをコピー");
	guideStr += _T(" Ctrl-Enter:フォルダを開く");

	return guideStr;
}

CString EverythingAdhocCommand::GetTypeDisplayName()
{
	return _T("Everything検索");
}

BOOL EverythingAdhocCommand::Execute(const Parameter& param)
{
	bool isCtrlPressed = param.GetNamedParamBool(_T("CtrlKeyPressed"));
	bool isShiftPressed = param.GetNamedParamBool(_T("ShiftKeyPressed"));
	if (isCtrlPressed == false && isShiftPressed != false) {
		// クリップボードにコピー
		Clipboard::Copy(in->mResult.mFullPath);
		return TRUE;
	}

	SubProcess exec(param);
	SubProcess::ProcessPtr process;
	if (exec.Run(in->mResult.mFullPath, param.GetParameterString(), process) == FALSE) {
		//in->mErrMsg = (LPCTSTR)process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON EverythingAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mResult.mFullPath);
}

launcherapp::core::Command*
EverythingAdhocCommand::Clone()
{
	return new EverythingAdhocCommand(in->mParam, in->mResult);
}

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

