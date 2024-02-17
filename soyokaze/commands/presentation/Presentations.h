#pragma once

#include "Pattern.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace presentation {

struct SLIDE_ITEM
{
	int mPage;
	int mMatchLevel;
	CString mTitle;
};


class Presentations
{
public:
	Presentations();
	~Presentations();

	void Abort();

	// 指定したパターンに合致するウインドウを探す
	void Query(Pattern* pattern, std::vector<SLIDE_ITEM>& items, int limit);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}
