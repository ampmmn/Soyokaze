#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// 本体側はLNCRAPPを定義しておく
#ifdef LNCRAPP
#define LNCRPLUGIN_API __declspec(dllimport)
#else
#define LNCRPLUGIN_API __declspec(dllexport)
#endif

// プラグインのバージョン。このヘッダの定義を変更したら、数値を上げる
// プラグイン側はLNCRPLUGIN_Bind内で想定するプラグインバージョンかどうかをチェックし、対応できない場合はエラーを返す
#define PLUGINVERSION 101

// キーワードマッチ結果を表すハンドル。ハンドルの実体はプラグイン実装側が定義する内部データ。
// このため、LNCRPLUGINMATCHHANDLEという型はプラグイン間で共通となるが、異なるプラグイン間での互換性はない。
typedef void* LNCRPLUGINMATCHHANDLE;

// ランチャー本体の機能
typedef void (*LPFUNCPRINTMSG)(const char* msg);
typedef HWND (*LPFUNCMAINWINDOWHANDLE)(void);

// プラグインからランチャー本体機能にアクセスするための関数テーブル
// Initializeでこの関数テーブルを渡す。プラグインは渡された関数ポインタを保持して起き、必要に応じて関数を利用する。
// 関数定義を追加する際は構造体の末尾に追加していくこと。順序の入れ替えは基本しない
typedef struct _LAUNCHER_FUNCTION_TABLE
{
	// Infoログを出力する
	LPFUNCPRINTMSG InfoLog;
	// Warningログを出力する
	LPFUNCPRINTMSG WarnLog;
	// Errorログを出力する
	LPFUNCPRINTMSG ErrorLog;
	// トースト通知メッセージを表示する
	LPFUNCPRINTMSG PopupMessage;
	// メインウインドウのハンドルを取得する
	LPFUNCMAINWINDOWHANDLE GetMainWindowHandle;

} LAUNCHER_FUNCTION_TABLE;

// キーワード比較に関する関数
typedef int (*LNCRMATCHERFUNC_MATCH)(void* ctx, const char*, int);
typedef const char* (*LNCRMATCHERFUNC_GETFIRSTWORD)(void* ctx);
typedef const char* (*LNCRMATCHERFUNC_GETWHOLESTRING)(void* ctx);
typedef int (*LNCRMATCHERFUNC_GETWORDCOUNT)(void* ctx);

// ランチャーアプリ側が提供するキーワード比較関数
// この構造体はプラグイン呼び出し元(アプリ本体)が生成し、プラグイン側に与える情報
// 関数定義を追加する際は構造体の末尾に追加していくこと。順序の入れ替えは基本しない
typedef struct _MATCHER_FUNCTION_TABLE
{
	// キーワード比較を行う関数
	// 第1引数は呼び出し元が使う内部データ。第2引数は比較キーワード、第3引数はオフセット
	// (Pattern::Match(LPCTSTR, uint32_t)同じ意味)
	LNCRMATCHERFUNC_MATCH Match;

	// 入力中の最初のキーワードを得る
	// 第1引数は呼び出し元が使う内部データ。
	// 戻り値で最初のキーワードを返す
	LNCRMATCHERFUNC_GETFIRSTWORD GetFirstWord;

	// 入力中の全体キーワードを得る
	// 第1引数は呼び出し元が使う内部データ。
	// 戻り値で最初のキーワード全体を表す文字列を返す
	LNCRMATCHERFUNC_GETWHOLESTRING GetWholeString;

	// トークン数を得る
	// 第1引数は呼び出し元が使う内部データ。
	// 戻り値はトークン数。
	LNCRMATCHERFUNC_GETWORDCOUNT GetWordCount;
} MATCHER_FUNCTION_TABLE;

////////////////////////////////////////////////////////////////////////////////
// 関数型の定義
////////////////////////////////////////////////////////////////////////////////

// プラグイン初期化
// tableのデータは揮発性なので、プラグイン側で構造体データをコピーすること
typedef int (*LNCRPLUGINFUNC_INITIALIZE)(LAUNCHER_FUNCTION_TABLE* table);

// マッチ件数を得る
typedef int (*LNCRPLUGINFUNC_GETMATCHCOUNT)(LNCRPLUGINMATCHHANDLE h);

// 検索結果ごとの一致レベルを得る
// 5:完全一致 4:前方一致 3:部分一致 2:弱い一致 -1不一致
typedef int (*LNCRPLUGINFUNC_GETMATCHLEVEL)(LNCRPLUGINMATCHHANDLE h, int index);

