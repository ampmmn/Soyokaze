#pragma once

#ifndef LNCRAPP
#define LNCRAPP
#endif

#include "../../../../plugin-include/soyokaze/PluginExportTable.h"
#include <memory>

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
};

using PluginModulePtr = std::shared_ptr<PluginModule>;
using PluginMatchPtr = std::shared_ptr<void>;

} // namespace plugin
} // namespace commands
} // namespace launcherapp
