#include "pch.h"
#include "PluginProvider.h"
#include "PluginCommand.h"
#include "SharedHwnd.h"
#include "app/LauncherApp.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/Path.h"
#include <mutex>
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

struct MatcherContext
{
	Pattern* mPattern;
	std::string mFirstWord;
	std::string mWholeString;
};

int Match(void* ctx, const char* text, int offset)
{
	MatcherContext* context = static_cast<MatcherContext*>(ctx);
	CString value;
	UTF2UTF(std::string(text ? text : ""), value);
	return context->mPattern->Match(value, static_cast<uint32_t>(offset));
}

const char* GetFirstWord(void* ctx)
{
	MatcherContext* context = static_cast<MatcherContext*>(ctx);
	UTF2UTF(std::wstring(context->mPattern->GetFirstWord()), context->mFirstWord);
	return context->mFirstWord.c_str();
}

const char* GetWholeString(void* ctx)
{
	MatcherContext* context = static_cast<MatcherContext*>(ctx);
	UTF2UTF(std::wstring(context->mPattern->GetWholeString()), context->mWholeString);
	return context->mWholeString.c_str();
}

void InfoLog(const char* message)
{
	spdlog::info("{}", message ? message : "");
}

void WarnLog(const char* message)
{
	spdlog::warn("{}", message ? message : "");
}

void ErrorLog(const char* message)
{
	spdlog::error("{}", message ? message : "");
}

void PopupMessage(const char* message)
{
	auto app = static_cast<LauncherApp*>(AfxGetApp());
	if (app) {
		app->PopupMessage(message ? message : "");
	}
}

HWND GetMainWindowHandle()
{
	SharedHwnd sharedHwnd;
	return sharedHwnd.GetHwnd();
}

int GetWordCount(void* ctx)
{
	return static_cast<MatcherContext*>(ctx)->mPattern->GetWordCount();
}

} // namespace

struct PluginProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this, _T("PluginProvider"));
	}

	~PImpl() override
	{
		AppPreference::Get()->UnregisterListener(this);
		std::lock_guard<std::mutex> lock(mMutex);
		mPlugins.clear();
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override {}

	void OnAppExit() override
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mPlugins.clear();
	}

	void LoadPlugins()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		if (mIsLoaded) {
			return;
		}
		mIsLoaded = true;

		// 実行ファイル側のプラグインを優先してロードする
		LoadPlugins(Path(Path::MODULEFILEDIR, _T("plugins")));
		LoadPlugins(Path(Path::APPDIR, _T("plugins")));
	}

	/**
	  指定されたpluginsディレクトリ以下のプラグインをロードする
	  @param[in] pluginsPath プラグインディレクトリ
	*/
	void LoadPlugins(const Path& pluginsPath)
	{
		if (pluginsPath.IsDirectory() == false) {
			SPDLOG_WARN(_T("Plugin directory does not exist: {}"), (LPCTSTR)pluginsPath);
			return;
		}

		CString pattern = (LPCTSTR)pluginsPath;
		pattern += _T("\\*");
		CFileFind directoryFinder;
		BOOL hasDirectory = directoryFinder.FindFile(pattern);
		while (hasDirectory) {
			hasDirectory = directoryFinder.FindNextFile();
			if (directoryFinder.IsDots() || directoryFinder.IsDirectory() == FALSE) {
				continue;
			}

			CString dllPattern = directoryFinder.GetFilePath() + _T("\\*.dll");
			CFileFind dllFinder;
			BOOL hasDll = dllFinder.FindFile(dllPattern);
			while (hasDll) {
				hasDll = dllFinder.FindNextFile();
				if (dllFinder.IsDirectory()) {
					continue;
				}
				LoadPlugin(dllFinder.GetFilePath());
			}
			dllFinder.Close();
		}
		directoryFinder.Close();
	}

	void LoadPlugin(const CString& path)
	{
		HMODULE handle = LoadLibrary(path);
		if (handle == nullptr) {
			SPDLOG_WARN(_T("Failed to load plugin DLL: {}"), (LPCTSTR)path);
			return;
		}

		auto bind = reinterpret_cast<int (*)(int, LNCRPLUGIN_EXPORTTABLE*)>(
			GetProcAddress(handle, "LNCRPLUGIN_Bind"));
		if (bind == nullptr) {
			SPDLOG_WARN(_T("LNCRPLUGIN_Bind was not found: {}"), (LPCTSTR)path);
			FreeLibrary(handle);
			return;
		}

		LNCRPLUGIN_EXPORTTABLE table{};
		if (bind(PLUGINVERSION, &table) != 0 || IsValid(table) == false) {
			SPDLOG_WARN(_T("Invalid plugin export table: {}"), (LPCTSTR)path);
			FreeLibrary(handle);
			return;
		}

		auto plugin = std::make_shared<PluginModule>();
		plugin->mModule = handle;
		plugin->mExportTable = table;
		LAUNCHER_FUNCTION_TABLE launcherFunctionTable{
			&InfoLog,
			&WarnLog,
			&ErrorLog,
			&PopupMessage,
			&GetMainWindowHandle,
		};
		if (plugin->mExportTable.Initialize(&launcherFunctionTable) != 0) {
			SPDLOG_WARN(_T("Plugin initialization failed: {}"), (LPCTSTR)path);
			plugin->mModule = nullptr;
			FreeLibrary(handle);
			return;
		}
		mPlugins.push_back(std::move(plugin));
	}

	static bool IsValid(const LNCRPLUGIN_EXPORTTABLE& table)
	{
		return table.Initialize && table.Query && table.GetMatchCount &&
			table.GetMatchLevel &&
			table.CloseHandle && table.GetName && table.GetDescription &&
			table.GetGuide && table.GetTypeDisplayName && table.CanExecute &&
			table.Execute && table.GetErrorString && table.GetIcon && table.Finalize;
	}

	bool mIsLoaded{false};
	std::vector<PluginModulePtr> mPlugins;
	std::mutex mMutex;
};

