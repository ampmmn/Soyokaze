#include "pch.h"
#include "framework.h"
#include "AppPreference.h"
#include "utility/AppProfile.h"
#include <regex>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**
 * コンストラクタ
 */
AppPreference::AppPreference()
{
	Load();
}

/**
 * デストラクタ
 */
AppPreference::~AppPreference()
{
}


AppPreference* AppPreference::Get()
{
	static AppPreference thePrefObj;
	return &thePrefObj;
}

static void TrimComment(CString& s)
{
	bool inDQ = false;

	int n = s.GetLength();
	for (int i = 0; i < n; ++i) {
		if (inDQ == false && s[i] == _T('#')) {
			s = s.Left(i);
			return;
		}

		if (inDQ == false && s[i] == _T('"')) {
			inDQ = true;
		}
		if (inDQ != true && s[i] == _T('"')) {
			inDQ = false;
		}
	}
}

/**
 * 設定値を読み込む
 */
void AppPreference::Load()
{
	TCHAR path[32768];
	CAppProfile::GetFilePath(path, 32768);

	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, path, _T("r,ccs=UTF-8")) != 0) {
		return;
	}

	Settings settings;

	std::wregex regInt(L"^ *-?[0-9]+ *$");
	std::wregex regDouble(L"^ *-?[0-9]+\\.[0-9]+ *$");

	// ファイルを読む
	CStdioFile file(fpIn);

	CString strCurSectionName;

	CString strCommandName;

	CString strLine;
	while(file.ReadString(strLine)) {

		TrimComment(strLine);
		strLine.Trim();

		if (strLine.IsEmpty()) {
			continue;
		}

		if (strLine[0] == _T('[')) {
			strCurSectionName = strLine.Mid(1, strLine.GetLength()-2);
			continue;
		}

		int n = strLine.Find(_T('='));
		if (n == -1) {
			continue;
		}

		CString strKey = strLine.Left(n);
		strKey.Trim();

		CString strValue = strLine.Mid(n+1);
		strValue.Trim();
		std::wstring pat(strValue);

		CString key(strCurSectionName + _T(":") + strKey);

		if (strValue == _T("true")) {
			settings.Set(key, true);
		}
		else if (strValue == _T("false")) {
			settings.Set(key, true);
		}
		else if (std::regex_match(pat, regDouble)) {
			double value;
			_stscanf_s(strValue, _T("%lg"), &value);
			settings.Set(key, value);
		}
		else if (std::regex_match(pat, regInt)) {
			int value;
			_stscanf_s(strValue, _T("%d"), &value);
			settings.Set(key, value);
		}
		else {
			if (strValue.Left(1) == _T('"') && strValue.Right(1) == _T('"')) {
				strValue = strValue.Mid(1, strValue.GetLength()-2);
			}
			settings.Set(key, strValue);
		}
	}

	file.Close();
	fclose(fpIn);

	mSettings.Swap(settings);

	return ;
}

void AppPreference::Save()
{
	TCHAR path[32768];
	CAppProfile::GetFilePath(path, 32768);

	FILE* fpOut = nullptr;
	try {
		CString filePathTmp(path);
		filePathTmp += _T(".tmp");

		if (_tfopen_s(&fpOut, filePathTmp, _T("w,ccs=UTF-8")) != 0) {
			return ;
		}

		CStdioFile file(fpOut);

		std::set<CString> keys;
		mSettings.EnumKeys(keys);

		std::map<CString, std::vector<CString> > sectionMap;

		// セクションごとにキー名をまとめる
		for (auto key : keys) {

			CString strSection;
			CString strKey;

			int pos = key.Find(_T(":"));
			if (pos != -1) {
				strSection = key.Left(pos);
				strKey = key.Mid(pos+1);
			}
			else {
				strSection = _T("Soyokaze");
				strKey = key;
			}

			sectionMap[strSection].push_back(strKey);
		}

		CString line;
		for (auto& item : sectionMap) {
			const auto& section = item.first;
			const auto& keys = item.second;

			line.Format(_T("[%s]\n"), section);
			file.WriteString(line);

			for (auto& key : keys) {

				CString fullKey(section + _T(":") + key);

				int type = mSettings.GetType(fullKey);
				if (type == Settings::TYPE_INT) {
					int value = mSettings.Get(fullKey, 0);
					line.Format(_T("%s=%d\n"), key, value);
				}
				else if (type == Settings::TYPE_DOUBLE) {
					double value = mSettings.Get(fullKey, 0.0);
					line.Format(_T("%s=%g\n"), key, value);
				}
				else if (type == Settings::TYPE_BOOLEAN) {
					bool value = mSettings.Get(fullKey, false);
					line.Format(_T("%s=%s\n"), key, value ? _T("true") : _T("false"));
				}
				else if (type == Settings::TYPE_STRING) {
					auto value = mSettings.Get(fullKey, _T(""));
					line.Format(_T("%s=\"%s\"\n"), key, value);
				}
				file.WriteString(line);
			}
		}

		file.Close();
		fclose(fpOut);

		// 最後に一時ファイルを書き戻す
		if (CopyFile(filePathTmp, path, FALSE)) {
			// 一時ファイルを消す
			DeleteFile(filePathTmp);
		}

		// リスナーへ通知
		for (auto listener : mListeners) {
			listener->OnAppPreferenceUpdated();
		}
	}
	catch(CFileException* e) {
		e->Delete();
		fclose(fpOut);
	}
}


CString AppPreference::GetFilerPath()
{
	return mSettings.Get(_T("Soyokaze:FilerPath"), _T(""));
}

CString AppPreference::GetFilerParam()
{
	return mSettings.Get(_T("Soyokaze:FilerParam"), _T(""));
}

bool AppPreference::IsUseFiler()
{
	return mSettings.Get(_T("Soyokaze:UseFiler"), false);
}

int AppPreference::GetMatchLevel()
{
	return mSettings.Get(_T("Soyokaze:MatchLevel"), 1);
}

bool AppPreference::IsTopMost()
{
	return mSettings.Get(_T("Soyokaze:TopMost"), false);
}

bool AppPreference::IsShowToggle()
{
	return mSettings.Get(_T("Soyokaze:ShowToggle"), true);
}

bool AppPreference::IsWindowTransparencyEnable()
{
	return mSettings.Get(_T("WindowTransparency:Enable"), false);
}

int AppPreference::GetAlpha()
{
	return mSettings.Get(_T("WindowTransparency:Alpha"), 128);
}

bool AppPreference::IsTransparencyInactiveOnly()
{
	return mSettings.Get(_T("WindowTransparency:InactiveOnly"), true);
}

UINT AppPreference::GetModifiers()
{
	return mSettings.Get(_T("HotKey:Modifiers"), 1);  // ALT
}

UINT AppPreference::GetVirtualKeyCode()
{
	return mSettings.Get(_T("HotKey:VirtualKeyCode"), 32);  // SPACE
}


void AppPreference::SetSettings(const Settings& settings)
{
	std::unique_ptr<Settings> tmp(settings.Clone());
	tmp->Swap(mSettings);

}

const Settings& AppPreference::GetSettings()
{
	return mSettings;
}

void AppPreference::RegisterListener(AppPreferenceListenerIF* listener)
{
	mListeners.insert(listener);
}

void AppPreference::UnregisterListener(AppPreferenceListenerIF* listener)
{
	mListeners.erase(listener);
}

