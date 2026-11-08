#pragma once

#include "commands/output_preview/OutputPreviewCommandParam.h"
#include "control/SinglePageDialog.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace output_preview {

class OutputPreviewSettingDialog : public launcherapp::control::SinglePageDialog
{
public:
	/**
	  設定ダイアログを初期化する
	  @param[in] parentWnd 親ウインドウ
	*/
	OutputPreviewSettingDialog(CWnd* parentWnd = nullptr);
	~OutputPreviewSettingDialog() override;

	/**
	  表示する設定値を指定する
	  @param[in] param 設定値
	*/
	void SetParam(const OutputPreviewParam& param);
	/**
	  編集された設定値を取得する
	  @return 設定値
	*/
	const OutputPreviewParam& GetParam() const;
	/**
	  編集前のコマンド名を指定する
	  @param[in] name 元のコマンド名
	*/
	void SetOriginalName(const CString& name);

	/**
	  ダイアログのデータ交換を行う
	  @param[in,out] pDX データ交換オブジェクト
	*/
	void DoDataExchange(CDataExchange* pDX) override;
	/**
	  ダイアログを初期化する
	  @return TRUE
	*/
	BOOL OnInitDialog() override;

protected:
	/**
	  入力値を検証してから確定する
	*/
	void OnOK() override;
	DECLARE_MESSAGE_MAP()
	/**
	  入力値の変更に応じて検証結果を更新する
	*/
	afx_msg void OnUpdateStatus();
	/**
	  実行ファイル選択ダイアログを表示する
	*/
	afx_msg void OnBrowseFile();
	/**
	  作業フォルダ選択ダイアログを表示する
	*/
	afx_msg void OnBrowseWorkDir();
	/**
	  ステータスメッセージの色を設定する
	  @param[in] pDC 描画コンテキスト
	  @param[in] pWnd 対象コントロール
	  @param[in] nCtlColor コントロール種別
	  @return 使用するブラシ
	*/
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

private:
	/**
	  コマンド設定の入力値を検証する
	*/
	void UpdateStatus();

	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
