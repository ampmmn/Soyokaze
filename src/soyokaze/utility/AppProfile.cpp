#include "pch.h"
#include "framework.h"
#include "AppProfile.h"
#include "app/AppName.h"
#include "utility/IniFile.h"
#include "utility/Path.h"

#include <shlwapi.h>
#include <atlpath.h>
#include <wincrypt.h>


#pragma comment(lib,"crypt32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr LPCTSTR INTERMADIATE_DIRNAME = _T("per_machine");

static void GetProfileDirRoot(TCHAR* path, size_t len)
{
	GetModuleFileName(NULL, path, (DWORD)len);
	PathRemoveFileSpec(path);
	PathAppend(path, _T("profile"));
	if (Path::IsDirectory(path)) {
		// exeと同じディレクトリにprofileフォルダが存在する場合は、ポータブル版として動作する
		return;
	}
	else {
		// profileフォルダがなければ、非ポータブル版として動作する
		size_t buflen = MAX_PATH_NTFS;

		// ユーザのホームディレクトリ直下のAPP_PROFILE_DIRNAMEフォルダをユーザ設定ディレクトとする
		std::vector<TCHAR> buff(buflen);
		_tgetenv_s(&buflen, &buff.front(), buff.size(), _T("USERPROFILE"));

		_tcscpy_s(path, len, &buff.front());

		PathAppend(path, APP_PROFILE_DIRNAME);

		return;
	}
}

static void GetIntermadiateDirPath(TCHAR* path, size_t len)
{
	GetProfileDirRoot(path, len);
	PathAppend(path, INTERMADIATE_DIRNAME);
}

/**
 	ユーザ設定ディレクトリのパスを取得する
 	@return 
 	@param[out] path         パス文字列を受け取るバッファ
 	@param[in]  len          バッファの長さ
 	@param[in]  isPerMachine ディレクトリのスコープはマシン固有か?
*/
const TCHAR* CAppProfile::GetDirPath(TCHAR* path, size_t len, bool isPerMachine)
{
	TCHAR pcName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD bufLen = sizeof(pcName);
	GetComputerName(pcName, &bufLen);

	if (isPerMachine == false) {
		GetProfileDirRoot(path, len);
		return path;
	}
	else {
		GetIntermadiateDirPath(path, len);
		PathAppend(path, pcName);
		return path;
	}
}

// ファイルパスを取得
const TCHAR* CAppProfile::GetFilePath(TCHAR* path, size_t len, bool isPerMachine)
{
	GetDirPath(path, len, isPerMachine);
	PathAppend(path, _T("settings.ini"));

	return path;
}

static bool IsDirectoryEmpty(LPCTSTR path)
{
	bool hasContent = false;

	CString pattern(path);
	pattern += _T("\\*.*");

	CFileFind ff;
	BOOL b = ff.FindFile(pattern);

	while(b) {
		b = ff.FindNextFile();

		if (ff.IsDots()) {
			continue;
		}
		auto name = ff.GetFileName();
		if (name == INTERMADIATE_DIRNAME) {
			continue;
		}

		hasContent = true;
		break;
	}

	ff.Close();

	return hasContent == false;
}


//! 設定フォルダの初期化
bool CAppProfile::InitializeProfileDir(bool* isNewCreated)
{
	// ユーザ設定ディレクトリがなければ作成する
	std::vector<TCHAR> pathBuf(MAX_PATH_NTFS);
	LPTSTR path = pathBuf.data();

	// ユーザ設定ディレクトリを作成する
	GetProfileDirRoot(path, MAX_PATH_NTFS);
	if (Path::IsDirectory(path) == FALSE) {
		if (CreateDirectory(path, NULL) == FALSE) {
			return false;
		}
	}

	if (isNewCreated) {
		// ディレクトリは新規に作成されたものかどうか?
		*isNewCreated = IsDirectoryEmpty(path);
	}

	// PC固有のユーザ設定を置くための中間ディレクトリを作成する
	GetIntermadiateDirPath(path, MAX_PATH_NTFS);
	if (Path::IsDirectory(path) == FALSE) {
		if (CreateDirectory(path, NULL) == FALSE) {
			return false;
		}
	}

	// ユーザ設定(PC別)ディレクトリがなければ作成する
	bool isPerMachine = true;
	GetDirPath(path, MAX_PATH_NTFS, isPerMachine);
	if (Path::IsDirectory(path) == false) {
		if (CreateDirectory(path, NULL) == FALSE) {
			return false;
		}
	}
	return true;
}

/*!
 *	@brief デフォルトコンストラクタ
*/
 CAppProfile::CAppProfile() : m_entity(std::make_unique<CIniFile>())
{
	std::vector<TCHAR> path(MAX_PATH_NTFS);
	GetFilePath(path.data(), path.size(), false);
	m_entity->Open((LPCTSTR)path.data());
}

/*!
 *	@brief デストラクタ
*/
 CAppProfile::~CAppProfile()
{
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
	ASSERT(m_entity.get());
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
	ASSERT(m_entity.get());
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
	ASSERT(m_entity.get());

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
	ASSERT(m_entity.get());

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
	ASSERT(m_entity.get());
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
	ASSERT(m_entity.get());
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

	DWORD nBinLen = 0;
	DWORD dwSkip = 0;
	DWORD dwFlags = 0;
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


