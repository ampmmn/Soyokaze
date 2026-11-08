#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/output_preview/OutputPreviewCommandParam.h"
#include <memory>

// {9F66ED2A-23A7-422B-B05A-BB94BA5C53C8}
constexpr launcherapp::core::IFID IFID_OUTPUTPREVIEWEDITOR =
	{ 0x9f66ed2a, 0x23a7, 0x422b, { 0xb0, 0x5a, 0xbb, 0x94, 0xba, 0x5c, 0x53, 0xc8 } };

namespace launcherapp {
namespace commands {
namespace output_preview {

class OutputPreviewEditor : virtual public launcherapp::core::CommandEditor
{
public:
	/**
	  設定ダイアログを初期化する
	  @param[in] parentWnd 親ウインドウ
	*/
	OutputPreviewEditor(CWnd* parentWnd = nullptr);
	/**
	  エディターの内部状態を解放する
	*/
	~OutputPreviewEditor();

	/**
	  編集対象の設定値を指定する
	  @param[in] param 設定値
	*/
	void SetParam(const OutputPreviewParam& param);
	/**
	  ダイアログで編集した設定値を取得する
	  @return 編集値
	*/
	const OutputPreviewParam& GetParam() const;
	/**
	  新規作成時に使用するコマンド名を指定する
	  @param[in] name コマンド名
	*/
	void OverrideName(LPCTSTR name) override;
	/**
	  編集前のコマンド名を指定する
	  @param[in] name 元のコマンド名
	*/
	void SetOriginalName(LPCTSTR name) override;
	/**
	  設定ダイアログを表示する
	  @return OKで確定した場合はtrue
	*/
	bool DoModal() override;
	/**
	  インターフェースを問い合わせる
	  @param[in] ifid インターフェース識別子
	  @param[out] obj インターフェースポインター
	  @return 対応する場合はtrue
	*/
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** obj) override;
	/**
	  参照カウントを増やす
	  @return 参照カウント
	*/
	uint32_t AddRef() override;
	/**
	  参照カウントを減らす
	  @return 参照カウント
	*/
	uint32_t Release() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
