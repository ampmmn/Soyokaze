#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/PreviewProvider.h"
#include "commands/core/SelectionBehavior.h"
#include "commands/output_preview/OutputPreviewRuntime.h"
#include <memory>

// {A29CA5E5-7CBB-4770-A663-514FAB50E1A7}
constexpr launcherapp::core::IFID IFID_OUTPUTPREVIEWRESULTCOMMAND =
	{ 0xa29ca5e5, 0x7cbb, 0x4770, { 0xa6, 0x63, 0x51, 0x4f, 0xab, 0x50, 0xe1, 0xa7 } };

namespace launcherapp {
namespace commands {
namespace output_preview {

class OutputPreviewResultCommand :
	public launcherapp::commands::common::AdhocCommandBase,
	public launcherapp::core::PreviewProvider,
	public launcherapp::core::SelectionBehavior
{
public:
	/**
	  プレビュー結果コマンドを初期化する
	  @param[in] result 実行結果
	*/
	explicit OutputPreviewResultCommand(const OutputPreviewRuntime::Result& result);
	/**
	  結果コマンドの内部状態を解放する
	*/
	~OutputPreviewResultCommand() override;

	/**
	  インターフェースを問い合わせる
	  @param[in] ifid インターフェース識別子
	  @param[out] cmd インターフェースポインター
	  @return 対応する場合はtrue
	*/
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;
	/**
	  コマンド種別の表示名を取得する
	  @return 表示名
	*/
	CString GetTypeDisplayName() override;
	/**
	  EnterまたはShift+Enterのアクションを取得する
	  @param[in] hotkeyAttr 押下キー
	  @param[out] action 生成したアクション
	  @return アクションを取得できた場合はtrue
	*/
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	/**
	  コマンドアイコンを取得する
	  @return アイコン
	*/
	HICON GetIcon() override;
	/**
	  コマンドを複製する
	  @return 複製したコマンド
	*/
	launcherapp::core::Command* Clone() override;
	/**
	  結果内容を表示するプレビューウインドウを取得する
	  @return プレビューウインドウ
	*/
	launcherapp::mainwindow::preview::PreviewWindow* FetchPreviewWindow() override;

	/**
	  コマンドが選択されたときに呼ばれる
	  @param[in] prior 直前に選択されていたコマンド
	*/
	void OnSelect(launcherapp::core::Command* prior) override;
	/**
	  コマンドの選択が解除されたときに結果セッションを閉じる
	  @param[in] next 次に選択されるコマンド
	*/
	void OnUnselect(launcherapp::core::Command* next) override;
	/**
	  コマンド実行後にウインドウを閉じる方法を取得する
	  @param[in] modifierMask 修飾キー
	  @return ウインドウの終了方針
	*/
	launcherapp::core::SelectionBehavior::CloseWindowPolicy GetCloseWindowPolicy(uint32_t modifierMask) override;
	/**
	  入力キーワードの補完を行う
	  @param[in,out] keyword 入力キーワード
	  @param[out] startPos 選択範囲の開始位置
	  @param[out] endPos 選択範囲の終了位置
	  @return 補完した場合はtrue
	*/
	bool CompleteKeyword(CString& keyword, int& startPos, int& endPos) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(OutputPreviewResultCommand)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
