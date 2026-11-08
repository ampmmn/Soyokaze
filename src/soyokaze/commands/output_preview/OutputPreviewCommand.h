#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/SelectionBehavior.h"
#include "commands/output_preview/OutputPreviewCommandParam.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace output_preview {

class OutputPreviewCommand : public launcherapp::commands::common::UserCommandBase,
	public launcherapp::core::SelectionBehavior
{
public:
	/**
	  コマンドを初期化する
	*/
	OutputPreviewCommand();
	/**
	  コマンドの内部状態を解放する
	*/
	~OutputPreviewCommand() override;

	/**
	  コマンド種別を示す内部名を取得する
	  @return 種別名
	*/
	static CString GetType();
	/**
	  コマンド種別の表示名を取得する
	  @return 表示名
	*/
	static CString TypeDisplayName();
	/**
	  コマンド設定値を適用する
	  @param[in] param 設定値
	*/
	void SetParam(const OutputPreviewParam& param);
	/**
	  コマンド設定値を取得する
	  @return 設定値
	*/
	const OutputPreviewParam& GetParam() const;

	/**
	  インターフェースを問い合わせる
	  @param[in] ifid インターフェース識別子
	  @param[out] cmd インターフェースポインター
	  @return 対応する場合はtrue
	*/
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;
	/**
	  コマンド名を取得する
	  @return コマンド名
	*/
	CString GetName() override;
	/**
	  コマンドの説明を取得する
	  @return 説明
	*/
	CString GetDescription() override;
	/**
	  コマンド種別の表示名を取得する
	  @return 表示名
	*/
	CString GetTypeDisplayName() override;
	/**
	  設定された実行ファイルが実行可能か確認する
	  @param[out] reasonMsg 実行できない理由
	  @return 実行可能な場合はtrue
	*/
	bool CanExecute(String* reasonMsg) override;
	/**
	  実行時引数を受け付ける
	  @return true
	*/
	bool IsAcceptArguments() override;
	/**
	  修飾キーに対応した実行アクションを取得する
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
	  コマンド設定を保存する
	  @param[in] entry 保存先
	  @return 保存に成功した場合はtrue
	*/
	bool Save(CommandEntryIF* entry) override;
	/**
	  コマンド設定を復元する
	  @param[in] entry 読み込み元
	  @return 復元に成功した場合はtrue
	*/
	bool Load(CommandEntryIF* entry) override;
	/**
	  設定編集用エディターを作成する
	  @param[in] parent 親ウインドウ
	  @param[out] editor 作成したエディター
	  @return 作成に成功した場合はtrue
	*/
	bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	/**
	  エディターで変更した設定を適用する
	  @param[in] editor エディター
	  @return 適用に成功した場合はtrue
	*/
	bool Apply(launcherapp::core::CommandEditor* editor) override;
	/**
	  エディターの設定から新しいコマンドを作成する
	  @param[in] editor エディター
	  @param[out] newCmd 新しいコマンド
	  @return 作成に成功した場合はtrue
	*/
	bool CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmd) override;
	/**
	  このコマンドのホットキー設定を取得する
	  @param[out] attr ホットキー属性
	  @return ホットキー設定がある場合はtrue
	*/
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	/**
	  入力パターンに対する一致度を取得する
	  @param[in] pattern 入力パターン
	  @return 一致度
	*/
	int Match(Pattern* pattern) override;

	/**
	  コマンドが選択されたときに呼ばれる
	  @param[in] prior 直前に選択されていたコマンド
	*/
	void OnSelect(launcherapp::core::Command* prior) override;
	/**
	  コマンドの選択が解除されたときに呼ばれる
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
	  コマンド名の補完を行う
	  @param[in,out] keyword 入力キーワード
	  @param[out] startPos 選択範囲の開始位置
	  @param[out] endPos 選択範囲の終了位置
	  @return 補完した場合はtrue
	*/
	bool CompleteKeyword(CString& keyword, int& startPos, int& endPos) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
