#pragma once

#include <memory>

#include "matcher/Pattern.h"


/**
 * 部分一致比較を行うためのクラス
 */
class PartialMatchPattern : public Pattern
{
public:
	PartialMatchPattern();
	virtual ~PartialMatchPattern();

	void SetParam(const launcherapp::core::CommandParameter& param) override;
	int Match(const CString& str) override;
	int Match(const CString& str, int offset) override;
	CString GetFirstWord() override;
	CString GetWholeString() override;
	bool shouldWholeMatch() override;
	void GetWords(std::vector<WORD>& words) override;
	int GetWordCount() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

