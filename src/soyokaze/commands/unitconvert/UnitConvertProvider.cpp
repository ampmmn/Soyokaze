#include "pch.h"
#include "UnitConvertProvider.h"
#include "commands/unitconvert/InchAdhocCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace unitconvert {


using CommandRepository = launcherapp::core::CommandRepository;

struct UnitConvertProvider::PImpl : public AppPreferenceListenerIF
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
		Reload();
	}
	void OnAppExit() override {}

	void Reload()
	{
	}

	InchAdhocCommand* mInchCmdPtr = nullptr;
	// 初回呼び出しフラグ(初回呼び出し時に設定をロードするため)
	bool mIsFirstCall = true;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(UnitConvertProvider)


UnitConvertProvider::UnitConvertProvider() : in(std::make_unique<PImpl>())
{
	in->mInchCmdPtr = new InchAdhocCommand();
}

UnitConvertProvider::~UnitConvertProvider()
{
	if (in->mInchCmdPtr) {
		in->mInchCmdPtr->Release();
	}
}

CString UnitConvertProvider::GetName()
{
	return _T("UnitConvert");
}

// 一時的なコマンドを必要に応じて提供する
void UnitConvertProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		in->Reload();
		in->mIsFirstCall = false;
	}

	int level = in->mInchCmdPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mInchCmdPtr->AddRef();
		commands.Add(CommandQueryItem(level, in->mInchCmdPtr));
		return;
	}
}

} // end of namespace unitconvert
} // end of namespace commands
} // end of namespace launcherapp

