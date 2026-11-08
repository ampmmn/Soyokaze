#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace output_preview {

class OutputPreviewRuntime
{
public:
	struct Result
	{
		CString mName;
		CString mCommandName;
		CString mInputText;
		CString mExecutablePath;
		CString mText;
		CString mFilePath;
		int mPreviewType{0};
	};

	/**
	  共有セッション管理インスタンスを取得する
	  @return 共有インスタンス
	*/
	static OutputPreviewRuntime* Get();

	/**
	  実行結果の処理を開始する。既存セッションがある場合は開始しない
	  @param[in] resultName セッション識別名
	  @param[in] commandName 元の実行結果確認コマンド名
	  @param[in] inputText 元の入力文字列
	  @return 開始できた場合はtrue
	*/
	bool TryBegin(const CString& resultName, const CString& commandName, const CString& inputText);
	/**
	  実行中セッションに削除対象の一時ファイルを登録する
	  @param[in] resultName 結果コマンド名
	  @param[in] path 削除対象パス
	  @return 登録できた場合はtrue
	*/
	bool AddTemporaryFile(const CString& resultName, const CString& path);
	/**
	  実行結果を公開し、プレビュー待ち状態にする
	  @param[in] resultName セッション識別名
	  @param[in] text コピー用の結果テキスト
	  @param[in] filePath Webコンテンツ表示に使用するファイル
	  @param[in] previewType プレビュー方法
	  @param[in] executablePath 実行プログラムのパス
	  @return 公開できた場合はtrue
	*/
	bool Publish(
		const CString& resultName,
		const CString& text,
		const CString& filePath,
		int previewType,
		const CString& executablePath
	);
	/**
	  表示待ちの結果コマンド名を取得する
	  @param[out] resultName セッション識別名
	  @return 結果が公開済みの場合はtrue
	*/
	bool GetResultName(CString& resultName) const;
	/**
	  指定したコマンドと入力文字列の結果が表示中か判定する
	  @param[in] commandName コマンド名
	  @param[in] inputText 入力文字列
	  @return 表示中の場合はtrue
	*/
	bool IsResultForCommand(const CString& commandName, const CString& inputText) const;
	/**
	  指定した結果コマンドの表示データを取得する
	  @param[in] resultName セッション識別名
	  @param[out] result 表示データ
	  @return 取得できた場合はtrue
	*/
	bool Fetch(const CString& resultName, Result& result);
	/**
	  セッションを閉じて、登録された一時ファイルを削除する
	  @param[in] resultName セッション識別名
	*/
	void Close(const CString& resultName);
	/**
	  実行中または結果表示中のセッションがあるかを取得する
	  @return セッションがある場合はtrue
	*/
	bool IsBusy() const;

private:
	/**
	  実行時状態を初期化する
	*/
	OutputPreviewRuntime();
	/**
	  実行終了を待ち、一時ファイルを削除する
	*/
	~OutputPreviewRuntime();

	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
