#pragma once

namespace launcherapp { namespace commands { namespace webhistory {

class TempDirectory
{
public:
	struct Exception {};

private:
	TempDirectory();
	~TempDirectory();

public:
	static TempDirectory* GetInstance();

	// ファイルパスを生成する
	CString MakePath(LPCTSTR fileName);

	// フォルダ内のファイルをすべて消す
	void Clear();

private:
	CString mDirPath;

};


}}}



