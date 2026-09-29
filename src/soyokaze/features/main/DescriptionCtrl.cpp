#include "pch.h"
#include "DescriptionCtrl.h"

#include <climits>
#include <cstring>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct DescriptionCtrl::PImpl
{
	CString mCachedText;
	LOGFONT mCachedLogFont{};
	int mCachedWidth{-1};
	int mCachedDpi{-1};
	int mScalePercent{100};
	int mDisplayLineHeight{1};
	bool mIsCacheValid{false};
	bool mFitsSingleLine{true};
	CBitmap mBackBuffer;
	CSize mBackBufferSize{0, 0};
	CFont mScaledFont;
};

BEGIN_MESSAGE_MAP(DescriptionCtrl, CStatic)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

/** DescriptionCtrlを構築する */
DescriptionCtrl::DescriptionCtrl() : in(std::make_unique<PImpl>())
{
}

/** DescriptionCtrlを破棄する */
DescriptionCtrl::~DescriptionCtrl()
{
}

/** フォントサイズに応じた説明文の最大行幅と改行数を測定する */
int DescriptionCtrl::MeasureMaxLineWidth(CDC* dc, const CString& text, int& lineCount) const
{
	int maxWidth = 0;
	int textLength = text.GetLength();
	int lineStart = 0;
	lineCount = 1;

	while (lineStart <= textLength) {
		int lineEnd = lineStart;
		while (lineEnd < textLength && text[lineEnd] != _T('\r') && text[lineEnd] != _T('\n')) {
			lineEnd++;
		}

		int lineLength = lineEnd - lineStart;
		if (lineLength > 0) {
			SIZE textSize{};
			int fittedChars = 0;
			BOOL isSucceeded = GetTextExtentExPointW(
				dc->GetSafeHdc(), text.GetString() + lineStart, lineLength, INT_MAX,
				&fittedChars, nullptr, &textSize);
			if (isSucceeded != FALSE) {
				maxWidth = (std::max)(maxWidth, static_cast<int>(textSize.cx));
			}
		}

		if (lineEnd >= textLength) {
			break;
		}

		if (text[lineEnd] == _T('\r') && lineEnd + 1 < textLength && text[lineEnd + 1] == _T('\n')) {
			lineStart = lineEnd + 2;
		}
		else {
			lineStart = lineEnd + 1;
		}
		lineCount++;
	}

	return maxWidth;
}

/** 測定済みの幅から使用するフォントサイズの割合を選択する */
int DescriptionCtrl::SelectScalePercent(const std::vector<int>& measuredWidths, int availableWidth)
{
	if (measuredWidths.empty()) {
		return 100;
	}

	int scalePercent = 100;
	for (int measuredWidth : measuredWidths) {
		if (measuredWidth <= availableWidth) {
			return scalePercent;
		}
		scalePercent = (std::max)(50, scalePercent - 5);
	}

	return (std::max)(50, 100 - static_cast<int>(measuredWidths.size() - 1) * 5);
}

