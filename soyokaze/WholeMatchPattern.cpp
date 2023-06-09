#include "pch.h"
#include "framework.h"
#include "WholeMatchPattern.h"
#include "core/CommandParameter.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct WholeMatchPattern::PImpl
{
	CString mWord;
	CString mWholeText;
};

WholeMatchPattern::WholeMatchPattern(const CString& word) : in(new PImpl)
{
	in->mWord = word;
	in->mWholeText = word;
}

WholeMatchPattern::~WholeMatchPattern()
{
	delete in;
}

void WholeMatchPattern::SetParam(
	const soyokaze::core::CommandParameter& param
)
{
	in->mWord = param.GetCommandString();
	in->mWholeText = param.GetWholeString();
}

int WholeMatchPattern::Match(
	const CString& str
)
{
	return str.CompareNoCase(in->mWord) == 0 ? WholeMatch : Mismatch;
}


CString WholeMatchPattern::GetOriginalPattern()
{
	return in->mWholeText;
}

