#pragma once

#include <memory>
#include <string>

// あふw(afxw)情報取得用のラッパークラス
class AfxWWrapper
{
public:
	AfxWWrapper();
	~AfxWWrapper();

public:
	// あふの自窓のディレクトリパスを取得
	bool GetCurrentDir(std::wstring& curDir);
	// あふの自窓のディレクトリパスを変更
	bool SetCurrentDir(const std::wstring& path);
	// あふの自窓の選択ファイルパスを取得
	bool GetSelectionPath(std::wstring& path, int index);
};

