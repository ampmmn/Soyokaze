#include "pch.h"
#include "framework.h"
#include "CommandString.h"

#pragma comment(lib, "version.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CommandString::CommandString(const CString& str) : 
	mWholeText(str),
	mCommandPart(str),
	mHasSpace(false)
{
	CString tmpStr = str;
	int n = tmpStr.Find(_T(" "));
	if (n > -1) {
		mCommandPart = tmpStr.Left(n);
		mParamPart = tmpStr.Mid(n + 1);
		mHasSpace = true;
	}
}

CommandString::~CommandString()
{
}

const CString& CommandString::GetCommandString()
{
	return mCommandPart;
}

const CString& CommandString::GetParameterString()
{
	return mParamPart;
}

void CommandString::GetParameters(std::vector<CString>& args)
{
	CString s = mParamPart;
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

void CommandString::AppendParameterPart(CString& str)
{
	if (mHasSpace) {
		str += _T(" ");
		str += mParamPart;
	}
}

// 補完
bool CommandString::ComplementCommand(
	const CString& commandName,
 	CString& comlementedStr
)
{
	if (commandName.Find(mCommandPart) != 0) {
		// 前方一致でなければ補完はしない
		return false;
	}

	if (mHasSpace == false) {
		// パラメータ指定がなければ何もしない
		return false;
	}

	comlementedStr = commandName;
	if (mHasSpace) {
		comlementedStr += _T(" ");
		comlementedStr += mParamPart;
	}
	return true;
}
