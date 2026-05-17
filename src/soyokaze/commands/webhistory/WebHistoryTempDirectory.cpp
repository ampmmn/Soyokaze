#include "pch.h"
#include "WebHistoryTempDirectory.h"
#include "utility/Path.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace webhistory {


TempDirectory::TempDirectory()
{
	Path dbDstPath(Path::APPDIRPERMACHINE, _T("tmp"));
	mDirPath = (LPCTSTR)dbDstPath;
}

TempDirectory::~TempDirectory()
{
}


TempDirectory* TempDirectory::GetInstance()
{
	static TempDirectory inst;
	return &inst;
}


// ファイルパスを生成する
CString TempDirectory::MakePath(LPCTSTR fileName)
{
	// ディレクトリを生成しておく
	if (Path::IsDirectory(mDirPath) == false) {
		if (CreateDirectory(mDirPath, nullptr) == FALSE) {
			spdlog::warn(_T("Failed to create directory {}"), (LPCTSTR)mDirPath);
			throw Exception();
		}
	}

	Path fullPath(mDirPath);
	fullPath.Append(fileName);
	return (LPCTSTR)fullPath;
}


// フォルダ内のファイルをすべて消す
void TempDirectory::Clear()
{
	if (Path::IsDirectory(mDirPath) == false) {
		return;
	}

	// tmpフォルダ直下のファイルをすべて消す(ファイルのみ)
	CString pattern(mDirPath);
	pattern += _T("\\*.*");

	CFileFind f;
	BOOL isLoop = f.FindFile(pattern, 0);
	while (isLoop) {
		isLoop = f.FindNextFile();
		if (f.IsDots()) {
			continue;
		}
		DeleteFile(f.GetFilePath());
	}

	f.Close();
}



}}}



