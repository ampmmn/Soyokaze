#include "pch.h"
#include "Migemo.h"
#include "utility/CharConverter.h"
#include "utility/Path.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CharConverter = launcherapp::utility::CharConverter;

// C/MigemoのAPI
typedef void* (*MIGEMO_OPEN)(const char*);
typedef int (*MIGEMO_LOAD)(void*,int, const char*);
typedef void (*MIGEMO_CLOSE)(void*);
typedef unsigned char* (*MIGEMO_QUERY)(void*, const unsigned char* query);
typedef void (*MIGEMO_RELEASE)(void*, unsigned char*);

struct Migemo::PImpl
{
	void* mMigemoObj{nullptr};
	bool mIsDictLoaded{false};

	HMODULE mMigemoDll{nullptr};
	MIGEMO_OPEN mMigemoOpen{nullptr};
	MIGEMO_LOAD mMigemoLoad{nullptr};
	MIGEMO_CLOSE mMigemoClose{nullptr};
	MIGEMO_QUERY mMigemoQuery{nullptr};
	MIGEMO_RELEASE mMigemoRelease{nullptr};

};

Migemo::Migemo() : in(std::make_unique<PImpl>())
{
	Path path(Path::MODULEFILEDIR);
	path.Append(_T("migemo.dll"));

	in->mMigemoDll = LoadLibrary(path);
	in->mIsDictLoaded = false;

	if (in->mMigemoDll) {
		auto dll = in->mMigemoDll;
		in->mMigemoOpen = (MIGEMO_OPEN)GetProcAddress(dll, "migemo_open");
		in->mMigemoLoad = (MIGEMO_LOAD)GetProcAddress(dll, "migemo_load");
		in->mMigemoClose = (MIGEMO_CLOSE)GetProcAddress(dll, "migemo_close");
		in->mMigemoQuery = (MIGEMO_QUERY)GetProcAddress(dll, "migemo_query");
		in->mMigemoRelease = (MIGEMO_RELEASE)GetProcAddress(dll, "migemo_release");
	}
}

Migemo::~Migemo()
{
	Close();

	in->mMigemoOpen = nullptr;
	in->mMigemoLoad = nullptr;
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

// 辞書データを読んでMigemoオブジェクトを生成する
bool Migemo::Open(LPCTSTR dictPath)
{
	// 辞書ファイルが存在しなければエラー
	if (Path::FileExists(dictPath) == FALSE) {
		return false;
	}

	// DLLからAPIを取得できていなければエラー
	if (in->mMigemoOpen == nullptr || in->mMigemoLoad == nullptr) {
		return false;
	}

	CharConverter converter(932);    // FIXME: 日本語環境でしか動作しない
	CStringA dictPathA;
	converter.Convert(CString(dictPath), dictPathA);

	// オブジェクトがなければ作成
	if (in->mMigemoObj == nullptr) {
		in->mMigemoObj = in->mMigemoOpen(dictPathA);
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


