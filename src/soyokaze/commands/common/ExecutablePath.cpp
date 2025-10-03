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

void ExecutablePath::EnableMacros(bool enable)
{
	mIsEnableMacros = enable;
}

bool ExecutablePath::IsExecutable(bool includeRelative) const
{
	// マクロを展開する
	CString path{mPath};
	if (mIsEnableMacros) {
		ExpandMacros(path);
	}

	if (PathIsURL(path)) {
		// URLの場合はOK
		return true;
	}

	// 存在する絶対パスのパスの場合はファイルが存在すれば実行可能とする
	if (Path::FileExists(path)) {
		return true;
	}

	if (PathIsUNC(path)) {
		// UNCの場合、接続が確立されていない可能性があるので、ここでは判断をしない
		return true;
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

