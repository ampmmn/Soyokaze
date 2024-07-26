#pragma once

#include "matcher/Pattern.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace presentation {

struct SLIDE_ITEM
{
	int mPage = 0;
	int mMatchLevel = 0;
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
