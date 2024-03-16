#pragma once

#include <memory>
#include "matcher/Pattern.h"


class WholeMatchPattern : public Pattern
{
public:
	WholeMatchPattern(const CString& word);
	virtual ~WholeMatchPattern();

	void SetParam(const soyokaze::core::CommandParameter& param) override;
	int Match(const CString& str) override;
	virtual int Match(const CString& str, int offset) override;
	CString GetFirstWord() override;
	CString GetWholeString() override;
	bool shouldWholeMatch() override;
	void GetWords(std::vector<WORD>& words) override;
	int GetWordCount() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

