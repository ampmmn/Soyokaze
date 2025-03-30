#pragma once

#include "settingwindow/AppSettingPageIF.h"
#include <memory>
#include <vector>

namespace launcherapp {
namespace settingwindow {

class AppSettingPageRepository
{
private:
	AppSettingPageRepository();
	~AppSettingPageRepository();

public:
	static AppSettingPageRepository* GetInstance();
	void ReleaseAllPages();

	// ページを列挙する
	void EnumSettingPages(std::vector<AppSettingPageIF*>& pages);

	// ページを登録
	bool RegisterPage(AppSettingPageIF* page);
	bool UnregisterPage(AppSettingPageIF* page);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}  // end of namespace settingwindow
}  // end of namespace launcherapp

// 派生クラス側で下記のマクロを通じてAppSettingPageとして登録する

#define DECLARE_APPSETTINGPAGE(clsName)   static bool RegisterAppSettingPage(); \
	private: \
	static bool _mIsPageRegistered; \
	public: \
	static bool IsPageRegistered();

#define REGISTER_APPSETTINGPAGE(clsName)   \
	bool clsName::RegisterAppSettingPage() { \
		try { \
			clsName* inst = new clsName(); \
			launcherapp::settingwindow::AppSettingPageRepository::GetInstance()->RegisterPage(inst); \
			inst->Release(); \
			return true; \
		} catch(...) { return false; } \
	} \
	bool clsName::_mIsPageRegistered = clsName::RegisterAppSettingPage(); \
	bool clsName::IsPageRegistered() { return _mIsPageRegistered; }


