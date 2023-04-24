#include "pch.h"
#include "framework.h"
#include "IniFile.h"

/**
 * @brief  コンストラクタ
 * 
 */
CIniFile::CIniFile()
{
}


/**
 * @brief  デストラクタ
 * 
 */
CIniFile::~CIniFile()
{
}


/**
 * @brief  INIファイルを使用可能な状態にする
 * 
 * @param  pathname ファイルパス
 */
void CIniFile::Open(LPCTSTR pathname)
{
	ASSERT( pathname != NULL );

	m_pathName = pathname;
}


/**
 * @brief  ファイルを閉じる
 */
void CIniFile::Close()
{
	m_pathName.Empty();
}



/**
 * @brief  Openしているか？
 * 
 * @return 
 */
bool CIniFile::IsOpen() const
{
	return !m_pathName.IsEmpty();
}


/**
 * @brief  すべてのデータを削除
 */
void CIniFile::RemoveAll()
{
	ASSERT( IsOpen() );

	std::vector<CString> sections;
	GetSectionList(sections);

	for(size_t i=0; i<sections.size(); ++i){
		RemoveSection(sections[i]);
	}
}


/**
 * @brief  指定したセクションのデータを空にする
 * 
 * @param  section セクション名
 */
void CIniFile::RemoveSection(LPCTSTR section)
{
	ASSERT( IsOpen() );

	::WritePrivateProfileString(section, NULL, NULL, m_pathName);
}


/**
 * @brief  指定したキーを削除する
 * 
 * @param  section セクション名
 * @param  key     キー名
 */
void CIniFile::RemoveKey(LPCTSTR section, LPCTSTR key)
{
	ASSERT( IsOpen() );

	::WritePrivateProfileString(section, key, NULL, m_pathName);
}


/**
 * @brief  指定したキーが存在するかを確認する
 * 
 * 
 * @param  section セクション名
 * @param  key     キー名
 * 
 * @retval true  存在する
 * @retval false 存在しない
 */
bool CIniFile::HasKey(LPCTSTR section, LPCTSTR key) const
{
	CString tmp;
	Read(section, key, tmp, _T(""));
	if( !tmp.IsEmpty() ) return true;

	Read(section, key, tmp, _T("a"));
	if( tmp != _T("a") ) return true;

	return false;
}


/**
 * @brief  値を出力する
 * 
 * @param  section セクション名
 * @param  key     キー名
 * @param  value   値
 */
void CIniFile::Write(LPCTSTR section, LPCTSTR key, LPCTSTR value)
{
	ASSERT( IsOpen() );

	::WritePrivateProfileString(section, key, value, m_pathName);
}


/**
 * @brief  値を出力する
 * 
 * @param  section セクション名
 * @param  key     キー名
 * @param  value   値
 */
void CIniFile::Write(LPCTSTR section, LPCTSTR key, int value)
{
	ASSERT( IsOpen() );

	CString str;
	str.Format(_T("%d"), value);
	Write(section, key, str);
}


/**
 * @brief  値を読み込む
 * 
 * @param  section セクション名
 * @param  key     キー名
 * @param  value   読み込みバッファ
 * @param  defval  デフォルト値
 */
void CIniFile::Read(LPCTSTR section, LPCTSTR key, CString& value, LPCTSTR defval) const
{
	ASSERT( IsOpen() );

	std::vector<TCHAR> buf;
	for(int i=1; ; ++i){
		buf.resize( 512 * i );
		DWORD res = GetPrivateProfileString(section, key, defval,
				&buf[0], static_cast<DWORD>(buf.size()), m_pathName);

		if( res < (buf.size()-1) ) break;
	}

	value = &buf[0];
}


/**
 * @brief  値を数値として取得
 * 
 * @param  section セクション名
 * @param  key     キー名
 * @param  defval  デフォルト値
 * 
 * @return 値
 */
int CIniFile::GetInt(LPCTSTR section, LPCTSTR key, int defval) const
{
	CString value, def;
	def.Format(_T("%d"), defval);
	Read(section, key, value, def);

	return _tcstol(value, NULL, 0);
}


/**
 * @brief  値を文字列として取得
 * 
 * @param  section セクション名
 * @param  key     キー名
 * @param  defval  デフォルト値
 * 
 * @return 値
 */
CString CIniFile::GetString(LPCTSTR section, LPCTSTR key, LPCTSTR defval) const
{
	CString value;
	Read(section, key, value, defval);
	return value;
}


/**
 * @brief  セクションを列挙する
 * 
 * @param  sections セクションリスト
 */
void CIniFile::GetSectionList(std::vector<CString>& sections) const
{
	ASSERT( IsOpen() );

	std::vector<TCHAR>	buf;
	for(int i=1; ; ++i){
		buf.resize( 4096 * i );

		DWORD n = GetPrivateProfileString(NULL, NULL, NULL, &buf[0], static_cast<DWORD>(buf.size()), m_pathName);
		if( n < (buf.size() - 2) ) break;
	}


	const TCHAR* p = &buf[0];
	while( *p != _T('\0') ){
		CString key = p;

		sections.push_back(p);

		p += key.GetLength() + 1;
	}
}
