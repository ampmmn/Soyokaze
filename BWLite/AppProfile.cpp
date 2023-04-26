#include "pch.h"
#include "framework.h"
#include "AppProfile.h"
#include "IniFile.h"

#include <shlwapi.h>
#include <atlpath.h>
#include <wincrypt.h>


#pragma comment(lib,"crypt32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//! 設定情報作成用のフォルダをつくる
bool CAppProfile::CreateProfileDirectory()
{
	TCHAR path[1024];
	GetDirPath(path, 1024);
	if (PathIsDirectory(path)) {
		return true;
	}
	// フォルダなければつくる
	CString msg;
	msg.Format(_T("【初回起動】\n設定ファイルは %s 以下に作成されます。"), path);
	AfxMessageBox(msg);

	return CreateDirectory(path, NULL) != FALSE;
}

// ディレクトリパスを取得
const TCHAR* CAppProfile::GetDirPath(TCHAR* path, size_t len)
{
	size_t buflen = 1024;
	TCHAR buff[1024];
	_tgetenv_s(&buflen, buff, _T("USERPROFILE"));

	_tcscpy_s(path, len, buff);

	PathAppend(path, _T(".bwlite"));

	return path;
}

// ファイルパスを取得
const TCHAR* CAppProfile::GetFilePath(TCHAR* path, size_t len)
{
	GetDirPath(path, len);
	PathAppend(path, _T("BWLite.ini"));

	return path;
}

/*!
 *	@brief デフォルトコンストラクタ
*/
 CAppProfile::CAppProfile() : m_entity(new CIniFile())
{
	TCHAR path[1024];
	GetFilePath(path, 1024);
	m_entity->Open((LPCTSTR)path);
}

/*!
 *	@brief デストラクタ
*/
 CAppProfile::~CAppProfile()
{
	delete m_entity;
}

/*!
 *	@brief インスタンスの生成・取得
 *	@return 生成されたインスタンスへのポインタ
*/
CAppProfile* CAppProfile::Get()
{
	static CAppProfile s_inst;
	return &s_inst;
}

/*!
 *	@brief 整数値を取得します。
 *	@return 取得した整数値
 *	@param[in] section セクション名
 *	@param[in] key   キー名
 *	@param[in] def     対応する値が存在しない場合に返す値
*/
int CAppProfile::Get(
	LPCTSTR section,
	LPCTSTR key,
	int def
)
{
	ASSERT(m_entity);
	return m_entity->GetInt(section, key, def);
}

/*!
 *	@brief 整数値を設定
 *	@return thisオブジェクトへの参照を返します
 *	@param[in] section セクション名
 *	@param[in] key     キー名
 *	@param[in] value   出力する値
*/
CAppProfile& CAppProfile::Write(
	LPCTSTR section,
	LPCTSTR key,
	int value
)
{
	ASSERT(m_entity);
	m_entity->Write(section, key, value);
	return *this;
}


/*!
 *	@brief 実数値を取得します。
 *	@return 取得した実数値
 *	@param[in] section セクション名
 *	@param[in] key     キー名
 *	@param[in] def     対応する値が存在しない場合に返す値
*/
double CAppProfile::Get(
	LPCTSTR section,
	LPCTSTR key,
	double def
)
{
	ASSERT(m_entity);

	CString str;
	str.Format(_T("%g"), def);

	CString retVal = m_entity->GetString(section, key, str);
	
	double d;
	if (_stscanf_s(retVal, _T("%lf"), &d) != 0) {
		return def;
	}
	return d;
}

/*!
 *	@brief 実数値を設定
 *	@return thisオブジェクトへの参照を返します
 *	@param[in] section セクション名
 *	@param[in] key     キー名
 *	@param[in] value     出力する値
*/
CAppProfile& CAppProfile::Write(
	LPCTSTR section,
	LPCTSTR key,
	double value
)
{
	ASSERT(m_entity);

	CString str;
	str.Format(_T("%f"), value);

	m_entity->Write(section, key, str);
	return *this;
}


