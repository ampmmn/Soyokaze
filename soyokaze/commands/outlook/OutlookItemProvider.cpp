#include "pch.h"
#include "OutlookItemProvider.h"
#include "commands/outlook/MailCommand.h"
#include "commands/outlook/OutlookItems.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace outlook {

using CommandRepository = launcherapp::core::CommandRepository;

struct OutlookItemProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnableMailItem = pref->IsEnableOutlookMailItem();
	}
	void OnAppExit() override {}

	//
	bool mIsEnableMailItem;
	bool mIsFirstCall;

	OutlookItems mOutlookItems;
	DWORD mLastUpdate;

	uint32_t mRefCount = 1;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(OutlookItemProvider)


OutlookItemProvider::OutlookItemProvider() : in(std::make_unique<PImpl>())
{
	in->mIsEnableMailItem = false;
	in->mIsFirstCall = true;
	in->mLastUpdate = 0;
}

OutlookItemProvider::~OutlookItemProvider()
{
}

CString OutlookItemProvider::GetName()
{
	return _T("OutlookItem");
}

// 一時的なコマンドを必要に応じて提供する
void OutlookItemProvider::QueryAdhocCommands(
	Pattern* pattern,
 	launcherapp::CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsEnableMailItem = pref->IsEnableOutlookMailItem();
		in->mIsFirstCall = false;
	}

	if (in->mIsEnableMailItem == false) {
		return ;
	}

	std::vector<MailItem*> mailItems;
	in->mOutlookItems.GetInboxMailItems(mailItems);

	for (auto item : mailItems) {

		Sleep(0);

		int level = pattern->Match(item->GetSubject());
		if (level != Pattern::Mismatch) {
			commands.push_back(CommandQueryItem(level, new MailCommand(item)));
		}
		item->Release();
	}
}

} // end of namespace outlook
} // end of namespace commands
} // end of namespace launcherapp

