#include "pch.h"
#include "WinScpCommandParam.h"
#include "setting/Settings.h"
#include "utility/Path.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace winscp {

CommandParam::CommandParam()
{
}


CommandParam::CommandParam(const CommandParam& rhs)
{
	mIsEnable = rhs.mIsEnable;
	mIsUsePortable = rhs.mIsUsePortable;
	mWinScpExeFilePath = rhs.mWinScpExeFilePath;
	mCachedWinExpFilePath = rhs.mCachedWinExpFilePath;
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (&rhs != this) {
		mIsEnable = rhs.mIsEnable;
		mIsUsePortable = rhs.mIsUsePortable;
		mWinScpExeFilePath = rhs.mWinScpExeFilePath;
		mCachedWinExpFilePath = rhs.mCachedWinExpFilePath;
	}
	return *this;
}

bool CommandParam::ResolveExecutablePath(CString& executableFilePath)
{
	if (mIsUsePortable) {
		// Portable版を使用する場合はアプリ設定で指定されたファイルパスを返す
		executableFilePath = mWinScpExeFilePath;
		return true;
	}

	if (mCachedWinExpFilePath.IsEmpty() == FALSE) {
		executableFilePath = mCachedWinExpFilePath;
		return true;
	}

	// Portable版を使用しない(=インストール版を使用する)場合は、レジストリからWinScp.exeのパスを得る
	std::vector<LPCTSTR> envNames = {
		_T("ProgramFiles"), _T("ProgramFiles(x86)")
	};

	Path path;
	for (auto& envName : envNames) {
		size_t reqLen = 0;
		if (_tgetenv_s(&reqLen, path, path.size(), envName) != 0) {
			continue;
		}
		path.Append(_T("WinScp\\WinScp.exe"));
		if (path.FileExists() == false) {
			continue;
		}

		executableFilePath = path;
		mCachedWinExpFilePath = path;
		return true;
	}

	spdlog::warn("Unable to find WinScp.exe.");
	return false;
}

bool CommandParam::Save(Settings& settings) const
{
	settings.Set(_T("WinScpSetting:IsEnable"), mIsEnable);
	settings.Set(_T("WinScpSetting:IsUsePortable"), mIsUsePortable);
	settings.Set(_T("WinScpSetting:ExecutbleFilePath"), mWinScpExeFilePath);
	return true;
}

bool CommandParam::Load(Settings& settings)
{
	mIsEnable = settings.Get(_T("WinScpSetting:IsEnable"), true);
	mIsUsePortable = settings.Get(_T("WinScpSetting:IsUsePortable"), false);
	mWinScpExeFilePath = settings.Get(_T("WinScpSetting:ExecutbleFilePath"), _T(""));
	mCachedWinExpFilePath.Empty();
	return true;
}

}}} // end of namespace launcherapp::commands::winscp

