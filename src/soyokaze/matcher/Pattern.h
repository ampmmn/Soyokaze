#pragma once

#include "commands/core/UnknownIF.h"

// キーワード比較処理のためのインタフェース
class Pattern : virtual public launcherapp::core::UnknownIF
{
public:
	enum MatchLevel {
		WholeMatch = 4,    // 完全一致
		FrontMatch = 3,    // 前方一致
		PartialMatch = 2,  // 部分一致
		WeakMatch = 1,     // 弱い一致
		Mismatch = -1,     // 不一致
	};

public:
	using IFID = launcherapp::core::IFID;

public:
	virtual void SetWholeText(LPCTSTR wholeText) = 0;
	virtual int Match(LPCTSTR str) = 0;
	virtual int Match(LPCTSTR str, int offset) = 0;
	virtual LPCTSTR GetFirstWord() = 0;
	virtual LPCTSTR GetWholeString() = 0;
	virtual bool shouldWholeMatch() = 0;
	virtual int GetWordCount() = 0;

};


