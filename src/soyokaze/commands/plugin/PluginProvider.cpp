#include "pch.h"
#include "PluginProvider.h"
#include "PluginCommand.h"
#include "PluginSettings.h"
#include "SharedHwnd.h"
#include "app/LauncherApp.h"
#include "icon/IconLoader.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/Path.h"
#include <mutex>
#include <nlohmann/json.hpp>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace plugin {

namespace {
PluginProvider* gPluginProvider{nullptr};
}

namespace {

using json = nlohmann::json;

/**
  プラグインAPIから取得した文字列をCStringへ変換する
  @param[in] func 文字列取得関数
  @param[in] match プラグインの検索結果ハンドル
  @param[in] index 検索結果内のコマンド番号
  @param[out] value 取得した文字列
  @return true:取得成功 false:取得失敗
*/
bool GetStringValue(LNCRPLUGINFUNC_GETSTRING func, LNCRPLUGINMATCHHANDLE match,
	int index, CString& value)
{
	value.Empty();
	// 先に必要なバッファサイズを確認し、その後に文字列本体を取得する。
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

/**
  プラグイン情報をJSONとして解析し、APIバージョンを検証する
  @param[in] pluginInfo プラグイン情報のJSON文字列
  @param[out] parsedInfo 解析したプラグイン情報
  @return true:検証成功 false:検証失敗
*/
bool ParsePluginInfo(const char* pluginInfo, json& parsedInfo)
{
	if (pluginInfo == nullptr) {
		return false;
	}

	try {
		parsedInfo = json::parse(pluginInfo);
		if (parsedInfo.is_object() == false ||
			parsedInfo.contains("pluginApiVersion") == false ||
			parsedInfo["pluginApiVersion"].is_number_integer() == false) {
			return false;
		}
		return parsedInfo["pluginApiVersion"].get<int>() == PLUGINVERSION;
	}
	catch (const json::exception&) {
		return false;
	}
}

struct MatcherContext
{
	Pattern* mPattern;
	std::string mFirstWord;
	std::string mWholeString;
};

/**
  プラグインからの文字列比較要求をPatternへ中継する
  @param[in] ctx 検索コンテキスト
  @param[in] text 比較対象文字列
  @param[in] offset 比較開始位置
  @return Patternによる一致レベル
*/
int Match(void* ctx, const char* text, int offset)
{
	MatcherContext* context = static_cast<MatcherContext*>(ctx);
	CString value;
	UTF2UTF(std::string(text ? text : ""), value);
	return context->mPattern->Match(value, static_cast<uint32_t>(offset));
}

/**
  入力中の最初の単語をUTF-8文字列として返す
  @param[in] ctx 検索コンテキスト
  @return 最初の単語
*/
const char* GetFirstWord(void* ctx)
{
	MatcherContext* context = static_cast<MatcherContext*>(ctx);
	UTF2UTF(std::wstring(context->mPattern->GetFirstWord()), context->mFirstWord);
	return context->mFirstWord.c_str();
}

/**
  入力文字列全体をUTF-8文字列として返す
  @param[in] ctx 検索コンテキスト
  @return 入力文字列全体
*/
const char* GetWholeString(void* ctx)
{
	MatcherContext* context = static_cast<MatcherContext*>(ctx);
	UTF2UTF(std::wstring(context->mPattern->GetWholeString()), context->mWholeString);
	return context->mWholeString.c_str();
}

/**
  情報レベルのログを出力する
  @param[in] message ログメッセージ
*/
void InfoLog(const char* message)
{
	spdlog::info("{}", message ? message : "");
}

/**
  警告レベルのログを出力する
  @param[in] message ログメッセージ
*/
void WarnLog(const char* message)
{
	spdlog::warn("{}", message ? message : "");
}

/**
  エラーレベルのログを出力する
  @param[in] message ログメッセージ
*/
void ErrorLog(const char* message)
{
	spdlog::error("{}", message ? message : "");
}

/**
  アプリケーションのポップアップメッセージを表示する
  @param[in] message 表示するメッセージ
*/
void PopupMessage(const char* message)
{
	auto app = static_cast<LauncherApp*>(AfxGetApp());
	if (app) {
		app->PopupMessage(message ? message : "");
	}
}

/**
  Soyokazeのメインウインドウハンドルを取得する
  @return メインウインドウのハンドル
*/
HWND GetMainWindowHandle()
{
	SharedHwnd sharedHwnd;
	return sharedHwnd.GetHwnd();
}

/**
  ファイルパスに関連付けられたアイコンを取得する
  @param[in] path UTF-8でエンコードされたファイルパス
  @return アイコンハンドル
*/
HICON LoadIconFromPath(const char* path)
{
	CString pathString;
	UTF2UTF(std::string(path ? path : ""), pathString);
	return IconLoader::Get()->LoadIconFromPath(pathString);
}

/**
  ファイル拡張子に関連付けられたアイコンを取得する
  @param[in] fileExt UTF-8でエンコードされたファイル拡張子
  @return アイコンハンドル
*/
HICON LoadExtensionIcon(const char* fileExt)
{
	CString extension;
	UTF2UTF(std::string(fileExt ? fileExt : ""), extension);
	return IconLoader::Get()->LoadExtensionIcon(extension);
}

/**
  指定したアイコンが本体側で管理されているか確認する
  @param[in] icon 確認対象のアイコンハンドル
  @return 1:本体側で管理されている 0:管理されていない
*/
int HasIcon(HICON icon)
{
	return IconLoader::Get()->HasIcon(icon) ? 1 : 0;
}

/**
  入力中の単語数を取得する
  @param[in] ctx 検索コンテキスト
  @return 単語数
*/
int GetWordCount(void* ctx)
{
	return static_cast<MatcherContext*>(ctx)->mPattern->GetWordCount();
}

} // namespace

struct PluginProvider::PImpl : public AppPreferenceListenerIF
{
	/**
	  設定変更通知のリスナーとして登録する
	*/
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this, _T("PluginProvider"));
		PluginSettings::GetInstance()->Load();
	}

	~PImpl() override
	{
		// 設定通知を解除してから、保持しているDLLを解放する。
		AppPreference::Get()->UnregisterListener(this);
		std::lock_guard<std::mutex> lock(mMutex);
		mPlugins.clear();
	}

	// 初回起動時の処理はない。
	void OnAppFirstBoot() override {}
	// 通常起動時の処理はない。
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		PluginSettings::GetInstance()->Load();
		std::lock_guard<std::mutex> lock(mMutex);
		SortPlugins();
	}

	/**
	  アプリ終了時にプラグインを解放する
	*/
	void OnAppExit() override
	{
		// アプリ終了時にプラグインモジュールを破棄する。
		std::lock_guard<std::mutex> lock(mMutex);
		mPlugins.clear();
	}

	/**
	  プラグインディレクトリを一度だけ走査する
	*/
	void LoadPlugins()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		if (mIsLoaded) {
			return;
		}
		mIsLoaded = true;

		// 実行ファイル側のプラグインを優先してロードする。
		LoadPlugins(Path(Path::MODULEFILEDIR, _T("plugins")));
		LoadPlugins(Path(Path::APPDIR, _T("plugins")));
		SortPlugins();
	}

	void SortPlugins()
	{
		const PluginSettings* settings = PluginSettings::GetInstance();
		std::stable_sort(mPlugins.begin(), mPlugins.end(), [settings](const PluginModulePtr& left, const PluginModulePtr& right) {
			return settings->Get(right->GetId()).mPriority < settings->Get(left->GetId()).mPriority;
		});
	}

	/**
	  指定されたpluginsディレクトリ以下のプラグインをロードする
	  @param[in] pluginsPath プラグインディレクトリ
	*/
	void LoadPlugins(const Path& pluginsPath)
	{
		// plugins直下の各サブディレクトリからDLLを検索する。
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

	/**
	  指定されたDLLをプラグインとしてロードする
	  @param[in] path DLLのパス
	*/
	void LoadPlugin(const CString& path)
	{
		// DLLをロードし、公開されたエクスポートテーブルを取得する。
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
			&LoadIconFromPath,
			&LoadExtensionIcon,
			&HasIcon,
		};
		const char* pluginInfo = nullptr;
		if (plugin->mExportTable.Initialize(&launcherFunctionTable, &pluginInfo) != 0) {
			SPDLOG_WARN(_T("Plugin initialization failed: {}"), (LPCTSTR)path);
			return;
		}
		if (ParsePluginInfo(pluginInfo, plugin->mPluginInfo) == false) {
			SPDLOG_WARN(_T("Invalid plugin information: {}"), (LPCTSTR)path);
			return;
		}
		// 初期化に成功したモジュールだけを保持し、DLLの寿命を管理する。
		mPlugins.push_back(std::move(plugin));
	}

	/**
	  プラグインエクスポートテーブルに必要な関数が揃っているか確認する
	  @param[in] table 検証対象のエクスポートテーブル
	  @return true:利用可能 false:不完全
	*/
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

