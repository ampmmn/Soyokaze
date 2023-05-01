#pragma once

class AppPreference
{
public:
	AppPreference();
	~AppPreference();

	void Load();

	CString GetFilerPath() const;
	CString GetFilerParam() const;


public:
	// ファイラー
	CString mFilerPath;
	// ファイラー実行時の引数
	CString mFilerParam;

};

