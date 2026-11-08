#pragma once

#include "commands/core/CommandEntryIF.h"

namespace launcherapp {
namespace commands {
namespace output_preview {

class OutputPreviewParam
{
public:
	/**
	  既定の設定値で初期化する
	*/
	OutputPreviewParam();

	/**
	  設定値を保存する
	  @param[in] entry 保存先
	  @return 保存に成功した場合はtrue
	*/
	bool Save(CommandEntryIF* entry) const;
	/**
	  設定値を復元する
	  @param[in] entry 読み込み元
	  @return 復元に成功した場合はtrue
	*/
	bool Load(CommandEntryIF* entry);
	/**
	  設定された表示方法をWindowsの表示フラグに変換する
	  @return ウインドウ表示フラグ
	*/
	int GetShowType() const;
	/**
	  終了コードに基づき結果表示を試みるか判定する
	  @param[in] exitCode プロセスの終了コード
	  @return 結果表示を試みる場合はtrue
	*/
	bool ShouldShowResultForExitCode(DWORD exitCode) const;

	CString mName;
	CString mDescription;
	CString mFilePath;
	CString mParameter;
	CString mWorkDir;
	CString mExtension;
	int mShowType{0};
	int mResultType{0};
	int mPreviewType{0};
	int mTimeout{500};
	int mForce{0};
};

}
}
}
