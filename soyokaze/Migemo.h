#pragma once

#include <memory>

// C/Migemoのラッパー
class Migemo
{
public:
	Migemo();
	~Migemo();

public:
	// 初期化ずみか?
	bool IsInitialized();
	// 辞書データを読んでMigemoオブジェクトを生成する
	bool Open(LPCTSTR dictPath);
	// Migemoオブジェクトを破棄する
	void Close();

	// queryStrで与えたローマ字テキストを正規表現に変換する
	const CString& Query(const CString& queryStr, CString& expression);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
