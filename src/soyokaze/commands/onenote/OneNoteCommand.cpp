#include "pch.h"
#include "framework.h"
#include "OneNoteCommand.h"
#include "commands/onenote/OneNoteCommandParam.h"
#include "commands/onenote/OneNoteAppProxy.h"
#include "actions/builtin/CallbackAction.h"
#include "utility/ScopeAttachThreadInput.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>
#include <tlhelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

namespace launcherapp { namespace commands { namespace onenote {

struct OneNoteCommand::PImpl
{
	CString mDispName;
	CString mNotebookName;
	CStringW mNavigateID;
};




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(OneNoteCommand)

OneNoteCommand::OneNoteCommand() : in(new PImpl)
{
}


OneNoteCommand::OneNoteCommand(
	CommandParam* param,
	const OneNoteBook& book,
	const OneNoteSection& section,
	const OneNotePage& page
) : in(new PImpl)
{
	in->mNavigateID = page.GetID();
	in->mNotebookName = book.GetNickName();

	auto& dispName = in->mDispName;
	if (param->mPrefix.IsEmpty() == FALSE) {
		dispName = param->mPrefix;
		dispName += _T(" ");
	}
	dispName += section.GetName();
	dispName += _T("/");
	dispName += page.GetName();
}

OneNoteCommand::~OneNoteCommand()
{
}

CString OneNoteCommand::GetName()
{
	return in->mDispName;
}

CString OneNoteCommand::GetDescription()
{
	return in->mNotebookName;
}

CString OneNoteCommand::GetGuideString()
{
	return _T("⏎:ページをOneNoteで開く");
}

CString OneNoteCommand::GetTypeDisplayName()
{
	return TypeDisplayName() + _T("(") + in->mNotebookName + _T(")");
}

// OneNoteのメインウインドウを探す
static HWND FindOneNoteWindow()
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return nullptr;
	}

	struct local_param {
		static BOOL CALLBACK EnumWindowProc(HWND h, LPARAM lp) {

			if (IsWindowVisible(h) == FALSE) {
				return TRUE;
			}

			// 所望のプロセスに属するウインドウを探す
			DWORD pid=0;
			GetWindowThreadProcessId(h, &pid);
			auto thisPtr = (local_param*)lp;
			if (pid != thisPtr->mPID) {
				return TRUE;
			}

			// 所望のプロセスに属するウインドウが見つかったら探索終了
			thisPtr->mHwnd = h;
			return FALSE;
		}
		DWORD mPID;
		HWND mHwnd{nullptr};
	};

	PROCESSENTRY32W pe { sizeof(PROCESSENTRY32W) };

	// "onenote.exe"というプロセス名のPIDを探す
	// (たまたま同名のexeファイル名が存在する場合には誤検知する)
	HWND oneNoteWindow = nullptr;
	if (Process32FirstW(snapshot, &pe)) {
		do {
			if (_wcsicmp(pe.szExeFile, L"onenote.exe") != 0) {
				continue;
			}

			// 得られたPIDから、同じプロセスに属するトップレベルウインドウを探す
			local_param param{ pe.th32ProcessID };

			EnumWindows(local_param::EnumWindowProc, (LPARAM)& param);

			if (IsWindow(param.mHwnd)) {
				// 見つかったら、そのウインドウハンドルを返す
				oneNoteWindow = param.mHwnd;
				break;
			}

		} while (Process32NextW(snapshot, &pe));
	}

	CloseHandle(snapshot);
	return oneNoteWindow;
}


bool OneNoteCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	UNREFERENCED_PARAMETER(modifierFlags);

	*action = new CallbackAction(_T("ページをOneNoteで開く"), [&](Parameter*, String*) -> bool {
		OneNoteAppProxy app;
		if (app.NavigateTo(in->mNavigateID) == false) {
			spdlog::error(L"Failed to NavigateTo id:{}", (LPCWSTR)in->mNavigateID);
		}
	
		// OneNoteのウインドウを探して前面に出す
		HWND h = FindOneNoteWindow();
		if (h == nullptr) {
			spdlog::warn("Failed to find onenote window.");
			return true;
		}
	
		ScopeAttachThreadInput scope;
		SetForegroundWindow(h);
		return true;
	});

	return true;
}

CString OneNoteCommand::GetErrorString()
{
	return _T("");
}

HICON OneNoteCommand::GetIcon()
{
	return IconLoader::Get()->LoadExtensionIcon(_T(".one"));
}

launcherapp::core::Command*
OneNoteCommand::Clone()
{
	auto newCmd = new OneNoteCommand();
	newCmd->in->mDispName = in->mDispName;
	newCmd->in->mNotebookName = in->mNotebookName;
	newCmd->in->mNavigateID = in->mNavigateID;

	return newCmd;
}

CString OneNoteCommand::TypeDisplayName()
{
	return _T("OneNote");
}

}}} // end of namespace launcherapp::commands::onenote
