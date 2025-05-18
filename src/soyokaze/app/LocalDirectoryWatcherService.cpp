#include "pch.h"
#include "LocalDirectoryWatcherService.h"
#include "utility/LocalDirectoryWatcher.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"

namespace LocalDirectoryWatcherService {

class FinalizeCaller : public AppPreferenceListenerIF
{
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override {}
	void OnAppExit()
	{
		LocalDirectoryWatcher::GetInstance()->Finalize();
		AppPreference::Get()->UnregisterListener(this);
	}
};

void Start()
{
	static FinalizeCaller inst;
	AppPreference::Get()->RegisterListener(&inst);
}

} // end of namespace LocalDirectoryWatcherService

 
