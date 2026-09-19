#include "pch.h"
#include "Migemo.h"
#include "utility/CharConverter.h"
#include "utility/Path.h"
#include <filesystem>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CharConverter = launcherapp::utility::CharConverter;

// C/MigemoのAPI
typedef void* (*MIGEMO_OPEN)(const char*);
typedef void* (*MIGEMO_OPEN_SDICT)(const char*);
typedef int (*MIGEMO_LOAD)(void*,int, const char*);
typedef int (*MIGEMO_SWITCH_SDICT)(void*, int);
typedef int (*MIGEMO_SAVE_SDICT)(void*, const char*);
typedef void (*MIGEMO_CLOSE)(void*);
typedef unsigned char* (*MIGEMO_QUERY)(void*, const unsigned char* query);
typedef void (*MIGEMO_RELEASE)(void*, unsigned char*);

typedef int (*MIGEMO_PROC_INT2CHAR)(unsigned int, unsigned char*);
typedef void (*MIGEMO_SETPROC_INT2CHAR)(void* object, MIGEMO_PROC_INT2CHAR proc);

static int int2char(unsigned int in, unsigned char* out);

struct Migemo::PImpl
{
	bool OpenMDict(LPCTSTR dictPath);
	bool OpenSDict(LPCTSTR dictPath);

	void* mMigemoObj{nullptr};
	bool mIsDictLoaded{false};

	HMODULE mMigemoDll{nullptr};
	MIGEMO_OPEN mMigemoOpen{nullptr};
	MIGEMO_OPEN_SDICT mMigemoOpenSdict{nullptr};
	MIGEMO_LOAD mMigemoLoad{nullptr};
	MIGEMO_SWITCH_SDICT mMigemoSwitchSdict{nullptr};
	MIGEMO_SAVE_SDICT mMigemoSaveSdict{nullptr};
	MIGEMO_CLOSE mMigemoClose{nullptr};
	MIGEMO_QUERY mMigemoQuery{nullptr};
	MIGEMO_RELEASE mMigemoRelease{nullptr};
	MIGEMO_SETPROC_INT2CHAR mMigemoSetProcInt2Char{nullptr};

};

bool Migemo::PImpl::OpenMDict(LPCTSTR dictPath)
{
	if (Path::FileExists(dictPath) == FALSE) {
		return false;
	}

	CharConverter converter(932);    // FIXME: 日本語環境でしか動作しない
	CStringA dictPathA;
	converter.Convert(CString(dictPath), dictPathA);
	mMigemoObj = mMigemoOpen(dictPathA);
	if (mMigemoObj == nullptr) {
		return false;
	}

	// このアプリで扱える正規表現にするためのエスケープ処理を追加
	mMigemoSetProcInt2Char(mMigemoObj, int2char);

	return true;
}

bool Migemo::PImpl::OpenSDict(LPCTSTR dictPath)
{
	constexpr LPCTSTR sdictFileNames[] = {
		_T("han2zen.dat"),
		_T("hira2kata.dat"),
		_T("roma2hira.dat"),
		_T("zen2han.dat"),
	};
	Path sdictDir(Path::APPDIRPERMACHINE, _T("migemo-sdict"));
	Path sdictPath(Path::APPDIRPERMACHINE, _T("migemo-sdict\\migemo-sdict"));
	bool hasSdictFiles = sdictPath.FileExists();

	// 補助辞書の有無を確認(欠けていたら従来辞書からの読み込みからやりなおし)
	for (auto fileName : sdictFileNames) {
		Path filePath(sdictDir);
		filePath.Append(fileName);
		hasSdictFiles = hasSdictFiles && filePath.FileExists();
	}

	CharConverter converter(932);    // FIXME: 日本語環境でしか動作しない

	// sdictの読み込みを試みる
	if (hasSdictFiles) {
		CStringA sdictPathA;
		converter.Convert(CString(sdictPath), sdictPathA);
		mMigemoObj = mMigemoOpenSdict(sdictPathA);
	}

	// sdictを開けなかった(あるいは補助辞書がなかった)場合は従来の辞書を読み込む
	if (mMigemoObj == nullptr) {
		if (Path::FileExists(dictPath) == FALSE) {
			return false;
		}

		CStringA dictPathA;
		converter.Convert(CString(dictPath), dictPathA);
		mMigemoObj = mMigemoOpen(dictPathA);
	}

	if (mMigemoObj == nullptr) {
		// 失敗したのでここで終了
		return false;
	}

	// このアプリで扱える正規表現にするためのエスケープ処理を追加
	mMigemoSetProcInt2Char(mMigemoObj, int2char);

	if (hasSdictFiles) {
		// sdictからロードしたのであればここで終了
		return true;
	}

	// 通常辞書からロードした場合は、辞書をsdictへ変換し、補助ファイルと一緒に次回起動用に保存する
	if (mMigemoSwitchSdict(mMigemoObj, 1) == 0) {
		// 変換失敗の場合は保存をあきらめる
		return true;
	}

	Path dictDir(dictPath);
	dictDir.RemoveFileSpec();
	std::error_code error;
	std::filesystem::create_directories(std::filesystem::path((LPCTSTR)sdictDir), error);
	if (error.value() != 0) {
		// ディレクトリ作成失敗の場合は保存をあきらめる
		return true;
	}

	// 補助辞書ファイルのコピーを行う
	bool copied = true;
	for (auto fileName : sdictFileNames) {
		Path sourcePath(dictDir);
		sourcePath.Append(fileName);
		Path destinationPath(sdictDir);
		destinationPath.Append(fileName);
		if (sourcePath.FileExists() == false) {
			copied = false;
			break;
		}
		std::filesystem::copy_file(
				std::filesystem::path((LPCTSTR)sourcePath),
				std::filesystem::path((LPCTSTR)destinationPath),
				std::filesystem::copy_options::overwrite_existing,
				error);
		if (error.value() != 0) {
			copied = false;
			break;
		}
	}

	// 最後にsdictを保存
	if (copied) {
		CStringA sdictPathA;
		converter.Convert(CString(sdictPath), sdictPathA);
		mMigemoSaveSdict(mMigemoObj, sdictPathA);
	}

	return true;
}

