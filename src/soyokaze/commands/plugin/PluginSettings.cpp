#include "pch.h"
#include "PluginSettings.h"
#include "utility/Path.h"

#include <algorithm>
#include <map>

#define PLUGIN_SETTINGS_FILENAME _T("plugin.dat")

namespace launcherapp {
namespace commands {
namespace plugin {

struct PluginSettings::PImpl
{
	std::map<CString, Item> mItems;
	CString mFilePath;
};

PluginSettings::PluginSettings() : in(new PImpl()) {}
PluginSettings::~PluginSettings() {}

PluginSettings* PluginSettings::GetInstance()
{
	static PluginSettings instance;
	return &instance;
}

bool PluginSettings::Load()
{
	CString path = in->mFilePath.IsEmpty() ? CString(Path(Path::APPDIR, PLUGIN_SETTINGS_FILENAME)) : in->mFilePath;
	FILE* fp = nullptr;
	if (_tfopen_s(&fp, path, _T("r,ccs=UTF-8")) != 0 || fp == nullptr) {
		return false;
	}

	std::map<CString, Item> items;
	CStdioFile file(fp);
	CString line;
	while (file.ReadString(line)) {
		line.Trim();
		if (line.IsEmpty()) {
			continue;
		}
		int position = 0;
		CString id = line.Tokenize(_T("\t"), position);
		CString enabled = line.Tokenize(_T("\t"), position);
		CString priority = line.Tokenize(_T("\t"), position);
		int enabledValue = 0;
		int priorityValue = 0;
		if (id.IsEmpty() || _stscanf_s(enabled, _T("%d"), &enabledValue) != 1 ||
			_stscanf_s(priority, _T("%d"), &priorityValue) != 1) {
			continue;
		}
		items[id] = {enabledValue != 0, (std::max)(0, priorityValue)};
	}
	file.Close();
	in->mItems.swap(items);
	return true;
}

bool PluginSettings::Save() const
{
	CString path = in->mFilePath.IsEmpty() ? CString(Path(Path::APPDIR, PLUGIN_SETTINGS_FILENAME)) : in->mFilePath;
	CString temporaryPath = path + _T(".tmp");
	FILE* fp = nullptr;
	if (_tfopen_s(&fp, temporaryPath, _T("w,ccs=UTF-8")) != 0 || fp == nullptr) {
		return false;
	}
	CStdioFile file(fp);
	for (const auto& item : in->mItems) {
		CString line;
		line.Format(_T("%s\t%d\t%d\n"), (LPCTSTR)item.first,
			item.second.mIsEnabled ? 1 : 0, (std::max)(0, item.second.mPriority));
		file.WriteString(line);
	}
	file.Close();
	if (CopyFile(temporaryPath, path, FALSE) == FALSE) {
		return false;
	}
	DeleteFile(temporaryPath);
	return true;
}

PluginSettings::Item PluginSettings::Get(const CString& pluginId) const
{
	auto it = in->mItems.find(pluginId);
	return it == in->mItems.end() ? Item{} : it->second;
}

void PluginSettings::Set(const CString& pluginId, const Item& item)
{
	if (pluginId.IsEmpty()) {
		return;
	}
	Item normalized = item;
	normalized.mPriority = (std::max)(0, normalized.mPriority);
	in->mItems[pluginId] = normalized;
}

std::unique_ptr<PluginSettings> PluginSettings::Clone() const
{
	std::unique_ptr<PluginSettings> result(new PluginSettings());
	CopyTo(result.get());
	return result;
}

void PluginSettings::CopyTo(PluginSettings* destination) const
{
	if (destination) {
	destination->in->mItems = in->mItems;
	destination->in->mFilePath = in->mFilePath;
	}
}

void PluginSettings::SetFilePath(const CString& path)
{
	in->mFilePath = path;
}

} // namespace plugin
} // namespace commands
} // namespace launcherapp
