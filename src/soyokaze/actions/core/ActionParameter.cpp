#include "pch.h"
#include "framework.h"
#include "ActionParameter.h"
#include "core/IFIDDefine.h"
#include "utility/Path.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using IFID = launcherapp::core::IFID;

namespace launcherapp { namespace actions { namespace core {

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

struct ParameterBuilder::PImpl
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
	uint32_t mRefCount{1};
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ParameterBuilder::ParameterBuilder() : in(std::make_unique<PImpl>())
{
}

ParameterBuilder::ParameterBuilder(const ParameterBuilder& rhs) :
	in(std::make_unique<PImpl>())
{
	in->mWholeText = rhs.in->mWholeText;
	in->mCommandPart = rhs.in->mCommandPart;
	in->SetParamPart(rhs.in->mParamPart);
	in->mHasSpace = rhs.in->mHasSpace;
	in->mStrParamMap = rhs.in->mStrParamMap;
	in->mBoolParamMap = rhs.in->mBoolParamMap;
}

ParameterBuilder::ParameterBuilder(
	const CString& str
) : in(std::make_unique<PImpl>())
{
	SetWholeString(str);
}

ParameterBuilder::~ParameterBuilder()
{
}

ParameterBuilder& ParameterBuilder::operator = (const ParameterBuilder& rhs)
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

ParameterBuilder* ParameterBuilder::Create()
{
	return new ParameterBuilder();
}

ParameterBuilder* ParameterBuilder::Create(const CString& str)
{
	return new ParameterBuilder(str);
}

ParameterBuilder* ParameterBuilder::Clone_() const
{
	return new ParameterBuilder(*this); 
}

ParameterBuilder* ParameterBuilder::EmptyParam()
{
	static ParameterBuilder paramEmpty;
	return &paramEmpty;
}


bool ParameterBuilder::IsEmpty() const
{
	return in->mWholeText.IsEmpty() != FALSE;
}

bool ParameterBuilder::HasParameter() const
{
	return in->mParamPart.IsEmpty() == FALSE;
}

void ParameterBuilder::AddArgument(const CString& arg)
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

LPCTSTR ParameterBuilder::GetWholeString() const
{
	return in->mWholeText;
}

LPCTSTR ParameterBuilder::GetCommandString() const
{
	return in->mCommandPart;
}

LPCTSTR ParameterBuilder::GetParameterString() const
{
	return in->mParamPart;
}

void ParameterBuilder::CopyParamTo(ParameterBuilder& rhs) const
{
	rhs.in->SetParamPart(in->mParamPart);
	rhs.in->mHasSpace = in->mHasSpace;

	// 内部用のパラメータ↓はコピーしない
	//rhs.in->mStrParamMap = in->mStrParamMap;
	//rhs.in->mBoolParamMap = in->mBoolParamMap;
}

void ParameterBuilder::CopyNamedParamTo(ParameterBuilder& rhs) const
{
	// 内部用のパラメータのみコピー
	rhs.in->mStrParamMap = in->mStrParamMap;
	rhs.in->mBoolParamMap = in->mBoolParamMap;
}

int ParameterBuilder::GetParamCount() const
{
	return (int)in->mParamArray.size();
}

LPCTSTR ParameterBuilder::GetParam(int index) const
{
	return in->mParamArray[index];
}

void ParameterBuilder::SetWholeString(LPCTSTR str)
{
	in->mWholeText = str;

	// 与えられた文字列が絶対パスで、かつ、存在するものである場合は、
	// たとえスペースを含むとしても、パラメータとして扱わない
	if (PathIsRelative(str) == FALSE && Path::FileExists(str)) {
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

void ParameterBuilder::SetParameterString(LPCTSTR param)
{
	in->SetParamPart(param);
	in->mWholeText = in->mCommandPart + _T(" ") + param;
	in->mHasSpace = (param[0] != _T('\0'));
}

Parameter* ParameterBuilder::Clone() const
{
	return new ParameterBuilder(*this); 
}

// 補完
bool ParameterBuilder::ComplementCommand(
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
int ParameterBuilder::GetNamedParamStringLength(
	LPCTSTR name
) const
{
	auto itFind = in->mStrParamMap.find(name);
	if (itFind == in->mStrParamMap.end()) {
		return 0;
	}
	return itFind->second.GetLength() + 1;
}

LPCTSTR ParameterBuilder::GetNamedParamString(LPCTSTR name, LPTSTR buf, int bufLen) const
{
	auto itFind = in->mStrParamMap.find(name);
	if (itFind == in->mStrParamMap.end()) {
		return _T("");
	}

	_tcsncpy_s(buf, bufLen, itFind->second, bufLen);
	return buf;
}

void ParameterBuilder::SetNamedParamString(LPCTSTR name, LPCTSTR value)
{
	in->mStrParamMap[name] = value;
}

bool ParameterBuilder::GetNamedParamBool(LPCTSTR name) const
{
	auto itFind = in->mBoolParamMap.find(name);
	if (itFind == in->mBoolParamMap.end()) {
		return false;
	}
	return itFind->second;
}

void ParameterBuilder::SetNamedParamBool(
	LPCTSTR name,
	bool value
)
{
	in->mBoolParamMap[name] = value;
}


bool ParameterBuilder::QueryInterface(const IFID& ifid, void** cmd)
{
	if (ifid == IFID_ACTIONPARAMETER) {
		AddRef();
		*cmd = (Parameter*)this;
		return true;
	}
	else if (ifid == IFID_ACTIONNAMEDPARAMETER) {
		AddRef();
		*cmd = (NamedParameter*)this;
		return true;
	}
	return false;
}
uint32_t ParameterBuilder::AddRef()
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t ParameterBuilder::Release()
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

}}}