Migemo::Migemo() : in(std::make_unique<PImpl>())
{
	Path path(Path::MODULEFILEDIR);
	path.Append(_T("migemo.dll"));

	in->mMigemoDll = LoadLibrary(path);
	in->mIsDictLoaded = false;

	if (in->mMigemoDll) {
		auto dll = in->mMigemoDll;
		in->mMigemoOpen = (MIGEMO_OPEN)GetProcAddress(dll, "migemo_open");
		in->mMigemoOpenSdict = (MIGEMO_OPEN_SDICT)GetProcAddress(dll, "migemo_open_sdict");
		in->mMigemoLoad = (MIGEMO_LOAD)GetProcAddress(dll, "migemo_load");
		in->mMigemoSwitchSdict = (MIGEMO_SWITCH_SDICT)GetProcAddress(dll, "migemo_switch_sdict");
		in->mMigemoSaveSdict = (MIGEMO_SAVE_SDICT)GetProcAddress(dll, "migemo_save_sdict");
		in->mMigemoClose = (MIGEMO_CLOSE)GetProcAddress(dll, "migemo_close");
		in->mMigemoQuery = (MIGEMO_QUERY)GetProcAddress(dll, "migemo_query");
		in->mMigemoRelease = (MIGEMO_RELEASE)GetProcAddress(dll, "migemo_release");
		in->mMigemoSetProcInt2Char = (MIGEMO_SETPROC_INT2CHAR)GetProcAddress(dll, "migemo_setproc_int2char");
	}
}

Migemo::~Migemo()
{
	Close();

	in->mMigemoOpen = nullptr;
	in->mMigemoOpenSdict = nullptr;
	in->mMigemoLoad = nullptr;
	in->mMigemoSwitchSdict = nullptr;
	in->mMigemoSaveSdict = nullptr;
	in->mMigemoClose = nullptr;
	in->mMigemoQuery = nullptr;
	in->mMigemoRelease = nullptr;
	if (in->mMigemoDll) {
		FreeLibrary(in->mMigemoDll);
		in->mMigemoDll = nullptr;
	}
}

// 初期化ずみか?
bool Migemo::IsInitialized()
{
	return in->mIsDictLoaded;
}

static int int2char(unsigned int in, unsigned char* out)
{
	switch (in) {
	case '\\':
	case '?':
	case '.':
 	case '*':
 	case '+':
 	case '^':
 	case '[':
 	case ']':
 	case '(':
 	case ')':
 	case '{':
 	case '}':
 	case '!':
 	case '&':
 	case '$':
 	case '|':
		if (out) {
			out[0] = '\\';
			out[1] = (unsigned char)(in & 0xFF);
		}
		return 2;
	default:
		return CharConverter::ScalarToUTF8(in, (char*)out);
	}
}

// 辞書データを読んでMigemoオブジェクトを生成する
bool Migemo::Open(LPCTSTR dictPath)
{
	if (in->mMigemoObj != nullptr) {
		return true;
	}

	// DLLからAPIを取得できていなければエラー
	if (in->mMigemoOpen == nullptr || in->mMigemoLoad == nullptr) {
		return false;
	}

	// sdict対応APIが利用できる場合はsdict対応の経路を使う
	bool canUseSdict = in->mMigemoOpenSdict != nullptr &&
		in->mMigemoSwitchSdict != nullptr && in->mMigemoSaveSdict != nullptr;
	if (canUseSdict) {
		if (in->OpenSDict(dictPath) == false) {
			return false;
		}
	} else {
		if (in->OpenMDict(dictPath) == false) {
			return false;
		}
	}

	in->mIsDictLoaded = true;

	return true;
}

// Migemoオブジェクトを破棄する
void Migemo::Close()
{
	if (in->mMigemoClose == nullptr) {
		return;
	}
	if (in->mMigemoObj == nullptr) {
		return;
	}

	in->mMigemoClose(in->mMigemoObj);
	in->mMigemoObj = nullptr;
	in->mIsDictLoaded = false;
}

// queryStrで与えたローマ字テキストを正規表現に変換する
const CString& Migemo::Query(const CString& queryStr, CString& expression)
{
	if (in->mIsDictLoaded == false) {
		return queryStr;
	}

	CStringA queryStrA;
	CharConverter converter;
	converter.Convert(queryStr, queryStrA);

	unsigned char* result = 
		in->mMigemoQuery(in->mMigemoObj, (const unsigned char*)(LPCSTR)queryStrA);

	converter.Convert((const char*)result, expression);

	in->mMigemoRelease(in->mMigemoObj, result);

	return expression;
}


