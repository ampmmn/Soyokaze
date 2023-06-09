#include "pch.h"
#include "framework.h"
#include "CommandFile.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include <map>
#include <set>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;


class CommandFile::Entry
{
public:
	Entry() : mIsUsed(false)
	{
	}


	CString mName;
	bool mIsUsed;

	std::map<CString, int> mTypeMap;

	std::map<CString, int> mIntMap;
	std::map<CString, CString> mStrMap;
	std::map<CString, bool> mBoolMap;
	std::map<CString, double> mDoubleMap;
};


struct CommandFile::PImpl
{
	// ファイルのパス
	CString mFilePath;

	// エントリのリスト
	std::vector<CommandFile::Entry*> mEntries;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CommandFile::CommandFile() : in(new PImpl)
{
}

CommandFile::~CommandFile()
{
	ClearEntries();
}

void CommandFile::SetFilePath(const CString& filePath)
{
	in->mFilePath = filePath;
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

bool CommandFile::Load(std::vector<soyokaze::core::Command*>& commands)
{
	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, in->mFilePath, _T("r,ccs=UTF-8")) != 0) {
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
				auto command = std::unique_ptr<ShellExecCommand>(new ShellExecCommand());
				command->SetName(strCommandName);
				command->SetDescription(strDescription);
				command->SetRunAs(runAs);

				if (normalAttr.mPath.IsEmpty() == FALSE) {
					command->SetAttribute(normalAttr);
				}
				if (noParamAttr.mPath.IsEmpty() == FALSE) {
					command->SetAttributeForParam0(noParamAttr);
				}

				commands.push_back(command.release());
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
		auto command = std::unique_ptr<ShellExecCommand>(new ShellExecCommand());
		command->SetName(strCommandName);
		command->SetDescription(strDescription);
		command->SetRunAs(runAs);

		if (normalAttr.mPath.IsEmpty() == FALSE) {
			command->SetAttribute(normalAttr);
		}
		if (noParamAttr.mPath.IsEmpty() == FALSE) {
			command->SetAttributeForParam0(noParamAttr);
		}

		commands.push_back(command.release());
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

static void WriteLine(CStdioFile& file, LPTSTR name, double value)
{
	file.WriteString(name);
	file.WriteString(_T("="));

	CString val;
	val.Format(_T("%lg"), value);
	file.WriteString(val);
	file.WriteString(_T("\n"));
}

bool CommandFile::Save(const std::vector<soyokaze::core::Command*>& commands)
{
	FILE* fpOut = nullptr;
	try {
		CString filePathTmp = in->mFilePath + _T(".tmp");

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
		if (CopyFile(filePathTmp, in->mFilePath, FALSE) == FALSE) {
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


int CommandFile::GetEntryCount() const
{
	return (int)in->mEntries.size();
}

CommandFile::Entry* CommandFile::NewEntry(
	const CString& name
)
{
	auto entry = new Entry();
	entry->mName = name;
	in->mEntries.push_back(entry);
	return entry;
}


CommandFile::Entry*
CommandFile::GetEntry(int index) const
{
	ASSERT(0 <= index && index < (int)in->mEntries.size());
	return in->mEntries[index];
}

CString CommandFile::GetName(Entry* entry) const
{
	ASSERT(entry);
	return entry->mName;
}

void CommandFile::MarkAsUsed(Entry* entry)
{
	ASSERT(entry);
	entry->mIsUsed = true;
}

bool CommandFile::IsUsedEntry(Entry* entry) const
{
	ASSERT(entry);
	return entry->mIsUsed;
}

bool CommandFile::HasValue(Entry* entry, LPCTSTR key) const
{
	ASSERT(entry);
	return GetValueType(entry, key) != TYPE_UNKNOWN;
}


int CommandFile::GetValueType(Entry* entry, LPCTSTR key) const
{
	ASSERT(entry);
	auto itFind = entry->mTypeMap.find(key);
	return itFind == entry->mTypeMap.end() ? TYPE_UNKNOWN : itFind->second;
}


int CommandFile::Get(Entry* entry, LPCTSTR key, int defValue) const
{
	ASSERT(entry);
	auto itFind = entry->mIntMap.find(key);
	if (itFind == entry->mIntMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void CommandFile::Set(Entry* entry, LPCTSTR key, int value)
{
	ASSERT(entry);
	entry->mIntMap[key] = value;
	entry->mTypeMap[key] = TYPE_INT;
}


double CommandFile::Get(Entry* entry, LPCTSTR key, double defValue) const
{
	ASSERT(entry);
	ASSERT(entry);
	auto itFind = entry->mDoubleMap.find(key);
	if (itFind == entry->mDoubleMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void CommandFile::Set(Entry* entry, LPCTSTR key, double value)
{
	ASSERT(entry);
	entry->mDoubleMap[key] = value;
	entry->mTypeMap[key] = TYPE_DOUBLE;
}


CString CommandFile::Get(Entry* entry, LPCTSTR key, LPCTSTR defValue) const
{
	ASSERT(entry);
	auto itFind = entry->mStrMap.find(key);
	if (itFind == entry->mStrMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void CommandFile::Set(Entry* entry, LPCTSTR key, const CString& value)
{
	ASSERT(entry);
	entry->mStrMap[key] = value;
	entry->mTypeMap[key] = TYPE_STRING;
}


bool CommandFile::Get(Entry* entry, LPCTSTR key, bool defValue) const
{
	ASSERT(entry);
	auto itFind = entry->mBoolMap.find(key);
	if (itFind == entry->mBoolMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void CommandFile::Set(Entry* entry, LPCTSTR key, bool value)
{
	ASSERT(entry);
	entry->mBoolMap[key] = value;
	entry->mTypeMap[key] = TYPE_BOOLEAN;
}

void CommandFile::ClearEntries()
{
	for (auto entry : in->mEntries) {
		delete entry;
	}
	in->mEntries.clear();
}

bool CommandFile::Load()
{
	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, in->mFilePath, _T("r,ccs=UTF-8")) != 0) {
		return false;
	}
	// ファイルを読む
	CStdioFile file(fpIn);

	std::unique_ptr<Entry> curEntry;

	std::vector<Entry*> entries;

	tregex regInt(_T("^ *-?[0-9]+ *$"));
	tregex regDouble(_T("^ *-?[0-9]+\\.[0-9]+ *$"));

	CString strLine;
	while(file.ReadString(strLine)) {

		TrimComment(strLine);
		strLine.Trim();

		if (strLine.IsEmpty()) {
			continue;
		}

		if (strLine[0] == _T('[')) {

			if (curEntry != nullptr) {
				entries.push_back(curEntry.release());
			}

			CString strCurSectionName = strLine.Mid(1, strLine.GetLength()-2);

			curEntry.reset(new Entry);
			curEntry->mName =strCurSectionName;
			continue;
		}

		int n = strLine.Find(_T('='));
		if (n == -1) {
			continue;
		}

		if (curEntry == nullptr) {
			continue;
		}

		CString strKey = strLine.Left(n);
		strKey.Trim();

		CString strValue = strLine.Mid(n+1);
		strValue.Trim();
		tstring pat(strValue);

		if (strValue== _T("true")) {
			curEntry->mBoolMap[strKey] = true;
			curEntry->mTypeMap[strKey] = TYPE_BOOLEAN;
		}
		else if (strValue== _T("false")) {
			curEntry->mBoolMap[strKey] = false;
			curEntry->mTypeMap[strKey] = TYPE_BOOLEAN;
		}
		else if (std::regex_match(pat, regDouble)) {
			double value;
			_stscanf_s(strValue, _T("%lg"), &value);
			curEntry->mDoubleMap[strKey] = value;
			curEntry->mTypeMap[strKey] = TYPE_DOUBLE;
		}
		else if (std::regex_match(pat, regInt)) {
			int value;
			_stscanf_s(strValue, _T("%d"), &value);
			curEntry->mIntMap[strKey] = value;
			curEntry->mTypeMap[strKey] = TYPE_INT;
		}
		else {
			if (strValue.Left(1) == _T('"') && strValue.Right(1) == _T('"')) {
				strValue = strValue.Mid(1, strValue.GetLength()-2);
			}
			else if (strValue.Left(1) == _T('\'') && strValue.Right(1) == _T('\'')) {
				strValue = strValue.Mid(1, strValue.GetLength()-2);
			}
			curEntry->mStrMap[strKey] = strValue;
			curEntry->mTypeMap[strKey] = TYPE_STRING;
		}
	}

	if (curEntry) {
		entries.push_back(curEntry.release());
	}

	file.Close();
	fclose(fpIn);

	in->mEntries.swap(entries);

	for (auto entry : entries) {
		delete entry;
	}

	return true;
}


bool CommandFile::Save()
{
	FILE* fpOut = nullptr;
	try {
		CString filePathTmp = in->mFilePath + _T(".tmp");

		if (_tfopen_s(&fpOut, filePathTmp, _T("w,ccs=UTF-8")) != 0) {
			return false;
		}
		CStdioFile file(fpOut);

		for (auto entry : in->mEntries) {

			file.WriteString(_T("["));
			file.WriteString(entry->mName);
			file.WriteString(_T("]\n"));

			for (auto kv : entry->mIntMap) {
				file.WriteString(kv.first);
				file.WriteString(_T("="));

				TCHAR val[256];
				_stprintf_s(val, _T("%d"), kv.second);
				file.WriteString(val);
				file.WriteString(_T("\n"));
			}
			for (auto kv : entry->mDoubleMap) {
				file.WriteString(kv.first);
				file.WriteString(_T("="));

				TCHAR val[256];
				_stprintf_s(val, _T("%lg"), kv.second);
				file.WriteString(val);
				file.WriteString(_T("\n"));
			}
			for (auto kv : entry->mStrMap) {
				file.WriteString(kv.first);
				file.WriteString(_T("=\""));
				file.WriteString(kv.second);
				file.WriteString(_T("\"\n"));
			}
			for (auto kv : entry->mBoolMap) {
				file.WriteString(kv.first);
				file.WriteString(_T("="));
				file.WriteString(kv.second ? _T("true") : _T("second"));
				file.WriteString(_T("\n"));
			}

			file.WriteString(_T("\n"));
		}

		file.Close();
		fclose(fpOut);

		// 最後に一時ファイルを書き戻す
		if (CopyFile(filePathTmp, in->mFilePath, FALSE) == FALSE) {
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

