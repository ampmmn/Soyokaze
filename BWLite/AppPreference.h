#pragma once

#include "AppPreferenceListenerIF.h"
#include <set>

class AppPreference
{
public:
	static AppPreference* Get();

	void Load();
	void Save();

	CString GetFilerPath() const;
	CString GetFilerParam() const;

	void RegisterListener(AppPreferenceListenerIF* listener);
	void UnregisterListener(AppPreferenceListenerIF* listener);

protected:
	AppPreference();
	~AppPreference();

public:
	// ファイラーを指定するか?
	bool mIsUseExternalFiler;
	// ファイラー
	CString mFilerPath;
	// ファイラー実行時の引数
	CString mFilerParam;
	// ランチャー表示用ホットキーの修飾キー
	UINT mModifiers;
	// ランチャー表示用ホットキー
	UINT mHotKeyVK;
	// トグル表示
	BOOL mIsShowToggle;

	// 透過表示の透明度(0-255)
	int mAlpha;
	// 透過表示をするか
	bool mIsTransparencyEnable;
	// 非アクティブのときだけ透過表示(false→常に透過)
	bool mIsTransparencyInactiveOnly;
	// ウインドウを最上位表示するか
	bool mIsTopmost;
	// 候補絞り込み方法
	int mMatchLevel;

protected:
	// 設定変更時(正確にはSave時)に通知を受け取る
	std::set<AppPreferenceListenerIF*> mListeners;
};

