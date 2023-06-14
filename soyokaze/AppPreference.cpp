#include "pch.h"
#include "framework.h"
#include "AppPreference.h"
#include "utility/AppProfile.h"
#include "CommandHotKeyMappings.h"
#include "HotKeyAttribute.h"
#include <regex>
#include <map>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct AppPreference::PImpl
{
	PImpl();
	~PImpl();

	void OnAppPreferenceUpdated();

	std::unique_ptr<NotifyWindow> mNotifyWindow;

	Settings mSettings;

	// 設定変更時(正確にはSave時)に通知を受け取る
	std::set<AppPreferenceListenerIF*> mListeners;
};

class AppPreference::NotifyWindow
{
public:
	NotifyWindow() : mHwnd(nullptr)
	{
	}
	~NotifyWindow()
	{
		if (mHwnd) {
			DestroyWindow(mHwnd);
			mHwnd = nullptr;
		}
	}

	HWND GetHwnd() {
		return mHwnd;
	}

	bool Create() {

		CRect rc(0, 0, 0, 0);
		HINSTANCE hInst = AfxGetInstanceHandle();

		// 内部のmessage処理用の不可視のウインドウを作っておく
		HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("NotifyWindow"), 0, 
		                           rc.left, rc.top, rc.Width(), rc.Height(),
		                           NULL, NULL, hInst, NULL);
		ASSERT(hwnd);

		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);

		mHwnd = hwnd;
		return true;
	}

	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

		if (msg == WM_APP+1) {
			// 設定が更新されたことをリスナーに通知する
			AppPreference::PImpl* in = (AppPreference::PImpl*)lp;
			in->OnAppPreferenceUpdated();
			return 0;
		}

		return DefWindowProc(hwnd, msg, wp, lp);
	}

private:
	HWND mHwnd;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


AppPreference::PImpl::PImpl() : mNotifyWindow(new NotifyWindow)
{
}

AppPreference::PImpl::~PImpl()
{ 
}

void AppPreference::PImpl::OnAppPreferenceUpdated()
{
	for (auto listener : mListeners) {
		listener->OnAppPreferenceUpdated();
	}
}


/**
 * コンストラクタ
 */
AppPreference::AppPreference() : in(new PImpl)
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

void AppPreference::Init()
{
	in->mNotifyWindow->Create();
}

/**
 * 設定値を読み込む
 */
void AppPreference::Load()
{
	TCHAR path[MAX_PATH_NTFS];
	CAppProfile::GetFilePath(path, MAX_PATH_NTFS);

	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, path, _T("r,ccs=UTF-8")) != 0) {
		return;
	}

	Settings settings;

	tregex regInt(_T("^ *-?[0-9]+ *$"));
	tregex regDouble(_T("^ *-?[0-9]+\\.[0-9]+ *$"));

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
			settings.Set(key, false);
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

	in->mSettings.Swap(settings);

	return ;
}

