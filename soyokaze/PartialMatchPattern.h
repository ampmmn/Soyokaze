#pragma once

#include "Pattern.h"


/**
 * 部分一致比較を行うためのクラス
 */
class PartialMatchPattern : public Pattern
{
public:
	PartialMatchPattern();
	virtual ~PartialMatchPattern();

	virtual void SetPattern(const CString& pattern);
	virtual int Match(const CString& str);
	virtual CString GetOriginalPattern();

protected:
	struct PImpl;
	PImpl* in;
};

