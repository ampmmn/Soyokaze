#include "pch.h"
#include "framework.h"
#include "CommandMap.h"
#include "AboutDlg.h"
#include <map>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Builtin Commands

class ExecCommand : public Command
{
public:
	ExecCommand(const CString& name, const CString& path, const CString& param, const CString& dir, const CString& comment) :
	m_strName(name), m_strPath(path), m_strParam(param), m_strDir(dir), m_strComment(comment)
	{
	}

	virtual CString GetDescription()
	{
		return m_strComment.IsEmpty() ? m_strName : m_strComment;
	}
	virtual BOOL Execute()
	{
		SHELLEXECUTEINFO si = SHELLEXECUTEINFO();
		si.cbSize = sizeof(si);
		si.nShow = SW_NORMAL;
		si.fMask = SEE_MASK_NOCLOSEPROCESS;
		si.lpFile = m_strPath;
		if (m_strParam.IsEmpty() == FALSE) {
			si.lpParameters = m_strParam;
		}
		if (m_strDir.IsEmpty() == FALSE) {
			si.lpDirectory = m_strDir;
		}
		BOOL bRun = ShellExecuteEx(&si);
		if (bRun == FALSE) {
			return FALSE;
		}

		CloseHandle(si.hProcess);

		return TRUE;
	}
	virtual CString GetErrorString()
	{
		return _T("");
	}
	virtual BOOL Match(const CString& strQueryStr)
	{
		// 前方一致検索
		return m_strName.Find(strQueryStr) == 0;
	}

	CString m_strName;
	CString m_strPath;
	CString m_strParam;
	CString m_strDir;
	CString m_strComment;
};

class ReloadCommand : public Command
{
public:
	ReloadCommand(CommandMap* pMap) : m_pCommandMap(pMap)
	{
	}

	virtual CString GetDescription()
	{
		return _T("設定のリロード");
	}
	virtual BOOL Execute()
	{
		return m_pCommandMap->Load();
	}
	virtual CString GetErrorString()
	{
		return _T("");
	}
	virtual BOOL Match(const CString& strQueryStr)
	{
		// 完全一致比較
		return strQueryStr == _T("reload");
	}


	CommandMap* m_pCommandMap;
};

class ExitCommand : public Command
{
public:
	ExitCommand()
	{
	}

	virtual CString GetDescription()
	{
		return _T("【終了】");
	}
	virtual BOOL Execute()
	{
		PostQuitMessage(0);
		return TRUE;
	}
	virtual CString GetErrorString()
	{
		return _T("");
	}
	virtual BOOL Match(const CString& strQueryStr)
	{
		// 完全一致比較
		return strQueryStr == _T("exit");
	}

	CommandMap* m_pCommandMap;
};

class VersionCommand : public Command
{
public:
	VersionCommand()
	{
	}

	virtual CString GetDescription()
	{
		return _T("【バージョン情報】");
	}
	virtual BOOL Execute()
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return TRUE;
	}
	virtual CString GetErrorString()
	{
		return _T("");
	}
	virtual BOOL Match(const CString& strQueryStr)
	{
		// 完全一致比較
		return strQueryStr == _T("version");
	}

	CommandMap* m_pCommandMap;
};


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
	GetModuleFileName(NULL, path, 65536);
	PathRemoveFileSpec(path);
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
				in->commands[strCommandName] = new ExecCommand(strCommandName, strPath, strParam, strDir, strComment);
			}

			// 初期化
			strCommandName = strCurSectionName;
			strPath.Empty();
			strParam.Empty();
			strDir.Empty();
			strComment.Empty();
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
	}

	if (strCommandName.IsEmpty() == FALSE && strPath.IsEmpty() == FALSE) {
		in->commands[strCommandName] = new ExecCommand(strCommandName, strPath, strParam, strDir, strComment);
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

