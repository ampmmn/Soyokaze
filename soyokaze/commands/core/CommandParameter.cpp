#include "pch.h"
#include "framework.h"
#include "CommandParameter.h"
#include "commands/core/CommandParameterIF.h"
#include "commands/core/IFIDDefine.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace core {

static void GetParameters(
	const CString& paramStr,
 	std::vector<CString>& args
)
{
	CString s = paramStr;
	s.Trim();
	if (s.IsEmpty()) {
		// 引数なし
		args.clear();
		return ;
	}

	std::vector<CString> argsTmp;

	int startPos = -1;
	int len = s.GetLength();

	for (int i = 0; i < len; ++i) {

		TCHAR c = s[i];

		if (c == _T(' ')) {
			continue;
		}

		if (c == _T('"')) {
			startPos = i + 1;

			// "あり
			for (int j = i + 1; j < len; ++j) {

				c = s[j];

				if (c == _T('"')) {
					// "終端
					argsTmp.push_back(s.Mid(startPos, j - startPos));
					startPos = -1;
					i = j;
					break;
				}
			}
			if (startPos != -1) {
				argsTmp.push_back(s.Mid(startPos));
				i = len;
			}
			continue;
		}

		// "なし
		startPos = i;
		for (int j = i; j < len; ++j) {

			c = s[j];

			if (c == _T(' ')) {
				// "終端
				argsTmp.push_back(s.Mid(startPos, j - startPos));
				startPos = -1;
				i = j;
				break;
			}
		}
		if (startPos != -1) {
			argsTmp.push_back(s.Mid(startPos));
			i = len;
		}
	}



	args.swap(argsTmp);
}

struct CommandParameterBuilder::PImpl
{
	PImpl() : 
		mWholeText(),
		mCommandPart(),
		mHasSpace(false)
	{
	}

	void SetParamPart(const CString& paramPart)
	{
		mParamPart = paramPart;
		mParamArray.clear();
		GetParameters(mParamPart, mParamArray);
	}

	CString mWholeText;
	CString mCommandPart;
	CString mParamPart;
	bool mHasSpace;

	std::vector<CString> mParamArray;

