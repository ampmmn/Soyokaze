#pragma once

namespace launcherapp {
namespace macros {
namespace core {

class Token
{
public:
	class Exception {};

public:
	Token(const CString& text) : mText(text), mPos(0)
	{
	}
	~Token() {}

	bool AtEnd()
	{
		return mPos == mText.GetLength();
	}

	void Next()
 	{
		if (AtEnd()) {
			return;
		}

		mPos++;
	}

	int GetPos()
	{
		return mPos;
	}

	TCHAR Get() {
		if (AtEnd()) {
			throw Exception(); 
		}
		return mText[mPos];
	}

	bool IsWhiteSpace()
	{
		if (AtEnd()) {
			return false;
		}
		return IsWhiteSpaceChar(Get());
	}

	void SkipWhiteSpace()
	{
		if (AtEnd()) {
			return;
		}

		TCHAR c = mText[mPos];

		while (IsWhiteSpaceChar(c)) {
			c = mText[++mPos];
			if (AtEnd()) {
				return;
			}
		}
	}

	void SkipName(CString& name)
 	{
		// 名前として使える文字
		static TCHAR NAMECHARS[] = _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
		static size_t len = sizeof(NAMECHARS) / sizeof(NAMECHARS[0]);

		CString tmpName;

		while (AtEnd() == false) {
			TCHAR c = Get();
			auto it = std::find(NAMECHARS, NAMECHARS + len, c);
			if (it == NAMECHARS + len) {
				break;
			}
			tmpName += c;
			Next();
		}
		name = tmpName;
	}

	// cが現れるまで位置を進める
	void SkipUntil(TCHAR c)
	{
		while (AtEnd() == false && Get() != c) {
			Next();
		}
	}

	// 現在位置にある文字列の次の位置に進める。飛ばした位置にある文字列をstrに入れて返す
	void SkipString(CString& str)
	{
		CString tmpStr;
		while (AtEnd() == false) {
			if (IsWhiteSpace() || Get() == _T('}')) {
				break;
			}
			if (Get() == _T('\\')) {
				// エスケープシーケンス
				Next();

				if (AtEnd()) {
					break;
				}
				if (Get() != _T('}')) {
					tmpStr += _T('\\');
					tmpStr += Get();
					Next();
					continue;
				}
			}
			tmpStr += Get();
			Next();
		}
		str = tmpStr;
	}
	
	static bool IsWhiteSpaceChar(TCHAR c) {
		return c == _T(' ') || c == _T('\t');
	}

private:
	CString mText;
	int mPos;
};




}
}
}

