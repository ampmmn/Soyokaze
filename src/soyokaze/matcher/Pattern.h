#pragma once

#include "core/UnknownIF.h"

// キーワード比較処理のためのインタフェース
class Pattern : virtual public launcherapp::core::UnknownIF
{
public:
	enum MatchLevel {
		WholeMatch = 5,    // 完全一致
		FrontMatch = 4,    // 前方一致
		PartialMatch = 3,  // 部分一致
		WeakMatch = 2,     // 弱い一致
		HiddenMatch = 1,   // 一致、しかし非表示
		Mismatch = -1,     // 不一致
	};

public:
	using IFID = launcherapp::core::IFID;

public:
	virtual void SetWholeText(LPCTSTR wholeText) = 0;
	virtual int Match(LPCTSTR str) = 0;
	virtual int Match(LPCTSTR str, uint32_t ignoreMask) = 0;
	virtual LPCTSTR GetFirstWord() = 0;
	virtual LPCTSTR GetWholeString() = 0;
	virtual bool shouldWholeMatch() = 0;
	virtual int GetWordCount() = 0;
	virtual int GetWholeTextLength() = 0;

};


