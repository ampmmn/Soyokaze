#pragma once

#include <memory>

#include "matcher/Pattern.h"


/**
 * 部分一致比較を行うためのクラス
 */
class PartialMatchPattern : public Pattern
{
public:
	PartialMatchPattern();
	virtual ~PartialMatchPattern();

	void SetWholeText(LPCTSTR wholeText) override;
	int Match(LPCTSTR str) override;
	int Match(LPCTSTR str, int offset) override;
	LPCTSTR GetFirstWord() override;
	LPCTSTR GetWholeString() override;
	bool shouldWholeMatch() override;
	void GetWords(std::vector<WORD>& words) override;
	void GetRawWords(std::vector<CString>& words) override;
	int GetWordCount() override;

	static CString StripEscapeChars(const CString& pat);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

