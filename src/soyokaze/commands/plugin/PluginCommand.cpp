#include "pch.h"
#include "PluginCommand.h"
#include "actions/core/ActionBase.h"
#include "icon/IconLoader.h"
#include "utility/CharConverter.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace plugin {

namespace {

bool GetStringValue(LNCRPLUGINFUNC_GETSTRING func, LNCRPLUGINMATCHHANDLE match,
	int index, CString& value)
{
	value.Empty();
	int len = func(match, index, nullptr, 0);
	if (len < 0) {
		return false;
	}
	if (len == 0) {
		return true;
	}

	std::vector<char> buffer(static_cast<size_t>(len), '\0');
	if (func(match, index, buffer.data(), buffer.size()) < 0) {
		return false;
	}
	buffer.back() = '\0';

	std::string utf8(buffer.data());
	UTF2UTF(utf8, value);
	return true;
}

class PluginAction : public launcherapp::actions::core::ActionBase
{
public:
	PluginAction(const PluginModulePtr& module, const PluginMatchPtr& match, int index,
	            const CString& displayName) :
		mModule(module), mMatch(match), mIndex(index), mDisplayName(displayName)
	{
	}

	CString GetDisplayName() override
	{
		return mDisplayName;
	}

	bool Perform(Parameter* param, String* errMsg) override
	{
		std::vector<std::string> arguments;
		std::vector<char*> argv;
		int argc = param ? param->GetParamCount() : 0;
		arguments.reserve(argc);
		argv.reserve(argc + 1);

		for (int i = 0; i < argc; ++i) {
			std::string argument;
			UTF2UTF(std::wstring(param->GetParam(i)), argument);
			arguments.push_back(std::move(argument));
		}
		for (auto& argument : arguments) {
			argv.push_back(argument.data());
		}
		argv.push_back(nullptr);

		int result = mModule->mExportTable.Execute(
			static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, argc, argv.data());
		if (result == 0) {
			return true;
		}

		if (errMsg) {
			CString error;
			if (GetStringValue(mModule->mExportTable.GetErrorString,
				static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, error)) {
				std::string errorUtf8;
				UTF2UTF(std::wstring(error), errorUtf8);
				*errMsg = errorUtf8;
			}
		}
		return false;
	}

private:
	PluginModulePtr mModule;
	PluginMatchPtr mMatch;
	int mIndex;
	CString mDisplayName;
};

} // namespace

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(PluginCommand)

PluginCommand::PluginCommand(const PluginModulePtr& module, const PluginMatchPtr& match,
	int index, const CString& name, const CString& description) :
	AdhocCommandBase(name, description), mModule(module), mMatch(match), mIndex(index)
{
	GetStringValue(mModule->mExportTable.GetTypeDisplayName,
		static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, mTypeDisplayName);
}

PluginCommand::~PluginCommand()
{
}

CString PluginCommand::GetTypeDisplayName()
{
	return mTypeDisplayName;
}

bool PluginCommand::CanExecute(String* reasonMsg)
{
	int result = mModule->mExportTable.CanExecute(static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex);
	if (result != 0) {
		return true;
	}

	if (reasonMsg) {
		CString reason;
		if (GetStringValue(mModule->mExportTable.GetErrorString,
			static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, reason)) {
			std::string reasonUtf8;
			UTF2UTF(std::wstring(reason), reasonUtf8);
			*reasonMsg = reasonUtf8;
		}
	}
	return false;
}

bool PluginCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0 || action == nullptr) {
		return false;
	}

	CString guide;
	if (GetStringValue(mModule->mExportTable.GetGuide,
		static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, guide) == false) {
		return false;
	}

	*action = new PluginAction(mModule, mMatch, mIndex, guide);
	return true;
}

HICON PluginCommand::GetIcon()
{
	if (mIcon) {
		return mIcon;
	}

	if (mModule->mExportTable.GetIcon(
		static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, &mIcon) != 0 ||
		mIcon == nullptr) {
		mIcon = IconLoader::Get()->LoadDefaultIcon();
	}
	return mIcon;
}

launcherapp::core::Command* PluginCommand::Clone()
{
	return new PluginCommand(mModule, mMatch, mIndex, mName, mDescription);
}

} // namespace plugin
} // namespace commands
} // namespace launcherapp
