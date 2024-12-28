#include "pch.h"
#include "framework.h"
#include "WholeMatchPattern.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/IFIDDefine.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct WholeMatchPattern::PImpl
{
	CString mWord;
	CString mWholeText;
	uint32_t mRefCount = 1;
};

WholeMatchPattern::WholeMatchPattern(const CString& word) : in(std::make_unique<PImpl>())
{
	in->mWord = word;
	in->mWholeText = word;
}

WholeMatchPattern::~WholeMatchPattern()
{
}

WholeMatchPattern* WholeMatchPattern::Create(const CString& word)
{
	return new WholeMatchPattern(word);
}

bool WholeMatchPattern::QueryInterface(const IFID& ifid, void** obj)
{
	if (ifid == IFID_PATTERN) {
		AddRef();
		*obj = (Pattern*)this;
		return true;
	}
	if (ifid == IFID_PATTERNINTERNAL) {
		AddRef();
		*obj = (PatternInternal*)this;
		return true;
	}
	return false;
}

uint32_t WholeMatchPattern::AddRef()
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t WholeMatchPattern::Release()
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

void WholeMatchPattern::SetWholeText(
	LPCTSTR wholeText
)
{
	auto param = launcherapp::core::CommandParameterBuilder::Create(wholeText);

	in->mWord = param->GetCommandString();
	in->mWholeText = param->GetWholeString();

	param->Release();
}

int WholeMatchPattern::Match(
	LPCTSTR str
)
{
	return in->mWord.CompareNoCase(str) == 0 ? WholeMatch : Mismatch;
}

int WholeMatchPattern::Match(LPCTSTR str, int offset)
{
	UNREFERENCED_PARAMETER(offset);

	// offsetをサポートしない
	return in->mWord.CompareNoCase(str) == 0 ? WholeMatch : Mismatch;
}


LPCTSTR WholeMatchPattern::GetFirstWord()
{
	return in->mWord;
}


LPCTSTR WholeMatchPattern::GetWholeString()
{
	return in->mWholeText;
}

bool WholeMatchPattern::shouldWholeMatch()
{
	return true;
}


void WholeMatchPattern::GetWords(std::vector<WORD>& words)
{
	words.push_back(WORD(in->mWholeText, PatternInternal::FixString));
}

void WholeMatchPattern::GetRawWords(std::vector<CString>& words)
{
	words.push_back(in->mWholeText);
}

int WholeMatchPattern::GetWordCount()
{
	return 1;
}


