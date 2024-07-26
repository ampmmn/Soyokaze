#include "pch.h"
#include "commands/core/CommandRanking.h"
#include "utility/AppProfile.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PRIORITY_FILENAME _T("priority.dat")

namespace launcherapp {
namespace commands {
namespace core {

struct CommandRanking::PImpl
{
	const CString& GetFilePath()
	{
		TCHAR path[MAX_PATH_NTFS];
		CAppProfile::GetDirPath(path, MAX_PATH_NTFS);
		PathAppend(path, PRIORITY_FILENAME);

		mFilePath = path;

		return mFilePath;
	}

	// コマンドごとのランクを保持するマップ
	std::map<CString, int> mRank;

	// 設定ファイル保存先パス
	CString mFilePath;

	bool mIsLoaded = false;

	bool mIsTemporary = false;
};

CommandRanking::CommandRanking() : in(std::make_unique<PImpl>())
{
}

CommandRanking::~CommandRanking()
{
}

CommandRanking* CommandRanking::GetInstance()
{
	static CommandRanking inst;
	return &inst;
}

bool CommandRanking::Load()
{
	in->mIsLoaded = true;

	const CString& filePath = in->GetFilePath();

	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, filePath, _T("r,ccs=UTF-8")) != 0 || fpIn == nullptr) {
		SPDLOG_WARN(_T("priority data file does not exist. {}"), PRIORITY_FILENAME);
		return false;
	}

	CStdioFile file(fpIn);
	std::map<CString, int> ranking;

	CString strLine;
	while(file.ReadString(strLine)) {

		strLine.Trim();
		if (strLine.IsEmpty()) {
			continue;
		}

		int n = 0;
		CString name = strLine.Tokenize(_T("\t"), n);
		if (name.IsEmpty()) {
			continue;
		}
		CString rankStr = strLine.Tokenize(_T("\t"), n);

		int rankVal = 0;
		if (rankStr.IsEmpty() || _stscanf_s(rankStr, _T("%d"), &rankVal) != 1) {
			continue;
		}
		ranking[name] = rankVal;
	}

	file.Close();
	fclose(fpIn);

	in->mRank.swap(ranking);

	return true;
}

bool CommandRanking::Save()
{
	if (in->mIsLoaded == false) {
		SPDLOG_WARN(_T("skip save."));
		return false;
	}

	const CString& filePath = in->GetFilePath();
	FILE* fpOut = nullptr;
	try {
		CString filePathTmp = filePath + _T(".tmp");

		if (_tfopen_s(&fpOut, filePathTmp, _T("w,ccs=UTF-8")) != 0 || fpOut == nullptr) {
			return false;
		}
		CStdioFile file(fpOut);

		CString lineBuff;
		for (auto& elem : in->mRank) {

			const CString& name = elem.first;
			int rank = elem.second;

			lineBuff.Format(_T("%s\t%d\n"), (LPCTSTR)name, rank);
			file.WriteString(lineBuff);
		}

		file.Close();
		fclose(fpOut);

		// 最後に一時ファイルを書き戻す
		if (CopyFile(filePathTmp, filePath, FALSE)) {
			DeleteFile(filePathTmp);
		}
		return true;
	}
	catch(CFileException* e) {
		spdlog::error(_T("an exception occurred."));
		e->Delete();
		fclose(fpOut);
	}
	return true;
}

void CommandRanking::SetFilePath(const CString& path)
{
	in->mFilePath = path;
}

// 順位変更
void CommandRanking::Add(const CString& name, int num)
{
	in->mRank[name] += num;
}

void CommandRanking::Set(const CString& name, int num)
{
	in->mRank[name] = num;
}

// 順位取得
int CommandRanking::Get(const CString& name) const
{
	auto it = in->mRank.find(name);
	return it != in->mRank.end() ? it->second : 0;
}

// 削除
bool CommandRanking::Delete(const CString& name)
{
	auto it = in->mRank.find(name);
	if (it == in->mRank.end()) {
		return false;
	}

	in->mRank.erase(it);
	return true;
}

// すべてリセット
void CommandRanking::ResetAll()
{
	in->mRank.clear();
}

CommandRanking* CommandRanking::CloneTemporarily()
{
	auto newObj = new CommandRanking();
	newObj->in->mRank = in->mRank;
	newObj->in->mFilePath = in->mFilePath;
	newObj->in->mIsLoaded = in->mIsLoaded;
	newObj->in->mIsTemporary = true;

	return newObj;
}
void CommandRanking::CopyTo(CommandRanking* dst)
{
	dst->in->mRank = in->mRank;
	dst->in->mFilePath = in->mFilePath;
	dst->in->mIsLoaded = in->mIsLoaded;
}

void CommandRanking::Release()
{
	if (in->mIsTemporary) {
		delete this;
	}
}

}
}
}

