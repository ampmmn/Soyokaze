#include "pch.h"
#include "framework.h"
#include "CommandParameter.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace core {

struct CommandParameter::PImpl
{
	PImpl() : 
		mWholeText(),
		mCommandPart(),
		mHasSpace(false)
	{
	}

	PImpl(const CString& str) : 
		mWholeText(str),
		mCommandPart(str),
		mHasSpace(false)
	{
	}

	CString mWholeText;
	CString mCommandPart;
	CString mParamPart;
	bool mHasSpace;

	std::map<CString, CString> mStrParamMap;
	std::map<CString, bool> mBoolParamMap;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CommandParameter::CommandParameter(): in(new PImpl)
{
}


CommandParameter::CommandParameter(
	const CString& str
) : in(new PImpl(str))
{
	CString tmpStr = str;
	int n = tmpStr.Find(_T(" "));
	if (n > -1) {
		in->mCommandPart = tmpStr.Left(n);
		in->mParamPart = tmpStr.Mid(n + 1);
		in->mHasSpace = true;
	}
}

CommandParameter::~CommandParameter()
{
}

const CString& CommandParameter::GetCommandString() const
{
	return in->mCommandPart;
}

const CString& CommandParameter::GetParameterString() const
{
	return in->mParamPart;
}

void CommandParameter::GetParameters(std::vector<CString>& args) const
{
	CString s = in->mParamPart;
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


bool CommandParameter::GetNamedParam(
	LPCTSTR name,
	CString* value
) const
{
	auto& it = in->mStrParamMap.find(name);
	if (it == in->mStrParamMap.end()) {
		return false;
	}
	if (value) {
		*value = it->second;
	}
	return true;
}

void CommandParameter::AppendParameterPart(CString& str)
{
	if (in->mHasSpace) {
		str += _T(" ");
		str += in->mParamPart;
	}
}

// 補完
bool CommandParameter::ComplementCommand(
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

CString CommandParameter::GetNamedParamString(LPCTSTR name) const
{
	auto itFind = in->mStrParamMap.find(name);
	if (itFind == in->mStrParamMap.end()) {
		return _T("");
	}
	return itFind->second;
}

void CommandParameter::SetNamedParamString(LPCTSTR name, LPCTSTR value)
{
	in->mStrParamMap[name] = value;
}

bool CommandParameter::GetNamedParamBool(LPCTSTR name) const
{
	auto itFind = in->mBoolParamMap.find(name);
	if (itFind == in->mBoolParamMap.end()) {
		return false;
	}
	return itFind->second;
}

void CommandParameter::SetNamedParamBool(
	LPCTSTR name,
	bool value
)
{
	in->mBoolParamMap[name] = value;
}

}
}
