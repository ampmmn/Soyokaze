#pragma once

#include <memory>
#include <vector>

class DescriptionCtrl : public CStatic
{
public:
	DescriptionCtrl();
	~DescriptionCtrl() override;

	/** 測定済みの幅から使用するフォントサイズの割合を選択する */
	static int SelectScalePercent(const std::vector<int>& measuredWidths, int availableWidth);

protected:
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

	/** フォントサイズに応じた説明文の最大行幅と改行数を測定する */
	int MeasureMaxLineWidth(CDC* dc, const CString& text, int& lineCount) const;
	/** テキスト、幅、フォントに応じた描画フォントを更新または再利用する */
	bool EnsureDisplaySettings(int width) const;
	/** 現在選択されている描画フォントを返す */
	CFont* GetDisplayFont() const;
	/** 説明文の表示設定を現在のコントロール幅で更新する */
	void UpdateDisplaySettings();
};
