#include "pch.h"
#include "framework.h"
#include "KillTaskCandidateCommand.h"
#include "actions/builtin/CallbackAction.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CallbackAction = launcherapp::actions::builtin::CallbackAction;

namespace launcherapp {
namespace commands {
namespace builtin {

struct KillTaskCandidateCommand::PImpl
{
	DWORD mProcessId;
	CString mImageName;
	CString mImagePath;
	CString mCommandLine;
	bool mIsPermissionDenied;
	uint64_t mCreationTime;
	bool mIsConfirmBeforeRun;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(KillTaskCandidateCommand)

KillTaskCandidateCommand::KillTaskCandidateCommand(
	DWORD processId,
	const CString& imageName,
	const CString& imagePath,
	const CString& commandLine,
	bool isPermissionDenied,
	uint64_t creationTime,
	bool isConfirmBeforeRun
) :
	AdhocCommandBase(imageName, imageName),
	in(std::make_unique<PImpl>())
{
	mName.Format(_T("kill %s"), (LPCTSTR)imageName);
	in->mCommandLine = commandLine.IsEmpty() ? imageName : commandLine;
	mDescription.Format(_T("PID:%lu (%s)"), processId, (LPCTSTR)in->mCommandLine);
	if (isPermissionDenied) {
		mDescription += _T("（権限不足のため終了できません）");
	}
	in->mProcessId = processId;
	in->mImageName = imageName;
	in->mImagePath = imagePath;
	in->mIsPermissionDenied = isPermissionDenied;
	in->mCreationTime = creationTime;
	in->mIsConfirmBeforeRun = isConfirmBeforeRun;
}

KillTaskCandidateCommand::~KillTaskCandidateCommand()
{
}

CString KillTaskCandidateCommand::GetTypeDisplayName()
{
	return _T("タスク終了コマンド");
}

bool KillTaskCandidateCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}

	*action = new CallbackAction(_T("タスク終了"), [&](Parameter*, String*) -> bool {
		if (in->mIsPermissionDenied || in->mCreationTime == 0) {
			return true;
		}

		HANDLE process = OpenProcess(
			PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE,
			FALSE,
			in->mProcessId);
		if (process == nullptr) {
			return true;
		}

		FILETIME creation{};
		FILETIME exit{};
		FILETIME kernel{};
		FILETIME user{};
		if (GetProcessTimes(process, &creation, &exit, &kernel, &user) == FALSE) {
			CloseHandle(process);
			return true;
		}

		ULARGE_INTEGER creationTime{};
		creationTime.LowPart = creation.dwLowDateTime;
		creationTime.HighPart = creation.dwHighDateTime;
		if (creationTime.QuadPart != in->mCreationTime) {
			CloseHandle(process);
			return true;
		}

		if (in->mIsConfirmBeforeRun) {
			CString message;
			message.Format(_T("タスクを終了しますか?\n\nPID:%lu (%s)"),
				in->mProcessId,
				(LPCTSTR)in->mCommandLine);
			if (AfxMessageBox(message, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) {
				CloseHandle(process);
				return true;
			}
		}

		TerminateProcess(process, 0);
		CloseHandle(process);
		return true;
	});
	return true;
}

HICON KillTaskCandidateCommand::GetIcon()
{
	if (in->mImagePath.IsEmpty()) {
		return IconLoader::Get()->LoadUnknownIcon();
	}

	HICON icon = IconLoader::Get()->LoadIconFromPath(in->mImagePath);
	return icon ? icon : IconLoader::Get()->LoadUnknownIcon();
}

launcherapp::core::Command* KillTaskCandidateCommand::Clone()
{
	return new KillTaskCandidateCommand(
		in->mProcessId,
		in->mImageName,
		in->mImagePath,
		in->mCommandLine,
		in->mIsPermissionDenied,
		in->mCreationTime,
		in->mIsConfirmBeforeRun);
}

}
}
}