/** テキスト、幅、フォントに応じた描画フォントを更新または再利用する */
bool DescriptionCtrl::EnsureDisplaySettings(int width) const
{
	CString text;
	GetWindowText(text);

	CFont* normalFont = GetFont();
	if (normalFont == nullptr || normalFont->m_hObject == nullptr) {
		return false;
	}

	LOGFONT normalLogFont{};
	normalFont->GetLogFont(&normalLogFont);
	CClientDC dc(const_cast<DescriptionCtrl*>(this));
	int dpi = GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY);

	if (in->mIsCacheValid && in->mCachedText == text && in->mCachedWidth == width &&
	    in->mCachedDpi == dpi && std::memcmp(&in->mCachedLogFont, &normalLogFont, sizeof(LOGFONT)) == 0) {
		return true;
	}

	in->mScaledFont.DeleteObject();
	in->mCachedText = text;
	in->mCachedLogFont = normalLogFont;
	in->mCachedWidth = width;
	in->mCachedDpi = dpi;
	in->mScalePercent = 100;
	in->mDisplayLineHeight = 1;
	in->mFitsSingleLine = false;
	in->mIsCacheValid = true;

	CFont* previousFont = dc.SelectObject(normalFont);
	int logicalLineCount = 1;
	int normalWidth = MeasureMaxLineWidth(&dc, text, logicalLineCount);
	dc.SelectObject(previousFont);

	std::vector<int> measuredWidths;
	measuredWidths.push_back(normalWidth);
	int scalePercent = 100;
	if (normalWidth > width) {
		for (int candidatePercent = 95; candidatePercent >= 75; candidatePercent -= 5) {
			LOGFONT candidateLogFont = normalLogFont;
			candidateLogFont.lfHeight = static_cast<LONG>(candidateLogFont.lfHeight * candidatePercent / 100);
			if (candidateLogFont.lfHeight == 0) {
				candidateLogFont.lfHeight = normalLogFont.lfHeight < 0 ? -1 : 1;
			}
			if (candidateLogFont.lfWidth != 0) {
				candidateLogFont.lfWidth = static_cast<LONG>(candidateLogFont.lfWidth * candidatePercent / 100);
				if (candidateLogFont.lfWidth == 0) {
					candidateLogFont.lfWidth = normalLogFont.lfWidth < 0 ? -1 : 1;
				}
			}

			CFont candidateFont;
			if (candidateFont.CreateFontIndirect(&candidateLogFont) == FALSE) {
				break;
			}

			previousFont = dc.SelectObject(&candidateFont);
			int candidateLineCount = 1;
			int candidateWidth = MeasureMaxLineWidth(&dc, text, candidateLineCount);
			dc.SelectObject(previousFont);
			measuredWidths.push_back(candidateWidth);

			if (candidateWidth <= width) {
				break;
			}
		}

		scalePercent = SelectScalePercent(measuredWidths, width);
	}

	if (scalePercent < 100) {
		LOGFONT scaledLogFont = normalLogFont;
		scaledLogFont.lfHeight = static_cast<LONG>(scaledLogFont.lfHeight * scalePercent / 100);
		if (scaledLogFont.lfHeight == 0) {
			scaledLogFont.lfHeight = normalLogFont.lfHeight < 0 ? -1 : 1;
		}
		if (scaledLogFont.lfWidth != 0) {
			scaledLogFont.lfWidth = static_cast<LONG>(scaledLogFont.lfWidth * scalePercent / 100);
			if (scaledLogFont.lfWidth == 0) {
				scaledLogFont.lfWidth = normalLogFont.lfWidth < 0 ? -1 : 1;
			}
		}
		if (in->mScaledFont.CreateFontIndirect(&scaledLogFont) == FALSE) {
			scalePercent = 100;
		}
	}

	in->mScalePercent = scalePercent;
	CFont* displayFont = scalePercent < 100 ? &in->mScaledFont : normalFont;
	previousFont = dc.SelectObject(displayFont);
	TEXTMETRIC displayMetrics{};
	if (dc.GetTextMetrics(&displayMetrics) != FALSE) {
		in->mDisplayLineHeight = (std::max)(1, static_cast<int>(displayMetrics.tmHeight + displayMetrics.tmExternalLeading));
	}
	int displayLineCount = 1;
	int displayWidth = MeasureMaxLineWidth(&dc, text, displayLineCount);
	dc.SelectObject(previousFont);

	in->mFitsSingleLine = displayLineCount <= 1 && displayWidth <= width;
	return true;
}

/** 現在選択されている描画フォントを返す */
CFont* DescriptionCtrl::GetDisplayFont() const
{
	if (in->mScalePercent < 100 && in->mScaledFont.m_hObject != nullptr) {
		return &in->mScaledFont;
	}
	return GetFont();
}

/** 説明文の表示設定を現在のコントロール幅で更新する */
void DescriptionCtrl::UpdateDisplaySettings()
{
	CRect clientRect;
	GetClientRect(&clientRect);
	if (clientRect.IsRectEmpty() == FALSE) {
		EnsureDisplaySettings(clientRect.Width());
	}
}

