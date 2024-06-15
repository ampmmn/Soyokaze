#include "pch.h"
#include "CommandFileEntry.h"
#include <map>
#include <wincrypt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static CString EscapeString(const CString& s)
{
	CString ret(s);
	ret.Replace(_T("\r"), _T("%0D"));
	ret.Replace(_T("\n"), _T("%0A"));
	return ret;
}

static CString EncodeBase64(const std::vector<uint8_t>& stm)
{
	// 長さを調べる
	DWORD dstLen = 0;
	if (stm.size() == 0) {
		return CString();
	}
	CryptBinaryToString( &stm.front(), (int)stm.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &dstLen);

	// 変換する
	CString dstStr;
	CryptBinaryToString( &stm.front(), (int)stm.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, dstStr.GetBuffer(dstLen+1), &dstLen );
	dstStr.ReleaseBuffer();

	return dstStr;
}

enum {
	TYPE_INT,
	TYPE_DOUBLE,
	TYPE_STRING,
	TYPE_BOOLEAN,
	TYPE_STREAM,
	TYPE_UNKNOWN,
};

struct CommandFileEntry::PImpl
{
	CString mName;
	bool mIsUsed = false;

	std::map<CString, int> mTypeMap;

	std::map<CString, int> mIntMap;
	std::map<CString, CString> mStrMap;
	std::map<CString, bool> mBoolMap;
	std::map<CString, double> mDoubleMap;
	std::map<CString, std::vector<uint8_t> > mStreamMap;
};

CommandFileEntry::CommandFileEntry() : in(new PImpl)
{
}

CommandFileEntry::~CommandFileEntry()
{
}

void CommandFileEntry::Save(CStdioFile& file)
{
	file.WriteString(_T("["));
	file.WriteString(GetName());
	file.WriteString(_T("]\n"));

	for (auto& kv : in->mIntMap) {
		file.WriteString(kv.first);
		file.WriteString(_T("="));

		TCHAR val[256];
		_stprintf_s(val, _T("%d"), kv.second);
		file.WriteString(val);
		file.WriteString(_T("\n"));
	}
	for (auto& kv : in->mDoubleMap) {
		file.WriteString(kv.first);
		file.WriteString(_T("="));

		TCHAR val[256];
		_stprintf_s(val, _T("%lg"), kv.second);
		file.WriteString(val);
		file.WriteString(_T("\n"));
	}
	for (auto& kv : in->mStrMap) {
		file.WriteString(kv.first);
		file.WriteString(_T("=\""));
		file.WriteString(EscapeString(kv.second));
		file.WriteString(_T("\"\n"));
	}
	for (auto& kv : in->mBoolMap) {
		file.WriteString(kv.first);
		file.WriteString(_T("="));
		file.WriteString(kv.second ? _T("true") : _T("false"));
		file.WriteString(_T("\n"));
	}
	for (auto& kv : in->mStreamMap) {
		file.WriteString(kv.first);
		file.WriteString(_T("=stream:"));
		file.WriteString(EncodeBase64(kv.second));
		file.WriteString(_T("\n"));
	}
	file.WriteString(_T("\n"));
}

void CommandFileEntry::SetName(LPCTSTR name)
{
	in->mName = name;
}

CString CommandFileEntry::GetName()
{
	return in->mName;
}

void CommandFileEntry::MarkAsUsed()
{
	in->mIsUsed = true;
}

bool CommandFileEntry::IsUsedEntry()
{
	return in->mIsUsed;
}

bool CommandFileEntry::HasValue(LPCTSTR key)
{
	return GetValueType(key) != TYPE_UNKNOWN;
}

int CommandFileEntry::GetValueType(LPCTSTR key)
{
	auto itFind = in->mTypeMap.find(key);
	return itFind == in->mTypeMap.end() ? TYPE_UNKNOWN : itFind->second;
}

int CommandFileEntry::Get(LPCTSTR key, int defValue)
{
	auto itFind = in->mIntMap.find(key);
	if (itFind == in->mIntMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void CommandFileEntry::Set(LPCTSTR key, int value)
{
	in->mIntMap[key] = value;
	in->mTypeMap[key] = TYPE_INT;
}

double CommandFileEntry::Get(LPCTSTR key, double defValue)
{
	auto itFind = in->mDoubleMap.find(key);
	if (itFind == in->mDoubleMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void CommandFileEntry::Set(LPCTSTR key, double value)
{
	in->mDoubleMap[key] = value;
	in->mTypeMap[key] = TYPE_DOUBLE;
}

CString CommandFileEntry::Get(LPCTSTR key, LPCTSTR defValue)
{
	auto itFind = in->mStrMap.find(key);
	if (itFind == in->mStrMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void CommandFileEntry::Set(LPCTSTR key, const CString& value)
{
	in->mStrMap[key] = value;
	in->mTypeMap[key] = TYPE_STRING;
}

bool CommandFileEntry::Get(LPCTSTR key, bool defValue)
{
	auto itFind = in->mBoolMap.find(key);
	if (itFind == in->mBoolMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void CommandFileEntry::Set(LPCTSTR key, bool value)
{
	in->mBoolMap[key] = value;
	in->mTypeMap[key] = TYPE_BOOLEAN;
}

bool CommandFileEntry::Get(LPCTSTR key, std::vector<uint8_t>& value)
{
	auto itFind = in->mStreamMap.find(key);
	if (itFind == in->mStreamMap.end()) {
		return false;
	}

	value = itFind->second;
	return true;
}

void CommandFileEntry::Set(LPCTSTR key, const std::vector<uint8_t>& value)
{
	in->mStreamMap[key] = value;
	in->mTypeMap[key] = TYPE_STREAM;
}


