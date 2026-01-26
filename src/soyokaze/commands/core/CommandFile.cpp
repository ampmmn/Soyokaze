#include "pch.h"
#include "framework.h"
#include "commands/core/CommandFile.h"
#include "commands/core/CommandFileEntry.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/Base64.h"
#include <wincrypt.h>
#include <map>
#include <set>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace utility::base64;

using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;

static CString UnescapeString(const CString& s)
{
	CString ret(s);
	ret.Replace(_T("%0D"), _T("\r"));
	ret.Replace(_T("%0A"), _T("\n"));
	return ret;
}



struct CommandFile::PImpl
{
	// ファイルのパス
	CString mFilePath;

	// エントリのリスト
	std::vector<RefPtr<CommandFileEntry> > mEntries;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CommandFile::CommandFile() : in(std::make_unique<PImpl>())
{
}

CommandFile::~CommandFile()
{
	in->mEntries.clear();
}

void CommandFile::SetFilePath(const CString& filePath)
{
	in->mFilePath = filePath;
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

int CommandFile::GetEntryCount() const
{
	return (int)in->mEntries.size();
}

CommandFile::Entry* CommandFile::NewEntry(
	const CString& name
)
{
	auto entry = new CommandFileEntry();
	entry->SetName(name);

	in->mEntries.push_back(RefPtr(entry));
	return entry;
}


CommandFile::Entry*
CommandFile::GetEntry(int index) const
{
	ASSERT(0 <= index && index < (int)in->mEntries.size());
	return in->mEntries[index].get();
}

CString CommandFile::GetName(Entry* entry)
{
	ASSERT(entry);
	return entry->GetName();
}

void CommandFile::MarkAsUsed(Entry* entry)
{
	ASSERT(entry);
	entry->MarkAsUsed();
}

bool CommandFile::IsUsedEntry(Entry* entry)
{
	ASSERT(entry);
	return entry->IsUsedEntry();
}

bool CommandFile::HasValue(Entry* entry, LPCTSTR key)
{
	ASSERT(entry);
	return entry->HasValue(key);
}


int CommandFile::GetValueType(Entry* entry, LPCTSTR key)
{
	ASSERT(entry);
	return entry->GetValueType(key);
}


int CommandFile::Get(Entry* entry, LPCTSTR key, int defValue)
{
	ASSERT(entry);
	return entry->Get(key, defValue);
}

void CommandFile::Set(Entry* entry, LPCTSTR key, int value)
{
	ASSERT(entry);
	entry->Set(key, value);
}


double CommandFile::Get(Entry* entry, LPCTSTR key, double defValue)
{
	ASSERT(entry);
	return entry->Get(key, defValue);
}

void CommandFile::Set(Entry* entry, LPCTSTR key, double value)
{
	ASSERT(entry);
	entry->Set(key, value);
}


CString CommandFile::Get(Entry* entry, LPCTSTR key, LPCTSTR defValue)
{
	ASSERT(entry);
	return entry->Get(key, defValue);
}

void CommandFile::Set(Entry* entry, LPCTSTR key, const CString& value)
{
	ASSERT(entry);
	entry->Set(key, value);
}


bool CommandFile::Get(Entry* entry, LPCTSTR key, bool defValue)
{
	ASSERT(entry);
	return entry->Get(key, defValue);
}

void CommandFile::Set(Entry* entry, LPCTSTR key, bool value)
{
	ASSERT(entry);
	entry->Set(key, value);
}

bool CommandFile::Get(Entry* entry, LPCTSTR key, std::vector<uint8_t>& value)
{
	ASSERT(entry);
	size_t len = entry->GetBytesLength(key);
	if (len != CommandEntryIF::NO_ENTRY) {
		value.resize(len);
		return entry->GetBytes(key, value.data(), len);
	}
	else {
		value.clear();
		return false;
	}
}

void CommandFile::Set(Entry* entry, LPCTSTR key, const std::vector<uint8_t>& value)
{
	ASSERT(entry);
	entry->SetBytes(key, value.data(), value.size());
}

bool CommandFile::Load()
{
	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, in->mFilePath, _T("r,ccs=UTF-8")) != 0 || fpIn == nullptr) {
		return false;
	}
	// ファイルを読む
	CStdioFile file(fpIn);

	CommandFileEntry* curEntry = nullptr;

	std::vector<RefPtr<CommandFileEntry> > entries;

	static tregex regInt(_T("^ *-?[0-9]+ *$"));
	static tregex regDouble(_T("^ *-?[0-9]+\\.[0-9]+ *$"));

	CString strLine;
	while(file.ReadString(strLine)) {

		TrimComment(strLine);
		strLine.Trim();

		if (strLine.IsEmpty()) {
			continue;
		}

		if (strLine[0] == _T('[')) {

			if (curEntry != nullptr) {
				entries.push_back(RefPtr(curEntry));
				curEntry = nullptr;
			}

			CString strCurSectionName = strLine.Mid(1, strLine.GetLength()-2);

			curEntry = new CommandFileEntry();
			curEntry->SetName(strCurSectionName);
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
			curEntry->Set(strKey, true);
		}
		else if (strValue== _T("false") || strValue== _T("second")) {  // second:初期実装時のバグのリカバーのための処理
			curEntry->Set(strKey, false);
		}
		else if (strValue.Left(7) == _T("stream:")) {
			std::vector<uint8_t> stm;
			DecodeBase64(strValue.Mid(7), stm);
			curEntry->SetBytes(strKey, stm.data(),stm.size());
		}
		else if (std::regex_match(pat, regDouble)) {
			double value;
			_stscanf_s(strValue, _T("%lg"), &value);
			curEntry->Set(strKey, value);
		}
		else if (std::regex_match(pat, regInt)) {
			int value;
			_stscanf_s(strValue, _T("%d"), &value);
			curEntry->Set(strKey, value);
		}
		else {
			if (strValue.Left(1) == _T('"') && strValue.Right(1) == _T('"')) {
				strValue = strValue.Mid(1, strValue.GetLength()-2);
			}
			else if (strValue.Left(1) == _T('\'') && strValue.Right(1) == _T('\'')) {
				strValue = strValue.Mid(1, strValue.GetLength()-2);
			}
			curEntry->Set(strKey, UnescapeString(strValue));
		}
	}

	if (curEntry) {
		entries.push_back(RefPtr(curEntry));
	}

	file.Close();
	fclose(fpIn);

	in->mEntries.swap(entries);
	return true;
}


bool CommandFile::Save()
{
	FILE* fpOut = nullptr;
	try {
		CString filePathTmp = in->mFilePath + _T(".tmp");

		if (_tfopen_s(&fpOut, filePathTmp, _T("w,ccs=UTF-8")) != 0 || fpOut == nullptr) {
			return false;
		}
		CStdioFile file(fpOut);

		for (auto& entry :  in->mEntries) {
			// FIXME: SaveとLoadで処理の形が非対称なのが気に入らない..
			entry->Save(file);
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

