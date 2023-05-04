#pragma once

#include "Pattern.h"


class ForwardMatchPattern : public Pattern
{
public:
	ForwardMatchPattern();
	virtual ~ForwardMatchPattern();

	virtual void SetPattern(const CString& pattern);
	virtual bool Match(const CString& str);
	virtual CString GetOriginalPattern();

protected:
	struct PImpl;
	PImpl* in;
};

