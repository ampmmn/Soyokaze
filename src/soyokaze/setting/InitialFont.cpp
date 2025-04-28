#include "pch.h"
#include "InitialFont.h"
#include "utility/Path.h"
#include "utility/CharConverter.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;
using CharConverter = launcherapp::utility::CharConverter;


static bool IsFontAvailable(const CString& fontName)
{
	struct local_param {

		static int CALLBACK callback(const LOGFONT* lf, const TEXTMETRIC*, DWORD, LPARAM param) {
			// フォント名を比較
			auto thisPtr = (local_param*)param;
			if (thisPtr->mTargetName != lf->lfFaceName) {
				return 1;
			}
			thisPtr->mIsFound = true;
			return 0;
		}
		CString mTargetName;
		bool mIsFound= false;
	} param{fontName};

	HDC hdc = GetDC(nullptr);
	LOGFONT lf{0};
	lf.lfCharSet = DEFAULT_CHARSET;
	EnumFontFamiliesEx(hdc, &lf, local_param::callback, (LPARAM)&param, 0);
	ReleaseDC(nullptr, hdc);
	return param.mIsFound;
}

InitialFont::InitialFont() : mFontName{_T("Tahoma")}, mFontSize{9}
{
	// Note: initSettings.jsonに"default"の定義もない場合はコンストラクタの値が使われる

	Path path(Path::MODULEFILEDIR, _T("config/initialSettings.json"));
	if (Path::FileExists(path) == false) {
		// ファイルない
		return;
	}

	// 
	try {
		std::ifstream f(path);
		json initConfig = json::parse(f);

		if (initConfig.find("font-settings") == initConfig.end()) {
			// キーがない
			spdlog::warn("font-settings key does not exist.");
			return;
		}

		// ロケールを取得
		wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
		GetUserDefaultLocaleName(localeName, sizeof(localeName) / sizeof(localeName[0]));

		std::string loc;
		UTF2UTF(std::wstring(localeName), loc);

		std::string langCode(loc.begin(), loc.begin() + 2);

		auto fontSettings = initConfig["font-settings"];

		for (auto it = fontSettings.begin(); it != fontSettings.end(); ++it) {
			std::string key = it.key();
			if (langCode != key) {
				continue;
			}

			auto entry = it.value();

			auto itFontName = entry.find("FontName");
			auto itFontSize = entry.find("FontSize");
			if (itFontName == entry.end() || itFontSize == entry.end()) {
				spdlog::warn("FontName or FontSize key does not exist.");
				continue;
			}

			// 定義があった場合
			CharConverter::UTF2UTF(*itFontName, mFontName);
			mFontSize = itFontSize->get<int>();

			// フォントの利用可否をチェック
			if (IsFontAvailable(mFontName) == false) {
				spdlog::warn(_T("font {} is unavailable."), (LPCTSTR)mFontName);
				break;
			}

			spdlog::debug(_T("initial font name:{0}, size:{1}"), (LPCTSTR)mFontName, mFontSize);
			return;
		}

		// 定義がなかった場合は"default"の値を適用する
		auto itDefault = fontSettings.find("default");
		if (itDefault ==fontSettings.end()) {
			// ない
			spdlog::warn("default key does not exist.");
			return;
		}

		auto entry = itDefault.value();
		auto itFontName = entry.find("FontName");
		auto itFontSize = entry.find("FontSize");
		if (itFontName == entry.end() || itFontSize == entry.end()) {
			// ない
			spdlog::warn("FontName or FontSize key does not exist.");
			return;
		}
		CharConverter::UTF2UTF(*itFontName, mFontName);
		mFontSize = itFontSize->get<int>();
	}
	catch(const std::exception& e) {
		spdlog::error("Failed to parse initialSettings.json message:{}", e.what());
	}
}

InitialFont::~InitialFont()
{
}

void InitialFont::GetFontName(CString& name)
{
	name = mFontName;
}

int InitialFont::GetFontSize()
{
	return mFontSize;
}


