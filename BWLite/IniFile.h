#pragma once

#include <vector>

/**
 * @brief  INIフォーマットファイルクラス
 * 
 * WindowsのIniファイル形式の入出力を行います
 */
class CIniFile
{
public:
	CIniFile();
	~CIniFile();

public:
	//! 
	void Open(LPCTSTR pathname);
	//! 
	void Close();
	//! 
	bool IsOpen() const;

	//! すべてのデータを削除
	void RemoveAll();
	//! 指定したセクションを空にする
	void RemoveSection(LPCTSTR section);
	//! 指定したキーを削除する
	void RemoveKey(LPCTSTR section, LPCTSTR key);

	//! キーが存在するか？
	bool HasKey(LPCTSTR section, LPCTSTR key) const;

	//! 値の出力
	void Write(LPCTSTR section, LPCTSTR key, LPCTSTR value);
	//! 値の出力
	void Write(LPCTSTR section, LPCTSTR key, int value);
	//! 値の入力
	void Read(LPCTSTR section, LPCTSTR key, CString& value, LPCTSTR defval) const;

	//! セクションのリストを取得
	void GetSectionList(std::vector<CString>& sections) const;

	//! 値の入力
	int     GetInt(LPCTSTR section, LPCTSTR key, int defval=0) const;
	//! 値の入力
	CString GetString(LPCTSTR section, LPCTSTR key, LPCTSTR defval=_T("")) const;

public:
	CString	m_pathName;	//!< Iniファイルパス
};
