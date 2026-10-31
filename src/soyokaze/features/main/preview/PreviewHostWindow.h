#pragma once

#include <memory>

namespace launcherapp {
namespace mainwindow {
namespace preview {

class PreviewWindow;

class PreviewHostWindow : public CWnd
{
public:
	PreviewHostWindow();
	~PreviewHostWindow() override;

	/**
	  プレビュー用の親ウインドウを作成する
	  @param[in] parent 親ウインドウ
	  @return 作成に成功した場合true
	*/
	bool Create(CWnd* parent);
	/**
	  表示するプレビューウインドウを関連付ける
	  @param[in] previewWindow 関連付けるプレビューウインドウ
	*/
	void AttachPreviewWindow(PreviewWindow* previewWindow);
	/**
	  関連付けられているプレビューウインドウがあるかを返す
	  @return 関連付けられている場合true
	*/
	bool HasPreviewWindow() const;
	/**
	  プレビュー領域を表示または非表示にする
	  @param[in] isVisible 表示する場合true
	*/
	void ShowPreview(bool isVisible);

protected:
	afx_msg void OnSize(UINT type, int cx, int cy);
	DECLARE_MESSAGE_MAP()

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
