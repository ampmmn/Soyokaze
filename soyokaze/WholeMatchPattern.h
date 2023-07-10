#pragma once

#include "Pattern.h"


class WholeMatchPattern : public Pattern
{
public:
	WholeMatchPattern(const CString& word);
	virtual ~WholeMatchPattern();

	virtual void SetParam(const soyokaze::core::CommandParameter& param);
	virtual int Match(const CString& str);
	virtual CString GetOriginalPattern();

protected:
	struct PImpl;
	PImpl* in;
};