// 文字列を取得する関数
// 戻り値はコピーされたバイト数。lenが0のときは\0も含めた必要なバッファ長(バイト単位)、-1はエラー
// バッファ長は\0終端を含む長さ。
// バッファ不足時は与えられた領域分だけコピーし(\0終端もする)、実際にコピーしたバイト数(つまりlen)を返すものとする
typedef int (*LNCRPLUGINFUNC_GETSTRING)(LNCRPLUGINMATCHHANDLE h, int index, char*, size_t len);

// 実行可能か
// 0:実行不可  0以外:実行可能
typedef int (*LNCRPLUGINFUNC_CANEXECUTE)(LNCRPLUGINMATCHHANDLE h, int index);

// 機能を実行する
// 戻り値は成功時0、その他は失敗
typedef int (*LNCRPLUGINFUNC_EXECUTE)(LNCRPLUGINMATCHHANDLE h, int index, int argc, char** argv);

// アイコンを得る
// 処理成功時、第2引数でアイコンハンドルを返す
// 戻り値 成功:0 失敗:1
// このAPIが返すアイコンの所有権はプラグイン側が持つ。不要になったらプラグイン側でリソースを破棄する。
// 基本的にはFinalizeで破棄すること。
typedef int (*LNCRPLUGINFUNC_GETICON)(LNCRPLUGINMATCHHANDLE h, int index, HICON*);

// 比較(結果一覧を内部で作成し、LNCRPLUGINMATCHHANDLEとして返す)
// ctxは検索状態を表す引数。呼び出し元側で設定する。呼び出し元は状態を保持するためのパラメータとして使うことができる。
// ctxはMATCHER_FUNCTION_TABLEで定義している各関数の第一引数として使われる。
// tblにはキーワードマッチングに利用できる各関数のポインタが格納される。
// プラグイン実装側はこの関数を通じてキーワードマッチングを行うことができる。
typedef LNCRPLUGINMATCHHANDLE (*LNCRPLUGINFUNC_MATCH)(void* ctx, MATCHER_FUNCTION_TABLE* tbl);

// LNCRPLUGINFUNC_MATCH関数によって生成したオブジェクトを解放する
typedef void (*LNCRPLUGINFUNC_CLOSEHANDLE)(LNCRPLUGINMATCHHANDLE);

// プラグイン終了処理
typedef void (*LNCRPLUGINFUNC_FINALIZE)(void);


////////////////////////////////////////////////////////////////////////////////
// 関数テーブル
////////////////////////////////////////////////////////////////////////////////

// この構造体はプラグイン側が生成し、呼び出し元に渡す情報
typedef struct _LNCRPLUGIN_EXPORTTABLE
{
	// プラグインの初期化
	LNCRPLUGINFUNC_INITIALIZE Initialize;
	// マッチングを行う
	LNCRPLUGINFUNC_MATCH Query;
	// マッチ件数を取得する
	LNCRPLUGINFUNC_GETMATCHCOUNT GetMatchCount;
	// 一致レベルを取得する
	LNCRPLUGINFUNC_GETMATCHLEVEL GetMatchLevel;
	// マッチング結果を破棄する
	LNCRPLUGINFUNC_CLOSEHANDLE CloseHandle;
	// コマンド名を取得する
	LNCRPLUGINFUNC_GETSTRING GetName;
	// 説明を得る
	LNCRPLUGINFUNC_GETSTRING GetDescription;
	// ガイド欄の文字列を得る
	LNCRPLUGINFUNC_GETSTRING GetGuide;
	// コマンド種別を得る
	LNCRPLUGINFUNC_GETSTRING GetTypeDisplayName;
	// 実行可能かを調べる(0:不可 1:可能)
	LNCRPLUGINFUNC_CANEXECUTE CanExecute;
	// 実行する
	LNCRPLUGINFUNC_EXECUTE Execute;
	// エラー発生時にエラー文字列を取得する
	LNCRPLUGINFUNC_GETSTRING GetErrorString;
	// アイコンハンドルを得る
	LNCRPLUGINFUNC_GETICON GetIcon;
	// プラグインを終了する
	LNCRPLUGINFUNC_FINALIZE Finalize;

} LNCRPLUGIN_EXPORTTABLE;

// プラグインを得る
// 戻り値 0:成功 0以外: 失敗
int LNCRPLUGIN_API
LNCRPLUGIN_Bind(int version, LNCRPLUGIN_EXPORTTABLE* table);

#ifdef __cplusplus
}  // extern "C"
#endif
