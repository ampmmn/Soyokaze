#pragma once

#include "Pattern.h"


class SkipMatchPattern : public Pattern
{
public:
	SkipMatchPattern();
	virtual ~SkipMatchPattern();

	virtual void SetParam(const soyokaze::core::CommandParameter& param) ;
	virtual int Match(const CString& str);
	virtual CString GetOriginalPattern();

protected:
	struct PImpl;
	PImpl* in;
};

