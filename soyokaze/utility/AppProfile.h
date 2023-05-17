#pragma once

class CIniFile;

// アプリケーションのプロファイル情報にアクセスするためのクラス
// シングルトンクラス
class CAppProfile
{
	CAppProfile();
	~CAppProfile();
	
	// コピー禁止
	CAppProfile(const CAppProfile&);
	CAppProfile& operator = (const CAppProfile& );
public:

	//! 設定情報作成用のフォルダをつくる
	static bool CreateProfileDirectory();

	//! ディレクトリパスを取得
	static const TCHAR* GetDirPath(TCHAR* path, size_t len);
	//! ファイルパスを取得
	static const TCHAR* GetFilePath(TCHAR* path, size_t len);

	//! インスタンスの生成・取得
	static CAppProfile* Get();

	//! 整数値を取得
	int Get(LPCTSTR section, LPCTSTR key, int def);
	//! 整数値を設定
	CAppProfile& Write(LPCTSTR section, LPCTSTR key, int value);

	//! 実数値を取得
	double Get(LPCTSTR section, LPCTSTR key, double def);
	//! 実数値を設定
	CAppProfile& Write(LPCTSTR section, LPCTSTR key, double value);

	//! 文字列を取得
	CString Get(LPCTSTR section, LPCTSTR key, LPCTSTR def = _T(""));
	CString GetString(LPCTSTR section, LPCTSTR key, LPCTSTR def = _T(""));
	//! 文字列を設定
	CAppProfile& Write(LPCTSTR section, LPCTSTR key, LPCTSTR value);
	CAppProfile& WriteString(LPCTSTR section, LPCTSTR key, LPCTSTR value);

	//! バイト列を取得
	size_t Get(LPCTSTR section, LPCTSTR key, void* out, size_t len);
	size_t GetBinary(LPCTSTR section, LPCTSTR key, void* out, size_t len);

	//! バイト列を取得
	template <typename CONTAINER>
	size_t Get(LPCTSTR section, LPCTSTR key, CONTAINER& out) {
		size_t n = Get(section, key, NULL, 0);
		if (n == 0) {
			return 0;
		}
		size_t count = n / sizeof(typename CONTAINER::value_type);
		out.resize(out);
		return Get(section, key, (void*)&out.front(), n);
	}
	//！ バイト列を設定
	CAppProfile& Write(LPCTSTR section, LPCTSTR key, const void* data, size_t len);
	CAppProfile& WriteBinary(LPCTSTR section, LPCTSTR key, const void* data, size_t len);
	//! バイト列を設定
	template <typename CONTAINER>
	CAppProfile& Write(LPCTSTR section, LPCTSTR key, const CONTAINER& data) {
		size_t n_bytes = data.size() * sizeof(typename CONTAINER::value_type);
		return Write(section, key, &data.front(), n_bytes);
	}

	//! 文字列をバイト列として保存
	CAppProfile& WriteStringAsBinary(LPCTSTR section, LPCTSTR key, LPCTSTR value);
	//! バイト列を文字列として取得
	CString GetBinaryAsString(LPCTSTR section, LPCTSTR key, LPCTSTR def = _T(""));

private:
	CIniFile* m_entity;  //!< 実際の保存先オブジェクト
};

