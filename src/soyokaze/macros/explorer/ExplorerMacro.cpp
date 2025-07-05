#include "pch.h"
#include "ExplorerMacro.h"
#include "commands/share/AfxWWrapper.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace macros { namespace builtin {

REGISTER_LAUNCHERMACRO(ExplorerMacro)

ExplorerMacro::ExplorerMacro()
{
	mName = _T("explorer");
}

ExplorerMacro::~ExplorerMacro()
{
}

bool ExplorerMacro::Evaluate(const std::vector<CString>& args, CString& result)
{
	if (args.size() == 0) {
		return false;
	}

	CString command = args[0];

	if (command.CompareNoCase(_T("location_path")) == 0) {
		return ExpandLocationPath(args, result);
	}
	else if (command.CompareNoCase(_T("selection_path")) == 0) {
		return ExpandSelectionPath(args, result);
	}
	return false;
}

bool ExplorerMacro::ExpandLocationPath(const std::vector<CString>& args, CString& result)
{
	UNREFERENCED_PARAMETER(args);

	// 管理者権限でアプリを実行していると、通常権限で実行しているインスタンスのAPI呼び出しに失敗する
	// launcher_proxy.exeを経由することにより、常に通常権限で実行されるようにする。
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	std::wstring path;
	if (proxy->GetExplorerCurrentDir(path) == false) {
		return false;
	}

	result = path.c_str();

	return true;
}

bool ExplorerMacro::ExpandSelectionPath(const std::vector<CString>& args, CString& result)
{
	int index = -1;
	if (args.size() > 1) {
		index = std::stoi(tstring((LPCTSTR)args[1]));
	}

	// 管理者権限でアプリを実行していると、通常権限で実行しているインスタンスのAPI呼び出しに失敗する
	// launcher_proxy.exeを経由することにより、常に通常権限で実行されるようにする。
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	std::wstring path;
	if (proxy->GetExplorerSelectionDir(path, index) == false) {
		return false;
	}

	result = path.c_str();

	return true;
}


}}}
