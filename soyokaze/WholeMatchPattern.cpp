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

WholeMatchPattern::WholeMatchPattern(const CString& word) : in(std::make_unique<PImpl>())
{
	in->mWord = word;
	in->mWholeText = word;
}

WholeMatchPattern::~WholeMatchPattern()
{
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

CString WholeMatchPattern::GetFirstWord()
{
	return in->mWord;
}


CString WholeMatchPattern::GetWholeString()
{
	return in->mWholeText;
}

bool WholeMatchPattern::shouldWholeMatch()
{
	return true;
}


