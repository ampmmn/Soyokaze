#pragma once

#include "Pattern.h"


class PartialMatchPattern : public Pattern
{
public:
	PartialMatchPattern();
	virtual ~PartialMatchPattern();

	void SetPattern(const CString& pattern);
	bool Match(const CString& str);

protected:
	struct PImpl;
	PImpl* in;
};

