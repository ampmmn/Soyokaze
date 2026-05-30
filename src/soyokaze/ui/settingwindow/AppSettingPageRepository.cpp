#include "pch.h"
#include "AppSettingPageRepository.h"
#include <map>
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace settingwindow {

struct AppSettingPageRepository::PImpl
{
	std::mutex mMutex;
	std::map<CString, AppSettingPageIF*> mPages;
};


AppSettingPageRepository::AppSettingPageRepository() : in(std::make_unique<PImpl>())
{
}

AppSettingPageRepository::~AppSettingPageRepository()
{
	ReleaseAllPages();
}

AppSettingPageRepository* AppSettingPageRepository::GetInstance()
{
	static AppSettingPageRepository inst;
	return &inst;
}

void AppSettingPageRepository::ReleaseAllPages()
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	for (auto& item : in->mPages) {
		auto page = item.second;
		page->Release();
	}
	in->mPages.clear();
}

void AppSettingPageRepository::EnumSettingPages(std::vector<AppSettingPageIF*>& pages)
{
	pages.reserve(in->mPages.size());
	for (auto item : in->mPages) {
		auto page = item.second;
		pages.push_back(page->Clone());
	}
}

// 登録
bool AppSettingPageRepository::RegisterPage(
	AppSettingPageIF* page
)
{
	ASSERT(page);

	std::lock_guard<std::mutex> lock(in->mMutex);

	auto path = page->GetPagePath();
	auto itFind = in->mPages.find(path);
	if (itFind != in->mPages.end()) {
		auto orgPage = itFind->second;
		orgPage->Release();
	}

	in->mPages[path] = page;
	page->AddRef();

	return true;
}

bool AppSettingPageRepository::UnregisterPage(AppSettingPageIF* page)
{
	ASSERT(page);

	std::lock_guard<std::mutex> lock(in->mMutex);

	auto it = in->mPages.find(page->GetPagePath());
	if (it == in->mPages.end()) {
		return false;
	}

	it->second->Release();
	in->mPages.erase(it);
	return true;
}

}  // end of namespace settingwindow
}  // end of namespace launcherapp