/**
  プラグインプロバイダを生成する
*/
PluginProvider::PluginProvider() : in(std::make_unique<PImpl>())
{
	gPluginProvider = this;
}

/**
  プラグインプロバイダを破棄する
*/
PluginProvider::~PluginProvider()
{
	if (gPluginProvider == this) {
		gPluginProvider = nullptr;
	}
}

PluginProvider* PluginProvider::GetInstance()
{
	return gPluginProvider;
}

void PluginProvider::EnumPlugins(std::vector<PluginModulePtr>& plugins)
{
	in->LoadPlugins();
	std::lock_guard<std::mutex> lock(in->mMutex);
	plugins = in->mPlugins;
}

/**
  プラグインプロバイダの名前を取得する
  @return プロバイダ名
*/
CString PluginProvider::GetName()
{
	return _T("Plugin");
}

/**
  初回のコマンド準備時にプラグインをロードする
*/
void PluginProvider::PrepareAdhocCommands()
{
	in->LoadPlugins();
}

/**
  各プラグインへ入力パターンを渡し、一致したコマンドを生成する
  @param[in] pattern 検索パターン
  @param[out] commands 検索結果を追加するリスト
*/
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
	in->SortPlugins();

	for (const auto& plugin : in->mPlugins) {
		if (PluginSettings::GetInstance()->Get(plugin->GetId()).mIsEnabled == false) {
			continue;
		}
		// 検索ハンドルは複数のPluginCommandで共有し、最後にCloseHandleする。
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

/**
  プラグインが保存対象のコマンドを持たないことを示す
  @param[out] displayNames コマンド表示名の格納先
  @return 常に0
*/
uint32_t PluginProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	UNREFERENCED_PARAMETER(displayNames);
	return 0;
}

} // namespace plugin
} // namespace commands
} // namespace launcherapp
