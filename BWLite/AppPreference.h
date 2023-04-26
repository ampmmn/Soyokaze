#pragma once

class AppPreference
{
public:
	AppPreference();
	~AppPreference();

	void Load();

	bool IsMatchCase() const;

	CString GetFilerPath() const;
	CString GetFilerParam() const;


public:
	// キーワードマッチングで大文字小文字を区別する
	bool mIsMatchCase;

	// ファイラー
	CString mFilerPath;
	// ファイラー実行時の引数
	CString mFilerParam;

};

