#include "pch.h"
#include "PluginModule.h"
#include "utility/CharConverter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace plugin {

namespace {
CString GetInfoString(const nlohmann::json& info, const char* key)
{
	if (info.contains(key) == false || info[key].is_string() == false) {
		return _T("");
	}
	CString result;
	UTF2UTF(info[key].get<std::string>(), result);
	return result;
}
}

CString PluginModule::GetId() const { return GetInfoString(mPluginInfo, "pluginId"); }
CString PluginModule::GetDisplayName() const { return GetInfoString(mPluginInfo, "displayName"); }
CString PluginModule::GetVersion() const { return GetInfoString(mPluginInfo, "pluginVersion"); }
CString PluginModule::GetDescription() const { return GetInfoString(mPluginInfo, "pluginDescription"); }


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
