#include "pch.h"
#include "framework.h"
#include "CommandFile.h"
#include "commands/ShellExecCommand.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CommandFile::CommandFile()
{
}

CommandFile::~CommandFile()
{
}

void CommandFile::SetFilePath(const CString& filePath)
{
	mFilePath = filePath;
}

void CommandFile::TrimComment(CString& s)
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

bool CommandFile::Load(std::vector<Command*>& commands)
{
	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, mFilePath, _T("r,ccs=UTF-8")) != 0) {
		return false;
	}
	// ファイルを読む
	CStdioFile file(fpIn);

	CString strCurSectionName;

	CString strCommandName;
	CString strDescription;
	ShellExecCommand::ATTRIBUTE normalAttr;
	ShellExecCommand::ATTRIBUTE noParamAttr;
	int runAs = 0;

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
				auto command = new ShellExecCommand();
				command->SetName(strCommandName);
				command->SetDescription(strDescription);
				command->SetRunAs(runAs);

				if (normalAttr.mPath.IsEmpty() == FALSE) {
					command->SetAttribute(normalAttr);
				}
				if (noParamAttr.mPath.IsEmpty() == FALSE) {
					command->SetAttributeForParam0(noParamAttr);
				}

				commands.push_back(command);
			}

			// 初期化
			strCommandName = strCurSectionName;
			strDescription.Empty();
			runAs = 0;

			normalAttr = ShellExecCommand::ATTRIBUTE();
			noParamAttr = ShellExecCommand::ATTRIBUTE();
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

		if (strKey.CompareNoCase(_T("description")) == 0) {
			strDescription = strValue;
		}
		else if (strKey.CompareNoCase(_T("runas")) == 0) {
			_stscanf_s(strValue, _T("%d"), &runAs);
		}
		else if (strKey.CompareNoCase(_T("path")) == 0) {
			normalAttr.mPath = strValue;
		}
		else if (strKey.CompareNoCase(_T("dir")) == 0) {
			normalAttr.mDir = strValue;
		}
		else if (strKey.CompareNoCase(_T("parameter")) == 0) {
			normalAttr.mParam = strValue;
		}
		else if (strKey.CompareNoCase(_T("show")) == 0) {
			_stscanf_s(strValue, _T("%d"), &normalAttr.mShowType);
		}
		else if (strKey.CompareNoCase(_T("path0")) == 0) {
			noParamAttr.mPath = strValue;
		}
		else if (strKey.CompareNoCase(_T("dir0")) == 0) {
			noParamAttr.mDir = strValue;
		}
		else if (strKey.CompareNoCase(_T("parameter0")) == 0) {
			noParamAttr.mParam = strValue;
		}
		else if (strKey.CompareNoCase(_T("show0")) == 0) {
			_stscanf_s(strValue, _T("%d"), &noParamAttr.mShowType);
		}
	}

	if (strCommandName.IsEmpty() == FALSE) {
		auto command = new ShellExecCommand();
		command->SetName(strCommandName);
		command->SetDescription(strDescription);
		command->SetRunAs(runAs);

		if (normalAttr.mPath.IsEmpty() == FALSE) {
			command->SetAttribute(normalAttr);
		}
		if (noParamAttr.mPath.IsEmpty() == FALSE) {
			command->SetAttributeForParam0(noParamAttr);
		}

		commands.push_back(command);
	}

	file.Close();
	fclose(fpIn);

	return true;
}

static void WriteLine(CStdioFile& file, LPTSTR name, LPCTSTR value)
{
	file.WriteString(name);
	file.WriteString(_T("="));
	file.WriteString(value);
	file.WriteString(_T("\n"));
}

static void WriteLine(CStdioFile& file, LPTSTR name, int value)
{
	file.WriteString(name);
	file.WriteString(_T("="));

	CString val;
	val.Format(_T("%d"), value);
	file.WriteString(val);
	file.WriteString(_T("\n"));
}

bool CommandFile::Save(const std::vector<Command*>& commands)
{
	FILE* fpOut = nullptr;
	try {
		CString filePathTmp = mFilePath + _T(".tmp");

		if (_tfopen_s(&fpOut, filePathTmp, _T("w,ccs=UTF-8")) != 0) {
			return false;
		}
		CStdioFile file(fpOut);

		for (auto cmdAbs : commands) {

			// ToDo: 設計見直し
			ShellExecCommand* cmd = (ShellExecCommand*)cmdAbs;

			file.WriteString(_T("["));
			file.WriteString(cmd->GetName());
			file.WriteString(_T("]\n"));

			WriteLine(file, _T("description"), cmd->GetDescription());
			WriteLine(file, _T("runas"), cmd->GetRunAs());

			ShellExecCommand::ATTRIBUTE normalAttr;
			cmd->GetAttribute(normalAttr);
			WriteLine(file, _T("path"), normalAttr.mPath);
			WriteLine(file, _T("dir"), normalAttr.mDir);
			WriteLine(file, _T("parameter"), normalAttr.mParam);
			WriteLine(file, _T("show"), normalAttr.mShowType);

			ShellExecCommand::ATTRIBUTE param0Attr;
			cmd->GetAttributeForParam0(param0Attr);
			WriteLine(file, _T("path0"), param0Attr.mPath);
			WriteLine(file, _T("dir0"), param0Attr.mDir);
			WriteLine(file, _T("parameter0"), param0Attr.mParam);
			WriteLine(file, _T("show0"), param0Attr.mShowType);

			file.WriteString(_T("\n"));
		}

		file.Close();
		fclose(fpOut);

		// 最後に一時ファイルを書き戻す
		if (CopyFile(filePathTmp, mFilePath, FALSE) == FALSE) {
			return false;
		}

		// 一時ファイルを消す
		DeleteFile(filePathTmp);

		return true;
	}
	catch(CFileException* e) {
		e->Delete();
		fclose(fpOut);
		return false;
	}
}


