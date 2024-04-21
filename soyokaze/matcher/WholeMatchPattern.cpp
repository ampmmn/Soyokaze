#include "pch.h"
#include "framework.h"
#include "matcher/WholeMatchPattern.h"
#include "commands/core/CommandParameter.h"
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
	const launcherapp::core::CommandParameter& param
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

int WholeMatchPattern::Match(const CString& str, int offset)
{
	// offsetをサポートしない
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


void WholeMatchPattern::GetWords(std::vector<WORD>& words)
{
	words.push_back(WORD(in->mWholeText, Pattern::FixString));
}

int WholeMatchPattern::GetWordCount()
{
	return 1;
}


