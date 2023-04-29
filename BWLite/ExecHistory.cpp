#include "pch.h"
#include "framework.h"
#include "ExecHistory.h"
#include "AppProfile.h"
#include <deque>

struct ExecHistory::PImpl
{
	std::deque<CString> history;
	int limit;
};


ExecHistory::ExecHistory() : in(new PImpl)
{
	// 上限
	in->limit = 256;
}

ExecHistory::~ExecHistory()
{
	delete in;
}

// 実行したコマンド文字列を追加
void ExecHistory::Add(const CString& commandStr)
{
	auto itFind = 
		std::find(in->history.begin(), in->history.end(), commandStr);
	if (itFind == in->history.end()) {

		in->history.push_back(commandStr);

		if (in->history.size() > in->limit) {
			size_t over = in->history.size() - in->limit;
			in->history.erase(in->history.begin(), in->history.begin() + over);
		}
		return;
	}
	else {
		in->history.erase(itFind);
		in->history.push_back(commandStr);
	}
}

// 指定したコマンド文字列が前回実行されたかを表す数値を取得する
size_t ExecHistory::GetOrder(const CString& commandStr)
{
	auto itFind = 
		std::find(in->history.begin(), in->history.end(), commandStr);

	if (itFind == in->history.end()) {
		return (size_t)-1;
	}

	return (size_t)((in->history.end() - itFind));
}

void ExecHistory::SetLimit(int limit)
{
	in->limit = limit;
}

void ExecHistory::Load()
{
	TCHAR path[32768];
	CAppProfile::GetDirPath(path, 32768);
	PathAppend(path, _T("history.txt"));

	CStdioFile file;
	if (file.Open(path, CFile::modeRead | CFile::shareDenyWrite) == FALSE) {
		return;
	}

	std::deque<CString> lines;

	CString strLine;
	while(file.ReadString(strLine)) {

		strLine.Trim();

		if (strLine.IsEmpty()) {
			continue;
		}

		lines.push_back(strLine);
	}
	in->history.swap(lines);
}

void ExecHistory::Save()
{
	TCHAR path[32768];
	CAppProfile::GetDirPath(path, 32768);
	PathAppend(path, _T("history.txt"));

	CStdioFile file;
	if (file.Open(path, CFile::modeWrite | CFile::modeCreate) == FALSE) {
		return;
	}

	for (const auto& cmdStr : in->history) {
		file.WriteString(cmdStr);
		file.WriteString(_T("\n"));
	}
}
