#pragma once

#include <memory>
#include "Pattern.h"


class WholeMatchPattern : public Pattern
{
public:
	WholeMatchPattern(const CString& word);
	virtual ~WholeMatchPattern();

	void SetParam(const soyokaze::core::CommandParameter& param) override;
	int Match(const CString& str) override;
	CString GetFirstWord() override;
	CString GetWholeString() override;
	bool shouldWholeMatch() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

