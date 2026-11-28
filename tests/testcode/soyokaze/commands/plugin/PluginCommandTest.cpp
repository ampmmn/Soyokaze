#include "stdafx.h"
#include "gtest/gtest.h"
#include "actions/core/Action.h"
#include "commands/plugin/PluginCommand.h"
#include "hotkey/HotKeyAttribute.h"
#include <algorithm>
#include <cstring>

using launcherapp::actions::core::Action;
using launcherapp::commands::plugin::PluginCommand;
using launcherapp::commands::plugin::PluginMatchPtr;
using launcherapp::commands::plugin::PluginModule;
using launcherapp::commands::plugin::PluginModulePtr;

namespace {

unsigned int gGuideModifier = 0;
unsigned int gExecuteModifier = 0;

int CopyString(const char* value, char* buffer, size_t length)
{
	const size_t required = std::strlen(value) + 1;
	if (buffer == nullptr || length == 0) {
		return static_cast<int>(required);
	}
	const size_t copied = (std::min)(std::strlen(value), length - 1);
	std::memcpy(buffer, value, copied);
	buffer[copied] = '\0';
	return static_cast<int>(copied);
}

int GetTypeDisplayName(LNCRPLUGINMATCHHANDLE, int, char* buffer, size_t length)
{
	return CopyString("Sample", buffer, length);
}

int GetGuide(LNCRPLUGINMATCHHANDLE, int, unsigned int modifier, char* buffer, size_t length)
{
	gGuideModifier = modifier;
	return CopyString("Sample action", buffer, length);
}

int Execute(LNCRPLUGINMATCHHANDLE, int, unsigned int modifier, int, char**)
{
	gExecuteModifier = modifier;
	return 0;
}

int GetErrorString(LNCRPLUGINMATCHHANDLE, int, char* buffer, size_t length)
{
	return CopyString("", buffer, length);
}

}

TEST(PluginCommandTest, PassesModifierToGuideAndExecute)
{
	gGuideModifier = 0;
	gExecuteModifier = 0;

	PluginModulePtr module = std::make_shared<PluginModule>();
	module->mExportTable.GetTypeDisplayName = &GetTypeDisplayName;
	module->mExportTable.GetGuide = &GetGuide;
	module->mExportTable.Execute = &Execute;
	module->mExportTable.GetErrorString = &GetErrorString;
	PluginMatchPtr match = std::make_shared<int>(0);
	PluginCommand command(module, match, 0, _T("sample"), _T("sample description"));

	Action* action = nullptr;
	ASSERT_TRUE(command.GetAction(HOTKEY_ATTR(MOD_CONTROL, VK_RETURN), &action));
	ASSERT_NE(nullptr, action);
	EXPECT_EQ(MOD_CONTROL, gGuideModifier);
	EXPECT_EQ(_T("Sample action"), action->GetDisplayName());

	EXPECT_TRUE(action->Perform(nullptr));
	EXPECT_EQ(MOD_CONTROL, gExecuteModifier);
	action->Release();
}