/** テキスト設定後、現在の幅に合わせて表示フォントを先行計算する */
LRESULT DescriptionCtrl::OnSetText(WPARAM wParam, LPARAM lParam)
{
	CString previousText;
	GetWindowText(previousText);
	LRESULT result = DefWindowProc(WM_SETTEXT, wParam, lParam);
	CString currentText;
	GetWindowText(currentText);
	if (previousText != currentText) {
		in->mIsCacheValid = false;
		UpdateDisplaySettings();
		Invalidate(FALSE);
	}
	return result;
}

/** フォント設定後、表示フォントと再描画を更新する */
LRESULT DescriptionCtrl::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	LRESULT result = DefWindowProc(WM_SETFONT, wParam, lParam);
	in->mIsCacheValid = false;
	UpdateDisplaySettings();
	Invalidate(FALSE);
	return result;
}

/** サイズ変更後、新しい幅に合わせて表示フォントを更新する */
void DescriptionCtrl::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);
	if (cx > 0 && cy > 0) {
		EnsureDisplaySettings(cx);
		Invalidate(FALSE);
	}
}

/** バッファ描画前の背景消去を抑制する */
BOOL DescriptionCtrl::OnEraseBkgnd(CDC* dc)
{
	UNREFERENCED_PARAMETER(dc);
	return TRUE;
}

/** 説明文を選択済みフォントで描画し、必要時は2行までに制限する */
void DescriptionCtrl::OnPaint()
{
	CPaintDC dc(this);
	CRect clientRect;
	GetClientRect(&clientRect);
	if (clientRect.IsRectEmpty()) {
		return;
	}

	if (EnsureDisplaySettings(clientRect.Width()) == false) {
		return;
	}

	CString text;
	GetWindowText(text);
	CFont* displayFont = GetDisplayFont();
	if (displayFont == nullptr || displayFont->m_hObject == nullptr) {
		return;
	}

	if (in->mBackBuffer.m_hObject == nullptr || in->mBackBufferSize != clientRect.Size()) {
		in->mBackBuffer.DeleteObject();
		if (in->mBackBuffer.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height()) == FALSE) {
			return;
		}
		in->mBackBufferSize = clientRect.Size();
	}

	CDC bufferDC;
	if (bufferDC.CreateCompatibleDC(&dc) == FALSE) {
		return;
	}
	CBitmap* previousBitmap = bufferDC.SelectObject(&in->mBackBuffer);

	HBRUSH backgroundBrush = nullptr;
	CWnd* parent = GetParent();
	if (parent != nullptr) {
		backgroundBrush = reinterpret_cast<HBRUSH>(parent->SendMessage(
			WM_CTLCOLORSTATIC, reinterpret_cast<WPARAM>(bufferDC.GetSafeHdc()), reinterpret_cast<LPARAM>(GetSafeHwnd())));
	}
	if (backgroundBrush != nullptr) {
		bufferDC.FillRect(&clientRect, CBrush::FromHandle(backgroundBrush));
	}
	else {
		bufferDC.FillSolidRect(&clientRect, GetSysColor(COLOR_3DFACE));
	}
	bufferDC.SetBkMode(TRANSPARENT);

	CFont* previousFont = bufferDC.SelectObject(displayFont);
	CRect drawRect = clientRect;
	UINT drawFlags = DT_CENTER | DT_NOPREFIX;
	if (in->mFitsSingleLine) {
		drawFlags |= DT_SINGLELINE | DT_VCENTER;
	}
	else {
		drawFlags |= DT_WORDBREAK | DT_EDITCONTROL;
		drawRect.bottom = (std::min)(drawRect.bottom, drawRect.top + in->mDisplayLineHeight * 2);
	}

	bufferDC.DrawText(text, &drawRect, drawFlags);
	bufferDC.SelectObject(previousFont);
	dc.BitBlt(clientRect.left, clientRect.top, clientRect.Width(), clientRect.Height(),
		&bufferDC, 0, 0, SRCCOPY);
	bufferDC.SelectObject(previousBitmap);
}
