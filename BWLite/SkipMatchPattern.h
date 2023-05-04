#pragma once

#include "Pattern.h"


class SkipMatchPattern : public Pattern
{
public:
	SkipMatchPattern();
	virtual ~SkipMatchPattern();

	virtual void SetPattern(const CString& pattern);
	virtual bool Match(const CString& str);
	virtual CString GetOriginalPattern();

protected:
	struct PImpl;
	PImpl* in;
};

