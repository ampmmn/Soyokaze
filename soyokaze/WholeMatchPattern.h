#pragma once

#include "Pattern.h"


class WholeMatchPattern : public Pattern
{
public:
	WholeMatchPattern(const CString& word);
	virtual ~WholeMatchPattern();

	virtual void SetPattern(const CString& pattern);
	virtual int Match(const CString& str);
	virtual CString GetOriginalPattern();

protected:
	struct PImpl;
	PImpl* in;
};

