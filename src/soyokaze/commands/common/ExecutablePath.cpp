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

bool ExecutablePath::IsExecutable() const
{
	// マクロを展開する
	CString path{mPath};
	ExpandMacros(path);

	if (PathIsURL(path) || PathIsUNC(path)) {
		// URL or UNCの場合はOK
		return true;
	}

	if (PathIsRelative(path)) {
		// 相対パスの場合は絶対パスに変換できるか試して判断をする
		CString fullPath;
		launcherapp::utility::LocalPathResolver resolver;
		if (resolver.Resolve(path, fullPath) == false) {
			return false;
		}
		return true;
	}

	// 絶対パスの場合は単にファイルが存在すれば実行可能とする
	return Path::FileExists(path);
}


}}} // end of namespace launcherapp::commands::common;

