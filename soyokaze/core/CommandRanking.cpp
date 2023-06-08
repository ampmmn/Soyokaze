#include "pch.h"
#include "core/CommandRanking.h"
#include "utility/AppProfile.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace core {

struct CommandRanking::PImpl
{
	const CString& GetFilePath()
	{
		TCHAR path[MAX_PATH_NTFS];
		CAppProfile::GetDirPath(path, MAX_PATH_NTFS);
		PathAppend(path, _T("usage.dat"));

		mFilePath = path;

		return mFilePath;
	}

	// コマンドごとのランクを保持するマップ
	std::map<CString, int> mRank;

	// 設定ファイル保存先パス
	CString mFilePath;
};

CommandRanking::CommandRanking() : in(new PImpl)
{
}

CommandRanking::~CommandRanking()
{
}

bool CommandRanking::Load()
{
	const CString& filePath = in->GetFilePath();

	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, filePath, _T("r,ccs=UTF-8")) != 0) {
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
	const CString& filePath = in->GetFilePath();
	FILE* fpOut = nullptr;
	try {
		CString filePathTmp = filePath + _T(".tmp");

		if (_tfopen_s(&fpOut, filePathTmp, _T("w,ccs=UTF-8")) != 0) {
			return false;
		}
		CStdioFile file(fpOut);

		CString lineBuff;
		for (auto elem : in->mRank) {

			const CString& name = elem.first;
			int rank = elem.second;

			lineBuff.Format(_T("%s\t%d\n"), name, rank);
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

}
}

