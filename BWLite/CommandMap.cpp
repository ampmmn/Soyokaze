#include "pch.h"
#include "framework.h"
#include "CommandMap.h"
#include "AboutDlg.h"
#include "AppProfile.h"
#include "commands/ShellExecCommand.h"
#include "commands/ReloadCommand.h"
#include "commands/ExitCommand.h"
#include "commands/VersionCommand.h"
#include <map>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct CommandMap::PImpl
{
	std::map<CString, Command*> commands;
};


CommandMap::CommandMap() : in(new PImpl)
{
}

CommandMap::~CommandMap()
{
	for(auto& item : in->commands) {
		delete item.second;
	}
	delete in;
}


static void TrimComment(CString& s)
{
	bool inDQ = false;

	int n = s.GetLength();
	for (int i = 0; i < n; ++i) {
		if (inDQ == false && s[i] == _T('#')) {
			s = s.Left(i);
			return;
		}

		if (inDQ == false && s[i] == _T('"')) {
			inDQ = true;
		}
		if (inDQ != true && s[i] == _T('"')) {
			inDQ = false;
		}
	}
}

BOOL CommandMap::Load()
{
	// ビルトインコマンドの登録
	in->commands[_T("reload")] = new ReloadCommand(this);
	in->commands[_T("exit")] = new ExitCommand();
	in->commands[_T("version")] = new VersionCommand();


	// 設定ファイルを読み、コマンド一覧を登録する
	TCHAR path[65536];
	CAppProfile::GetDirPath(path, 65536);
	PathAppend(path, _T("commands.ini"));

	// ファイルを読む
	CStdioFile file;
	if (file.Open(path, CFile::modeRead | CFile::shareDenyWrite) == FALSE) {
		return FALSE;
	}

	CString strCurSectionName;

	CString strCommandName;
	CString strPath;
	CString strParam;
	CString strDir;
	CString strComment;
	int nShowType = SW_NORMAL;


	CString strLine;
	while(file.ReadString(strLine)) {

		TrimComment(strLine);
		strLine.Trim();

		if (strLine.IsEmpty()) {
			continue;
		}

		if (strLine[0] == _T('[')) {
			strCurSectionName = strLine.Mid(1, strLine.GetLength()-2);

 			if (strCommandName.IsEmpty() == FALSE) {
				in->commands[strCommandName] =
				 	new ShellExecCommand(strCommandName, strPath, strParam,
					                     strDir, strComment, nShowType);
			}

			// 初期化
			strCommandName = strCurSectionName;
			strPath.Empty();
			strParam.Empty();
			strDir.Empty();
			strComment.Empty();
			nShowType = SW_NORMAL;
			continue;
		}

		int n = strLine.Find(_T('='));
		if (n == -1) {
			continue;
		}

		CString strKey = strLine.Left(n);
		strKey.Trim();

		CString strValue = strLine.Mid(n+1);
		strValue.Trim();

		if (strKey.CompareNoCase(_T("path")) == 0) {
			strPath = strValue;
		}
		else if (strKey.CompareNoCase(_T("dir")) == 0) {
			strDir = strValue;

		}
		else if (strKey.CompareNoCase(_T("comment")) == 0) {
			strComment = strValue;
		}
		else if (strKey.CompareNoCase(_T("parameter")) == 0) {
			strParam = strValue;
		}
		else if (strKey.CompareNoCase(_T("show")) == 0) {
			_stscanf_s(strValue, _T("%d"), &nShowType);
		}
	}

	if (strCommandName.IsEmpty() == FALSE && strPath.IsEmpty() == FALSE) {
		in->commands[strCommandName] = new ShellExecCommand(strCommandName, strPath, strParam, strDir, strComment, nShowType);
	}

	return TRUE;
}

Command* CommandMap::Query(const CString& strQueryStr)
{
	for (auto& item : in->commands) {
		const CString& key = item.first;
		auto& command = item.second;
		if (command->Match(strQueryStr) == FALSE) {
			continue;
		}
		return command;
	}
	// ToDo: 前方一致検索対応
	return nullptr;
}

