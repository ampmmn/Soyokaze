#include "pch.h"
#include "ExecutablePath.h"
#include "commands/common/ExpandFunctions.h"
#include "utility/LocalPathResolver.h"
#include "utility/Path.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace common {

ExecutablePath::ExecutablePath(const CString& path) : mPath(path)
{
}

ExecutablePath::~ExecutablePath()
{
}

bool ExecutablePath::IsExecutable(bool includeRelative) const
{
	// マクロを展開する
	CString path{mPath};
	ExpandMacros(path);

	if (PathIsURL(path)) {
		// URLの場合はOK
		return true;
	}

	// 存在する絶対パスのパスの場合はファイルが存在すれば実行可能とする
	if (Path::FileExists(path)) {
		return true;
	}

	if (PathIsUNC(path)) {
		// UNCの場合はホスト名を取り出す
		int end = path.Find(L'\\', 2);
		CString hostName(end == -1 ? path : path.Left(end));

		if (Path::IsHostConnected(hostName) == false) {
			// 接続が確立していないホストの場合、パスが有効かどうかをこの時点では判断できないため、候補として表示する
			// (コマンド実行時に接続を試みる)
			return true;
		}

		// パスで指定された文字列がホスト名そのものの場合、PathFileExistsではひっかからないため、候補として表示する
		if (end == -1) {
			return true;
		}

		// "\\hostname\"も同様
		hostName += _T("\\");
		if (path == hostName) {
			return true;
		}
	}

	if (includeRelative && PathIsRelative(path)) {
		// 相対パスの場合は絶対パスに変換できるか試して判断をする
		CString fullPath;
		launcherapp::utility::LocalPathResolver resolver;
		if (resolver.Resolve(path, fullPath) == false) {
			return false;
		}
		return true;
	}

	return false;
}


}}} // end of namespace launcherapp::commands::common;

