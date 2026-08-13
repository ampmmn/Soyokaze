#include "pch.h"
#include "PluginModule.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace plugin {


PluginModule::~PluginModule()
{
	if (mModule) {
		mExportTable.Finalize();
		FreeLibrary(mModule);
		mModule = nullptr;
	}
}


} // namespace plugin
} // namespace commands
} // namespace launcherapp
