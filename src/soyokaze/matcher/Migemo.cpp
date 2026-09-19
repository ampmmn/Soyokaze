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


struct Migemo::PImpl
{
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
	// DLLからAPIを取得できていなければエラー
	if (in->mMigemoOpen == nullptr || in->mMigemoLoad == nullptr) {
		return false;
	}

	CharConverter converter(932);    // FIXME: 日本語環境でしか動作しない
	constexpr LPCTSTR sdictFileNames[] = {
		_T("han2zen.dat"),
		_T("hira2kata.dat"),
		_T("roma2hira.dat"),
		_T("zen2han.dat"),
	};

	// オブジェクトがなければ作成
	if (in->mMigemoObj == nullptr) {
		// sdict対応APIが利用でき、保存済みのsdictが存在する場合は優先して読み込む
		bool canUseSdict = in->mMigemoOpenSdict != nullptr &&
			in->mMigemoSwitchSdict != nullptr && in->mMigemoSaveSdict != nullptr;
		Path sdictDir(Path::APPDIRPERMACHINE, _T("migemo-sdict"));
		Path sdictPath(Path::APPDIRPERMACHINE, _T("migemo-sdict\\migemo-sdict"));
		bool hasSdictFiles = canUseSdict && sdictPath.FileExists();
		for (auto fileName : sdictFileNames) {
			Path filePath(sdictDir);
			filePath.Append(fileName);
			hasSdictFiles = hasSdictFiles && filePath.FileExists();
		}
		if (hasSdictFiles) {
			CStringA sdictPathA;
			converter.Convert(CString(sdictPath), sdictPathA);
			in->mMigemoObj = in->mMigemoOpenSdict(sdictPathA);
		}

		// sdictを開けなかった場合は従来の辞書を読み込む
		if (in->mMigemoObj == nullptr) {
			if (Path::FileExists(dictPath) == FALSE) {
				return false;
			}

			CStringA dictPathA;
			converter.Convert(CString(dictPath), dictPathA);
			in->mMigemoObj = in->mMigemoOpen(dictPathA);
		}

		if (in->mMigemoObj == nullptr) {
			return false;
		}

		in->mMigemoSetProcInt2Char(in->mMigemoObj, int2char);

		if (canUseSdict && hasSdictFiles == false) {
			// 通常辞書をsdictへ変換し、補助ファイルと一緒に次回起動用に保存する
			if (in->mMigemoSwitchSdict(in->mMigemoObj, 1) != 0) {
				Path dictDir(dictPath);
				dictDir.RemoveFileSpec();
				std::error_code error;
				std::filesystem::create_directories(std::filesystem::path((LPCTSTR)sdictDir), error);
				if (error.value() == 0) {
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

					if (copied) {
						CStringA sdictPathA;
						converter.Convert(CString(sdictPath), sdictPathA);
						in->mMigemoSaveSdict(in->mMigemoObj, sdictPathA);
					}
				}
			}
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


