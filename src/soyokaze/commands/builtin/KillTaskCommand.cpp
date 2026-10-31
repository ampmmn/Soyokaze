#include "pch.h"
#include "framework.h"
#include "KillTaskCommand.h"
#include "core/IFIDDefine.h"
#include "commands/builtin/KillTaskCandidateCommand.h"
#include "matcher/PartialMatchPattern.h"
#include "icon/IconLoader.h"
#include "utility/ProcessPath.h"
#include <tlhelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

namespace {

struct ProcessCandidateInfo
{
	DWORD mProcessId;
	CString mImageName;
	CString mImagePath;
	CString mCommandLine;
	uint64_t mCreationTime;
	int mMatchLevel;
	bool mHasTerminateAccess;
};

/** プロセス情報と作成時刻を取得する */
bool QueryProcessInfo(HANDLE process, CString& imagePath, uint64_t& creationTime)
{
	std::vector<TCHAR> pathBuffer(32768);
	DWORD pathLength = static_cast<DWORD>(pathBuffer.size());
	if (QueryFullProcessImageName(process, 0, pathBuffer.data(), &pathLength) != FALSE) {
		imagePath.SetString(pathBuffer.data(), pathLength);
	}

	FILETIME creation{};
	FILETIME exit{};
	FILETIME kernel{};
	FILETIME user{};
	if (GetProcessTimes(process, &creation, &exit, &kernel, &user) == FALSE) {
		return false;
	}

	ULARGE_INTEGER value{};
	value.LowPart = creation.dwLowDateTime;
	value.HighPart = creation.dwHighDateTime;
	creationTime = value.QuadPart;
	return true;
}

}

CString KillTaskCommand::TYPE(_T("Builtin-KillTask"));

REGISTER_BUILTINCOMMAND(KillTaskCommand)

CString KillTaskCommand::GetType()
{
	return TYPE;
}

KillTaskCommand::KillTaskCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("kill"))
{
	mDescription = _T("【タスクを終了する】");
	mCanSetConfirm = true;
	mCanDisable = true;
}

KillTaskCommand::KillTaskCommand(const KillTaskCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

KillTaskCommand::~KillTaskCommand()
{
}

bool KillTaskCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (BuiltinCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = static_cast<launcherapp::commands::core::ExtraCandidateSource*>(this);
		return true;
	}
	return false;
}

int KillTaskCommand::Match(Pattern* pattern)
{
	int level = pattern->Match(GetName());
	if (level == Pattern::WholeMatch && pattern->GetWordCount() > 1) {
		return Pattern::HiddenMatch;
	}
	return level;
}

BOOL KillTaskCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);
	AfxMessageBox(_T("削除するタスク名を指定してください"));
	return TRUE;
}

launcherapp::core::Command* KillTaskCommand::Clone()
{
	return new KillTaskCommand(*this);
}

bool KillTaskCommand::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	if (GetName().CompareNoCase(pattern->GetFirstWord()) != 0) {
		return false;
	}

	RefPtr<PatternInternal> patternInternal;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&patternInternal) == false) {
		return false;
	}

	std::vector<CString> words;
	patternInternal->GetRawWords(words);
	CString queryText;
	for (size_t i = 1; i < words.size(); ++i) {
		queryText += words[i];
		queryText += _T(" ");
	}

	RefPtr<PartialMatchPattern> processPattern(PartialMatchPattern::Create());
	processPattern->SetWholeText(queryText);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return false;
	}

	std::vector<ProcessCandidateInfo> processes;
	PROCESSENTRY32W processEntry{};
	processEntry.dwSize = sizeof(processEntry);
	if (Process32FirstW(snapshot, &processEntry) != FALSE) {
		do {
			DWORD processId = processEntry.th32ProcessID;
			if (processId == 0 || processId == 4 || processId == GetCurrentProcessId()) {
				continue;
			}

			HANDLE process = OpenProcess(
				PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE,
				FALSE,
				processId);
			bool hasTerminateAccess = process != nullptr;
			if (process == nullptr && GetLastError() != ERROR_ACCESS_DENIED) {
				continue;
			}

			if (process == nullptr) {
				process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
			}

			CString imagePath;
			uint64_t creationTime = 0;
			bool hasCreationTime = false;
			if (process != nullptr) {
				hasCreationTime = QueryProcessInfo(process, imagePath, creationTime);
				CloseHandle(process);
			}

			if (hasTerminateAccess && hasCreationTime == false) {
				continue;
			}

			CString imageName(processEntry.szExeFile);
			if (imagePath.IsEmpty() == FALSE) {
				int separator = imagePath.ReverseFind(_T('\\'));
				CString pathImageName = imagePath.Mid(separator + 1);
				if (pathImageName.CompareNoCase(imageName) != 0) {
					continue;
				}
			}

			int imageNameMatchLevel = processPattern->Match(imageName);
			CString processIdText;
			processIdText.Format(_T("%lu"), processId);
			int processIdMatchLevel = processPattern->Match(processIdText);
			int matchLevel = std::max(imageNameMatchLevel, processIdMatchLevel);
			if (matchLevel == Pattern::Mismatch) {
				continue;
			}

			CString commandLine;
			try {
				commandLine = ProcessPath::GetFullCommandLine(processId);
			}
			catch (ProcessPath::Exception&) {
			}
			if (commandLine.IsEmpty()) {
				commandLine = imageName;
			}

			processes.push_back({processId, imageName, imagePath, commandLine, creationTime, matchLevel, hasTerminateAccess});
		} while (Process32NextW(snapshot, &processEntry) != FALSE);
	}
	CloseHandle(snapshot);

	std::sort(processes.begin(), processes.end(), [](const ProcessCandidateInfo& lhs, const ProcessCandidateInfo& rhs) {
		if (lhs.mMatchLevel != rhs.mMatchLevel) {
			return lhs.mMatchLevel > rhs.mMatchLevel;
		}
		int nameOrder = lhs.mImageName.CompareNoCase(rhs.mImageName);
		return nameOrder == 0 ? lhs.mProcessId < rhs.mProcessId : nameOrder < 0;
	});

	for (const auto& process : processes) {
		commands.Add(CommandQueryItem(process.mMatchLevel,
			new KillTaskCandidateCommand(
				process.mProcessId,
				process.mImageName,
				process.mImagePath,
				process.mCommandLine,
				process.mHasTerminateAccess == false,
				process.mCreationTime,
				mIsConfirmBeforeRun)));
	}

	return true;
}

void KillTaskCommand::ClearCache()
{
}

}
}
}