void AppPreference::Save()
{
	TCHAR path[MAX_PATH_NTFS];
	CAppProfile::GetFilePath(path, MAX_PATH_NTFS);

	FILE* fpOut = nullptr;
	try {
		CString filePathTmp(path);
		filePathTmp += _T(".tmp");

		if (_tfopen_s(&fpOut, filePathTmp, _T("w,ccs=UTF-8")) != 0) {
			return ;
		}

		CStdioFile file(fpOut);

		std::set<CString> keys;
		in->mSettings.EnumKeys(keys);

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

				int type = in->mSettings.GetType(fullKey);
				if (type == Settings::TYPE_INT) {
					int value = in->mSettings.Get(fullKey, 0);
					line.Format(_T("%s=%d\n"), key, value);
				}
				else if (type == Settings::TYPE_DOUBLE) {
					double value = in->mSettings.Get(fullKey, 0.0);
					line.Format(_T("%s=%g\n"), key, value);
				}
				else if (type == Settings::TYPE_BOOLEAN) {
					bool value = in->mSettings.Get(fullKey, false);
					line.Format(_T("%s=%s\n"), key, value ? _T("true") : _T("false"));
				}
				else if (type == Settings::TYPE_STRING) {
					auto value = in->mSettings.Get(fullKey, _T(""));
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
		ASSERT(in->mNotifyWindow->GetHwnd());
		PostMessage(in->mNotifyWindow->GetHwnd(), WM_APP+1, 0 ,(LPARAM)in.get());
		// Saveは異なるスレッドが呼ばれうるが、通知先の処理の都合上、メインスレッドで通知をしたいので、
		// イベント投げる用のウインドウ経由でイベント通知する
	}
	catch(CFileException* e) {
		e->Delete();
		fclose(fpOut);
	}
}

void AppPreference::OnExit()
{
	// リスナーへ終了を通知
	auto cloned = in->mListeners;
	for (auto listener : cloned) {
		listener->OnAppExit();
	}
}


CString AppPreference::GetFilerPath()
{
	return in->mSettings.Get(_T("Soyokaze:FilerPath"), _T(""));
}

CString AppPreference::GetFilerParam()
{
	return in->mSettings.Get(_T("Soyokaze:FilerParam"), _T(""));
}

bool AppPreference::IsUseFiler()
{
	return in->mSettings.Get(_T("Soyokaze:UseFiler"), false);
}

bool AppPreference::IsHideOnStartup()
{
	return in->mSettings.Get(_T("Soyokaze:IsHideOnStartup"), false);
}

bool AppPreference::IsHideOnInactive()
{
	return in->mSettings.Get(_T("Soyokaze:IsHideOnInactive"), false);
}

int AppPreference::GetMatchLevel()
{
	return in->mSettings.Get(_T("Soyokaze:MatchLevel"), 1);
}

bool AppPreference::IsTopMost()
{
	return in->mSettings.Get(_T("Soyokaze:TopMost"), false);
}

bool AppPreference::IsShowToggle()
{
	return in->mSettings.Get(_T("Soyokaze:ShowToggle"), true);
}

bool AppPreference::IsWindowTransparencyEnable()
{
	return in->mSettings.Get(_T("WindowTransparency:Enable"), false);
}

int AppPreference::GetAlpha()
{
	return in->mSettings.Get(_T("WindowTransparency:Alpha"), 128);
}

bool AppPreference::IsTransparencyInactiveOnly()
{
	return in->mSettings.Get(_T("WindowTransparency:InactiveOnly"), true);
}

UINT AppPreference::GetModifiers()
{
	return in->mSettings.Get(_T("HotKey:Modifiers"), 1);  // ALT
}

UINT AppPreference::GetVirtualKeyCode()
{
	return in->mSettings.Get(_T("HotKey:VirtualKeyCode"), 32);  // SPACE
}

// 入力画面表示時にIMEをオフにするか?
bool AppPreference::IsIMEOffOnActive()
{
	return in->mSettings.Get(_T("Soyokaze:IsIMEOffOnActive"), false);
}

// ネットワークパスを無視する
bool AppPreference::IsIgnoreUNC()
{
	return in->mSettings.Get(_T("Soyokaze:IsIgnoreUNC"), false);
}


void AppPreference::SetSettings(const Settings& settings)
{
	std::unique_ptr<Settings> tmp(settings.Clone());
	tmp->Swap(in->mSettings);

}

const Settings& AppPreference::GetSettings()
{
	return in->mSettings;
}

void AppPreference::SetCommandKeyMappings(
	const CommandHotKeyMappings& keyMap
)
{
	int count = keyMap.GetItemCount();
	in->mSettings.Set(_T("CommandHotKey:ItemCount"), count);

	CString key;
	for (int i = 0; i < count; ++i) {

		int keyIdx = i + 1;

		key.Format(_T("CommandHotKey:Command%d"), keyIdx);
		in->mSettings.Set(key, keyMap.GetName(i));

		HOTKEY_ATTR hotKeyAttr;
		keyMap.GetHotKeyAttr(i, hotKeyAttr);
		key.Format(_T("CommandHotKey:Modifiers%d"), keyIdx);
		in->mSettings.Set(key, (int)hotKeyAttr.GetModifiers());

		key.Format(_T("CommandHotKey:VirtualKeyCode%d"), keyIdx);
		in->mSettings.Set(key, (int)hotKeyAttr.GetVKCode());

		key.Format(_T("CommandHotKey:IsGlobal%d"), keyIdx);
		in->mSettings.Set(key, keyMap.IsGlobal(i));
	}
}

void AppPreference::GetCommandKeyMappings(
	CommandHotKeyMappings& keyMap
)
{
	CommandHotKeyMappings tmp;

	int count = in->mSettings.Get(_T("CommandHotKey:ItemCount"), 0);

	CString key;
	for (int i = 0; i < count; ++i) {

		int keyIdx = i + 1;

		key.Format(_T("CommandHotKey:Command%d"), keyIdx);
		CString commandStr = in->mSettings.Get(key, _T(""));

		key.Format(_T("CommandHotKey:Modifiers%d"), keyIdx);
		UINT modifiers = (UINT)in->mSettings.Get(key, -1);

		key.Format(_T("CommandHotKey:VirtualKeyCode%d"), keyIdx);
		UINT vk = (UINT)in->mSettings.Get(key, -1);

		key.Format(_T("CommandHotKey:IsGlobal%d"), keyIdx);
		bool isGlobal = in->mSettings.Get(key, false);

		HOTKEY_ATTR attr(modifiers, vk);
		tmp.AddItem(commandStr, attr, isGlobal);
	}

	keyMap.Swap(tmp);
}

void AppPreference::RegisterListener(AppPreferenceListenerIF* listener)
{
	in->mListeners.insert(listener);
}

void AppPreference::UnregisterListener(AppPreferenceListenerIF* listener)
{
	in->mListeners.erase(listener);
}

bool AppPreference::CreateUserDirectory()
{
	TCHAR path[MAX_PATH_NTFS];
	CAppProfile::GetDirPath(path, MAX_PATH_NTFS);

	if (PathIsDirectory(path)) {
		return true;
	}
	// フォルダがなければつくる(初回起動とみなす)
	CString msg;
	msg.Format(_T("【初回起動】\n設定ファイルは %s 以下に作成されます。"), path);
	AfxMessageBox(msg);

	if (CreateDirectory(path, NULL) == FALSE) {
		return false;
	}

	// 初回起動によりユーザディレクトリが作成されたことをユーザに通知する
	for (auto listener : in->mListeners) {
		listener->OnAppFirstBoot();
	}

	return true;

}
