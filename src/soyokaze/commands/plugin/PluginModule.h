#pragma once

#ifndef LNCRAPP
#define LNCRAPP
#endif

#include "../../../../plugin-include/soyokaze/PluginExportTable.h"
#include <memory>
#include <nlohmann/json.hpp>

namespace launcherapp {
namespace commands {
namespace plugin {

struct PluginModule
{
	/**
	  プラグインDLLを終了処理してアンロードする
	*/
	~PluginModule();

	HMODULE mModule{nullptr};
	LNCRPLUGIN_EXPORTTABLE mExportTable{};
	nlohmann::json mPluginInfo{};

	CString GetId() const;
	CString GetDisplayName() const;
	CString GetVersion() const;
	CString GetDescription() const;
};

using PluginModulePtr = std::shared_ptr<PluginModule>;
using PluginMatchPtr = std::shared_ptr<void>;

} // namespace plugin
} // namespace commands
} // namespace launcherapp