	std::map<CString, CString> mStrParamMap;
	std::map<CString, bool> mBoolParamMap;
	uint32_t mRefCount = 1;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CommandParameterBuilder::CommandParameterBuilder() : in(std::make_unique<PImpl>())
{
}

CommandParameterBuilder::CommandParameterBuilder(const CommandParameterBuilder& rhs) :
	in(std::make_unique<PImpl>())
{
	in->mWholeText = rhs.in->mWholeText;
	in->mCommandPart = rhs.in->mCommandPart;
	in->SetParamPart(rhs.in->mParamPart);
	in->mHasSpace = rhs.in->mHasSpace;
	in->mStrParamMap = rhs.in->mStrParamMap;
	in->mBoolParamMap = rhs.in->mBoolParamMap;
}

CommandParameterBuilder::CommandParameterBuilder(
	const CString& str
) : in(std::make_unique<PImpl>())
{
	SetWholeString(str);
}

CommandParameterBuilder::~CommandParameterBuilder()
{
}

CommandParameterBuilder& CommandParameterBuilder::operator = (const CommandParameterBuilder& rhs)
{
	if (&rhs != this) {
		in->mWholeText = rhs.in->mWholeText;
		in->mCommandPart = rhs.in->mCommandPart;
		in->SetParamPart(rhs.in->mParamPart);
		in->mHasSpace = rhs.in->mHasSpace;
		in->mStrParamMap = rhs.in->mStrParamMap;
		in->mBoolParamMap = rhs.in->mBoolParamMap;
	}
	return *this;
}

CommandParameterBuilder* CommandParameterBuilder::Create()
{
	return new CommandParameterBuilder();
}

CommandParameterBuilder* CommandParameterBuilder::Create(const CString& str)
{
	return new CommandParameterBuilder(str);
}

CommandParameterBuilder* CommandParameterBuilder::Clone_() const
{
	return new CommandParameterBuilder(*this); 
}

CommandParameterBuilder* CommandParameterBuilder::EmptyParam()
{
	static CommandParameterBuilder paramEmpty;
	return &paramEmpty;
}


bool CommandParameterBuilder::IsEmpty() const
{
	return in->mWholeText.IsEmpty() != FALSE;
}

bool CommandParameterBuilder::HasParameter() const
{
	return in->mParamPart.IsEmpty() == FALSE;
}

void CommandParameterBuilder::AddArgument(const CString& arg)
{
	if (in->mParamPart.IsEmpty() == FALSE) {
		in->SetParamPart(in->mParamPart + _T(" ") + arg);
	}
	else {
		in->SetParamPart(arg);
	}

	in->mWholeText += _T(" ");
	in->mWholeText += arg;
}

LPCTSTR CommandParameterBuilder::GetWholeString() const
{
	return in->mWholeText;
}

LPCTSTR CommandParameterBuilder::GetCommandString() const
{
	return in->mCommandPart;
}

LPCTSTR CommandParameterBuilder::GetParameterString() const
{
	return in->mParamPart;
}

void CommandParameterBuilder::CopyParamTo(CommandParameterBuilder& rhs) const
{
	rhs.in->SetParamPart(in->mParamPart);
	rhs.in->mHasSpace = in->mHasSpace;

	// 内部用のパラメータ↓はコピーしない
	//rhs.in->mStrParamMap = in->mStrParamMap;
	//rhs.in->mBoolParamMap = in->mBoolParamMap;
}

void CommandParameterBuilder::CopyNamedParamTo(CommandParameterBuilder& rhs) const
{
	// 内部用のパラメータのみコピー
	rhs.in->mStrParamMap = in->mStrParamMap;
	rhs.in->mBoolParamMap = in->mBoolParamMap;
}

int CommandParameterBuilder::GetParamCount() const
{
	return (int)in->mParamArray.size();
}

LPCTSTR CommandParameterBuilder::GetParam(int index) const
{
	return in->mParamArray[index];
}

void CommandParameterBuilder::SetWholeString(LPCTSTR str)
{
	in->mWholeText = str;

	// 与えられた文字列が絶対パスで、かつ、存在するものである場合は、
	// たとえスペースを含むとしても、パラメータとして扱わない
	if (PathIsRelative(str) == FALSE && PathFileExists(str)) {
		in->mCommandPart = str;
		in->SetParamPart(_T(""));
		in->mHasSpace = false;
		return;
	}

	// パラメータ指定部分を取り出す
	CString tmpStr(str);
	tmpStr.TrimLeft();

	int pos = 0;
	if (tmpStr[pos] == _T('"')) {
		// 対応するダブルクォーテーションが現れるまでスキップ
		pos++;
		int len = tmpStr.GetLength();
		while(pos < len) {
			if (tmpStr[pos] != _T('"')) {
				pos++;
				continue;
			}
			break;
		}
	}	

	int n = tmpStr.Find(_T(" "), pos);
	if (n > -1) {
		in->mCommandPart = tmpStr.Left(n);
		in->SetParamPart(tmpStr.Mid(n + 1));
		in->mHasSpace = true;
	}
	else {
		in->mCommandPart = str;
		in->SetParamPart(_T(""));
		in->mHasSpace = false;
	}
}

void CommandParameterBuilder::SetParameterString(LPCTSTR param)
{
	in->SetParamPart(param);
	in->mWholeText = in->mCommandPart + _T(" ") + param;
	in->mHasSpace = (param[0] != _T('\0'));
}

CommandParameter* CommandParameterBuilder::Clone() const
{
	return new CommandParameterBuilder(*this); 
}

// 補完
bool CommandParameterBuilder::ComplementCommand(
	const CString& commandName,
 	CString& comlementedStr
) const
{
	if (commandName.Find(in->mCommandPart) != 0) {
		// 前方一致でなければ補完はしない
		return false;
	}

	if (in->mHasSpace == false) {
		// パラメータ指定がなければ何もしない
		return false;
	}

	comlementedStr = commandName;
	if (in->mHasSpace) {
		comlementedStr += _T(" ");
		comlementedStr += in->mParamPart;
	}
	return true;
}

/**
 	名前付きパラメータの値の文字列長を取得する
 	@return 文字列の長さ(\0終端分含む。パラメータが存在しない場合は0)
 	@param[in] name 名前
*/
int CommandParameterBuilder::GetNamedParamStringLength(
	LPCTSTR name
) const
{
	auto itFind = in->mStrParamMap.find(name);
	if (itFind == in->mStrParamMap.end()) {
		return 0;
	}
	return itFind->second.GetLength() + 1;
}

LPCTSTR CommandParameterBuilder::GetNamedParamString(LPCTSTR name, LPTSTR buf, int bufLen) const
{
	auto itFind = in->mStrParamMap.find(name);
	if (itFind == in->mStrParamMap.end()) {
		return _T("");
	}

	_tcsncpy_s(buf, bufLen, itFind->second, bufLen);
	return buf;
}

void CommandParameterBuilder::SetNamedParamString(LPCTSTR name, LPCTSTR value)
{
	in->mStrParamMap[name] = value;
}

bool CommandParameterBuilder::GetNamedParamBool(LPCTSTR name) const
{
	auto itFind = in->mBoolParamMap.find(name);
	if (itFind == in->mBoolParamMap.end()) {
		return false;
	}
	return itFind->second;
}

void CommandParameterBuilder::SetNamedParamBool(
	LPCTSTR name,
	bool value
)
{
	in->mBoolParamMap[name] = value;
}


bool CommandParameterBuilder::QueryInterface(const IFID& ifid, void** cmd)
{
	if (ifid == IFID_COMMANDPARAMETER) {
		AddRef();
		*cmd = (launcherapp::core::CommandParameter*)this;
		return true;
	}
	else if (ifid == IFID_COMMANDNAMEDPARAMETER) {
		AddRef();
		*cmd = (launcherapp::core::CommandNamedParameter*)this;
		return true;
	}
	return false;
}
uint32_t CommandParameterBuilder::AddRef()
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t CommandParameterBuilder::Release()
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

}
}