REGISTER_COMMANDPROVIDER(PluginProvider)

PluginProvider::PluginProvider() : in(std::make_unique<PImpl>())
{
}

PluginProvider::~PluginProvider()
{
}

CString PluginProvider::GetName()
{
	return _T("Plugin");
}

void PluginProvider::PrepareAdhocCommands()
{
	in->LoadPlugins();
}

void PluginProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	if (pattern == nullptr) {
		return;
	}

	MATCHER_FUNCTION_TABLE matcherTable{
		&Match, &GetFirstWord, &GetWholeString, &GetWordCount
	};
	MatcherContext context{pattern, {}, {}};
	std::lock_guard<std::mutex> lock(in->mMutex);

	for (const auto& plugin : in->mPlugins) {
		LNCRPLUGINMATCHHANDLE handle = plugin->mExportTable.Query(&context, &matcherTable);
		if (handle == nullptr) {
			continue;
		}

		PluginModulePtr module = plugin;
		PluginMatchPtr match(handle, [module](void* value) {
			module->mExportTable.CloseHandle(static_cast<LNCRPLUGINMATCHHANDLE>(value));
		});

		int count = plugin->mExportTable.GetMatchCount(handle);
		if (count < 0) {
			match.reset();
			continue;
		}

		for (int i = 0; i < count; ++i) {
			CString name;
			CString description;
			if (GetStringValue(plugin->mExportTable.GetName, handle, i, name) == false ||
				GetStringValue(plugin->mExportTable.GetDescription, handle, i, description) == false) {
				continue;
			}

			int level = plugin->mExportTable.GetMatchLevel(handle, i);
			if (level == Pattern::Mismatch) {
				continue;
			}
			commands.Add(CommandQueryItem(level, new PluginCommand(plugin, match, i, name, description)));
		}
	}
}

uint32_t PluginProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	UNREFERENCED_PARAMETER(displayNames);
	return 0;
}

} // namespace plugin
} // namespace commands
} // namespace launcherapp
