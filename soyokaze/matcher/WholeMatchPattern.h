#pragma once

#include <memory>
#include "matcher/Pattern.h"


class WholeMatchPattern :
 	virtual public Pattern
{
private:
	WholeMatchPattern(const CString& word);
	~WholeMatchPattern();

public:
	static WholeMatchPattern* Create(const CString& word);

// UnknownIF
	bool QueryInterface(const IFID& ifid, void** cmd) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

// Pattern
	void SetWholeText(LPCTSTR wholeText) override;
	int Match(LPCTSTR str) override;
	virtual int Match(LPCTSTR str, int offset) override;
	LPCTSTR GetFirstWord() override;
	LPCTSTR GetWholeString() override;
	bool shouldWholeMatch() override;
	void GetWords(std::vector<WORD>& words) override;
	void GetRawWords(std::vector<CString>& words) override;
	int GetWordCount() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

