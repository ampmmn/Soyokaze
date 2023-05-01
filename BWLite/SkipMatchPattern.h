#pragma once

#include "Pattern.h"


class SkipMatchPattern : public Pattern
{
public:
	SkipMatchPattern();
	virtual ~SkipMatchPattern();

	void SetPattern(const CString& pattern);
	bool Match(const CString& str);

protected:
	struct PImpl;
	PImpl* in;
};

