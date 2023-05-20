#include "pch.h"
#include "framework.h"
#include "WholeMatchPattern.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct WholeMatchPattern::PImpl
{
	CString mWord;
};

WholeMatchPattern::WholeMatchPattern(const CString& word) : in(new PImpl)
{
	in->mWord = word;
}

WholeMatchPattern::~WholeMatchPattern()
{
	delete in;
}

void WholeMatchPattern::SetPattern(
	const CString& pattern
)
{
	in->mWord = pattern;
}

int WholeMatchPattern::Match(
	const CString& str
)
{
	return str.CompareNoCase(in->mWord) == 0 ? WholeMatch : Mismatch;
}


CString WholeMatchPattern::GetOriginalPattern()
{
	return in->mWord;
}