/*!
 *	@brief 文字列値を取得します。
 *	@return 取得した文字列値
 *	@param[in] section セクション名
 *	@param[in] key     キー名
 *	@param[in] def     対応する値が存在しない場合に返す値
*/
CString CAppProfile::Get(
	LPCTSTR section,
	LPCTSTR key,
	LPCTSTR def
)
{
	return GetString(section, key, def);
}

CString CAppProfile::GetString(
	LPCTSTR section,
	LPCTSTR key,
	LPCTSTR def
)
{
	ASSERT(m_entity);
	return m_entity->GetString(section, key, def);
}

/*!
 *	@brief 文字列を設定します。
 *	@return thisオブジェクトへの参照を返します
 *	@param[in] section セクション名
 *	@param[in] key     キー名
 *	@param[in] value   出力する値
*/
CAppProfile& CAppProfile::Write(
	LPCTSTR section,
	LPCTSTR key,
	LPCTSTR value
)
{
	return WriteString(section, key, value);
}

CAppProfile& CAppProfile::WriteString(
	LPCTSTR section,
	LPCTSTR key,
	LPCTSTR value
)
{
	ASSERT(m_entity);
	m_entity->Write(section, key, value);
	return *this;
}


//! バイト列を取得
size_t CAppProfile::Get(LPCTSTR section, LPCTSTR key, void* out, size_t len)
{
	return GetBinary(section, key, out, len);
}

//! バイト列を取得
size_t CAppProfile::GetBinary(LPCTSTR section, LPCTSTR key, void* out, size_t len)
{
	CString str = Get(section, key, _T(""));
	if (str.IsEmpty()) {
		return 0;
	}

	DWORD nBinLen;
	DWORD dwSkip, dwFlags;
	if (CryptStringToBinary(str, str.GetLength(), CRYPT_STRING_BASE64, NULL, &nBinLen, &dwSkip, &dwFlags) == FALSE) {
		return 0;
	}

	if (out == NULL) {
		return nBinLen;
	}
	if (len < nBinLen) {
		return nBinLen;
	}

	VERIFY(CryptStringToBinary(str, str.GetLength(), CRYPT_STRING_BASE64, (LPBYTE)out, &nBinLen, &dwSkip, &dwFlags));
	return nBinLen;
}

//！ バイト列を設定
CAppProfile& CAppProfile::Write(LPCTSTR section, LPCTSTR key, const void* data, size_t len)
{
	return WriteBinary(section, key, data, len);
}

//！ バイト列を設定
CAppProfile& CAppProfile::WriteBinary(LPCTSTR section, LPCTSTR key, const void* data, size_t len)
{
	DWORD nStrLen = 0;
	CryptBinaryToString((const LPBYTE)data, (DWORD)len, CRYPT_STRING_BASE64, NULL, &nStrLen);

	CString str;
	CryptBinaryToString((const LPBYTE)data, (DWORD)len, CRYPT_STRING_BASE64, str.GetBuffer(nStrLen), &nStrLen);
	str.ReleaseBuffer();
	return Write(section, key, (LPCTSTR)str);
}

/*!
 *	@brief 文字列をバイト列として保存
 *	@return 
 *	@param[in] section 
 *	@param[in] key     
 *	@param[in] value   
 */
CAppProfile& CAppProfile::WriteStringAsBinary(
	LPCTSTR section,
	LPCTSTR key,
	LPCTSTR value
)
{
	size_t n_strings = _tcslen(value) + 1;   // + 1 NUL終端込み
	size_t n_bytes = n_strings * sizeof(TCHAR);
	return WriteBinary(section, key, value, n_bytes);
}

/*!
 *	@brief バイト列を文字列として取得
 *	@return 
 *	@param[in] section 
 *	@param[in] key     
 *	@param[in] def     
 */
CString CAppProfile::GetBinaryAsString(
	LPCTSTR section,
	LPCTSTR key,
	LPCTSTR def
)
{
	size_t len_in_byte = GetBinary(section, key, NULL, 0);
	if (len_in_byte == 0) {
		return def;
	}

	size_t len_in_char = len_in_byte / sizeof(TCHAR);

	CString str;
	GetBinary(section, key, str.GetBuffer((int)len_in_char), len_in_byte);
	str.ReleaseBuffer();

	return str;
}


