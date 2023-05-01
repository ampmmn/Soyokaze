#pragma once

#include "Pattern.h"


class ForwardMatchPattern : public Pattern
{
public:
	ForwardMatchPattern();
	virtual ~ForwardMatchPattern();

	void SetPattern(const CString& pattern);
	bool Match(const CString& str);

protected:
	struct PImpl;
	PImpl* in;
};

