#pragma once

#include "Pattern.h"


/**
 * 前方一致比較を行うためのクラス
 */
class ForwardMatchPattern : public Pattern
{
public:
	ForwardMatchPattern();
	virtual ~ForwardMatchPattern();

	virtual void SetPattern(const CString& pattern);
	virtual int Match(const CString& str);
	virtual CString GetOriginalPattern();

protected:
	struct PImpl;
	PImpl* in;
};

