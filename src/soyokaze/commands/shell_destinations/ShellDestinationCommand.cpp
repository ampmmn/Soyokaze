#include "pch.h"
#include "ShellDestinationCommand.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/builtin/ShowPropertiesAction.h"
#include "actions/core/ActionBase.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"

#include <objbase.h>
#include <algorithm>
#include <cwctype>
#include <limits>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace shell_destinations {

namespace {

using OpenPathInFilerAction = launcherapp::actions::builtin::OpenPathInFilerAction;
using ShowPropertiesAction = launcherapp::actions::builtin::ShowPropertiesAction;
using ActionBase = launcherapp::actions::core::ActionBase;
using SubProcess = launcherapp::commands::common::SubProcess;

/**
  表示用パスからファイル名部分を取得する
  @param[in] displayPath 表示用パス
  @return ファイル名
*/
CString GetFileName(const std::wstring& displayPath)
{
	std::wstring path(displayPath);
	auto queryPos = path.find_first_of(L"?#");
	if (queryPos != std::wstring::npos) {
		path.resize(queryPos);
	}

	auto separatorPos = path.find_last_of(L"/\\");
	if (separatorPos != std::wstring::npos) {
		path = path.substr(separatorPos + 1);
	}
	return CString(path.c_str());
}

/**
  Ctrl+Enter/Alt+Enter で扱えるローカルファイルシステムのパスか判定する
  @param[in] path 判定対象のパス
  @return true:ローカルパス false:URLや特殊形式など
*/
bool IsLocalPath(const CString& path)
{
	if (path.IsEmpty() || PathIsURL(path)) {
		return false;
	}
	if (PathIsUNC(path)) {
		return true;
	}
	return path.GetLength() >= 3 && iswalpha(path[0]) && path[1] == _T(':') &&
		(path[2] == _T('\\') || path[2] == _T('/'));
}

/**
  リンクデータを一時的な .lnk ファイルに書き出す
  @param[in] linkData ショートカットデータ
  @param[out] path 作成したファイルのパス
  @return true:成功 false:失敗
*/
bool WriteTemporaryLink(const std::vector<unsigned char>& linkData, CString& path)
{
	TCHAR tempDirectory[MAX_PATH]{};  // GetTempPathでMAX_PATHを超えるケースは通常ない
	DWORD directoryLength = GetTempPath(_countof(tempDirectory), tempDirectory);
	if (directoryLength == 0 || directoryLength >= _countof(tempDirectory)) {
		return false;
	}

	GUID id{};
	if (FAILED(CoCreateGuid(&id))) {
		return false;
	}
	TCHAR idString[40]{};
	if (StringFromGUID2(id, idString, _countof(idString)) == 0) {
		return false;
	}

	path.Format(_T("%sSoyokaze-%s.lnk"), tempDirectory, idString);
	HANDLE file = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);
	if (file == INVALID_HANDLE_VALUE) {
		path.Empty();
		return false;
	}

	size_t offset = 0;
	bool succeeded = true;
	while (offset < linkData.size()) {
		DWORD written = 0;
		DWORD writeLength = static_cast<DWORD>((std::min)(linkData.size() - offset,
			static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
		if (WriteFile(file, linkData.data() + offset, writeLength, &written, nullptr) == FALSE || written == 0) {
			succeeded = false;
			break;
		}
		offset += written;
	}
	CloseHandle(file);
	if (!succeeded) {
		DeleteFile(path);
		path.Empty();
	}
	return succeeded;
}

// ショートカットを一時展開し、既定の関連付けで実行する
class OpenShellDestinationAction : public ActionBase
{
public:
	explicit OpenShellDestinationAction(const std::vector<unsigned char>& linkData) : mLinkData(linkData)
	{
	}

	CString GetDisplayName() override
	{
		return _T("開く");
	}

	bool Perform(Parameter* param, String* errMsg) override
	{
		CString temporaryPath;
		if (WriteTemporaryLink(mLinkData, temporaryPath) == false) {
			return false;
		}

		// ShellExecute がリンク先を解決した後で一時ファイルを削除する
		struct TemporaryFileRemover
		{
			CString& mPath;
			~TemporaryFileRemover()
			{
				DeleteFile(mPath);
			}
		} remover{ temporaryPath };

		SubProcess processRunner(param);
		SubProcess::ProcessPtr process;
		if (processRunner.Run(temporaryPath, process) == false) {
			if (errMsg != nullptr && process != nullptr) {
				UTF2UTF(process->GetErrorMessage(), *errMsg);
			}
			return false;
		}
		return true;
	}

private:
	std::vector<unsigned char> mLinkData;
};

} // 無名名前空間

struct ShellDestinationCommand::PImpl
{
	AutomaticDestinationEntry mEntry;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(ShellDestinationCommand)

ShellDestinationCommand::ShellDestinationCommand(const AutomaticDestinationEntry& entry) :
	AdhocCommandBase(GetFileName(entry.displayPath), CString(entry.displayPath.c_str())),
	in(std::make_unique<PImpl>())
{
	in->mEntry = entry;
}

ShellDestinationCommand::~ShellDestinationCommand() = default;

CString ShellDestinationCommand::GetTypeDisplayName()
{
	return _T("ピン留めされた項目");
}

bool ShellDestinationCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto modifiers = hotkeyAttr.GetModifiers();
	if (modifiers == 0) {
		*action = new OpenShellDestinationAction(in->mEntry.lnk);
		return true;
	}

	if (IsLocalPath(CString(in->mEntry.displayPath.c_str())) == false) {
		return false;
	}
	if (modifiers == MOD_CONTROL) {
		*action = new OpenPathInFilerAction(CString(in->mEntry.displayPath.c_str()));
		return true;
	}
	if (modifiers == MOD_ALT) {
		*action = new ShowPropertiesAction(CString(in->mEntry.displayPath.c_str()));
		return true;
	}
	return false;
}

HICON ShellDestinationCommand::GetIcon()
{
	HICON linkIcon = IconLoader::Get()->LoadLnkIcon(in->mEntry.lnk);
	if (linkIcon != nullptr) {
		return linkIcon;
	}

	if (in->mEntry.isDirectory) {
		return IconLoader::Get()->LoadFolderIcon();
	}

	CString fileName = GetFileName(in->mEntry.displayPath);
	int dotPos = fileName.ReverseFind(_T('.'));
	if (dotPos < 0) {
		return IconLoader::Get()->LoadWebIcon();
	}
	return IconLoader::Get()->LoadExtensionIcon(fileName.Mid(dotPos));
}

launcherapp::core::Command* ShellDestinationCommand::Clone()
{
	return new ShellDestinationCommand(in->mEntry);
}

int ShellDestinationCommand::GetMenuItemCount()
{
	return IsLocalPath(CString(in->mEntry.displayPath.c_str())) ? 3 : 1;
}

bool ShellDestinationCommand::GetMenuItem(int index, Action** action)
{
	if (index == 0) {
		return GetAction(HOTKEY_ATTR(0, VK_RETURN), action);
	}
	if (index == 1) {
		return GetAction(HOTKEY_ATTR(MOD_CONTROL, VK_RETURN), action);
	}
	if (index == 2) {
		return GetAction(HOTKEY_ATTR(MOD_ALT, VK_RETURN), action);
	}
	return false;
}

bool ShellDestinationCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = static_cast<launcherapp::commands::core::ContextMenuSource*>(this);
		return true;
	}
	return false;
}

}}}
