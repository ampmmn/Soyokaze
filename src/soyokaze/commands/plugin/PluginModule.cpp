#include "pch.h"
#include "PluginModule.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace plugin {


/**
  プラグインの終了処理を呼び出してDLLをアンロードする
*/
PluginModule::~PluginModule()
{
	if (mModule) {
		// DLLをアンロードする前に、プラグインが保持するリソースを解放させる。
		mExportTable.Finalize();
		FreeLibrary(mModule);
		mModule = nullptr;
	}
}


} // namespace plugin
} // namespace commands
} // namespace launcherapp
