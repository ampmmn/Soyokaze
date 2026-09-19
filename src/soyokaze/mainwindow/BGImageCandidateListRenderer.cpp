#include "pch.h"
#include "BGImageCandidateListRenderer.h"
#include "control/ColorSettings.h"
#include "setting/AppPreference.h"
#include "utility/ScopedDCState.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Msimg32.lib")

namespace {

constexpr int POSITION_LEFT_TOP = 0;
constexpr int POSITION_RIGHT_TOP = 1;
constexpr int POSITION_CENTER = 2;
constexpr int POSITION_LEFT_BOTTOM = 3;
constexpr int POSITION_RIGHT_BOTTOM = 4;
constexpr int POSITION_LONG_SIDE = 5;
constexpr int POSITION_SHORT_SIDE = 6;

/**
  画像の透過率を有効範囲へ収める
  @param[in] alpha 透過率
  @return 0～100の範囲に収めた透過率
*/
int ClampAlpha(int alpha)
{
	return (std::max)(0, (std::min)(100, alpha));
}

}

struct BGImageCandidateListRenderer::PImpl
{
	/**
	  設定された背景画像を読み込む
	  @return true:成功 false:失敗
	*/
	bool LoadImage()
	{
		mImage.Destroy();
		mFilePath = AppPreference::Get()->GetSettings().Get(_T("BGImage:BGImageFilePath"), _T(""));
		if (mFilePath.IsEmpty()) {
			spdlog::warn(_T("Background image file path is not configured."));
			return false;
		}

		HRESULT hr = mImage.Load(mFilePath);
		if (FAILED(hr)) {
			spdlog::error(_T("Failed to load background image. path:{}, HRESULT:0x{:08X}"),
				(LPCTSTR)mFilePath, static_cast<unsigned long>(hr));
			return false;
		}

		spdlog::debug(_T("Background image loaded. path:{}, size:({},{})"),
			(LPCTSTR)mFilePath, mImage.GetWidth(), mImage.GetHeight());
		return true;
	}

	/**
	  描画用の画像バッファを破棄する
	*/
	void DestroyBuffers()
	{
		mImageBuffer.Destroy();
		mBackgroundBuffer.Destroy();
		mAlternateBackgroundBuffer.Destroy();
		mImageRect.SetRectEmpty();
	}

	/**
	  描画領域のサイズに合わせて画像と背景色のバッファを作成する
	  @param[in] width 描画領域の幅
	  @param[in] height 描画領域の高さ
	  @return true:成功 false:失敗
	*/
	bool CreateBuffers(int width, int height)
	{
		DestroyBuffers();
		if (width <= 0 || height <= 0) {
			spdlog::error("Invalid background image buffer size. size:({},{})", width, height);
			return false;
		}
		if (mImage.IsNull()) {
			spdlog::error(_T("Cannot create drawing buffers because the background image is not loaded. path:{}"),
				(LPCTSTR)mFilePath);
			return false;
		}

		BOOL result = mImageBuffer.Create(width, height, 32);
		if (result == FALSE) {
			spdlog::error("Failed to create background image buffer. size:({},{})", width, height);
			DestroyBuffers();
			return false;
		}
		result = mBackgroundBuffer.Create(width, height, 32);
		if (result == FALSE) {
			spdlog::error("Failed to create regular background color buffer. size:({},{})", width, height);
			DestroyBuffers();
			return false;
		}
		result = mAlternateBackgroundBuffer.Create(width, height, 32);
		if (result == FALSE) {
			spdlog::error("Failed to create alternate background color buffer. size:({},{})", width, height);
			DestroyBuffers();
			return false;
		}

		int imageWidth = mImage.GetWidth();
		int imageHeight = mImage.GetHeight();
		if (imageWidth <= 0 || imageHeight <= 0) {
			spdlog::error("Invalid background image size. size:({},{})", imageWidth, imageHeight);
			DestroyBuffers();
			return false;
		}
		int drawWidth = imageWidth;
		int drawHeight = imageHeight;
		if (mPosition == POSITION_LONG_SIDE || mPosition == POSITION_SHORT_SIDE) {
			bool useWidth = width >= height;
			if (mPosition == POSITION_SHORT_SIDE) {
				useWidth = width <= height;
			}

			if (useWidth) {
				drawWidth = width;
				drawHeight = (std::max)(1, MulDiv(imageHeight, drawWidth, imageWidth));
			}
			else {
				drawHeight = height;
				drawWidth = (std::max)(1, MulDiv(imageWidth, drawHeight, imageHeight));
			}
		}

		int x = 0;
		int y = 0;
		if (mPosition == POSITION_RIGHT_TOP || mPosition == POSITION_RIGHT_BOTTOM) {
			x = width - drawWidth;
		}
		else if (mPosition == POSITION_CENTER || mPosition == POSITION_LONG_SIDE || mPosition == POSITION_SHORT_SIDE) {
			x = (width - drawWidth) / 2;
		}

		if (mPosition == POSITION_LEFT_BOTTOM || mPosition == POSITION_RIGHT_BOTTOM) {
			y = height - drawHeight;
		}
		else if (mPosition == POSITION_CENTER || mPosition == POSITION_LONG_SIDE || mPosition == POSITION_SHORT_SIDE) {
			y = (height - drawHeight) / 2;
		}

		// 設定された表示位置と拡大縮小方法から、画像の描画矩形を決定する。
		mImageRect.SetRect(x, y, x + drawWidth, y + drawHeight);
		HDC imageDC = mImageBuffer.GetDC();
		if (imageDC == nullptr) {
			spdlog::error("Failed to get the background image buffer device context.");
			DestroyBuffers();
			return false;
		}
		{
			ScopedDCState dcState(imageDC);
			// HALFTONE補間で画像を拡大縮小し、最近傍補間による色の不連続を抑える。
			SetStretchBltMode(imageDC, HALFTONE);
			SetBrushOrgEx(imageDC, 0, 0, nullptr);
			result = mImage.Draw(imageDC, x, y, drawWidth, drawHeight, 0, 0, imageWidth, imageHeight);
		}
		mImageBuffer.ReleaseDC();
		if (result == FALSE) {
			spdlog::error("Failed to draw background image.");
			DestroyBuffers();
			return false;
		}

		auto colorScheme = ColorSettings::Get()->GetCurrentScheme();
		if (colorScheme == nullptr) {
			spdlog::error("Failed to get the background color scheme.");
			DestroyBuffers();
			return false;
		}
		// 背景画像の下地として通常色と交互色の2種類のバッファを用意する。
		HDC backgroundDC = mBackgroundBuffer.GetDC();
		if (backgroundDC == nullptr) {
			spdlog::error("Failed to get the regular background color buffer device context.");
			DestroyBuffers();
			return false;
		}
		CDC* dc = CDC::FromHandle(backgroundDC);
		dc->FillSolidRect(0, 0, width, height, colorScheme->GetListBackgroundColor());
		mBackgroundBuffer.ReleaseDC();

		backgroundDC = mAlternateBackgroundBuffer.GetDC();
		if (backgroundDC == nullptr) {
			spdlog::error("Failed to get the alternate background color buffer device context.");
			DestroyBuffers();
			return false;
		}
		dc = CDC::FromHandle(backgroundDC);
		dc->FillSolidRect(0, 0, width, height, colorScheme->GetListBackgroundAltColor());
		mAlternateBackgroundBuffer.ReleaseDC();

		return true;
	}

	/**
	  指定領域へ背景画像と背景色を合成する
	  @param[in] targetDC 描画先デバイスコンテキスト
	  @param[in] rect 合成対象領域
	  @param[in] isAlternateColor 交互背景色を使用する場合はtrue
	*/
	void DrawComposite(CDC* targetDC, const CRect& rect, bool isAlternateColor)
	{
		if (rect.IsRectEmpty()) {
			return;
		}

		int imageAlpha = MulDiv(255, 100 - mAlpha, 100);
		int backgroundAlpha = 255 - imageAlpha;
		BLENDFUNCTION blend{AC_SRC_OVER, 0, (BYTE)imageAlpha, 0};

		CRect imagePart;
		imagePart.IntersectRect(rect, mImageRect);
		if (imageAlpha > 0 && !imagePart.IsRectEmpty()) {
			HDC sourceDC = mImageBuffer.GetDC();
			if (sourceDC == nullptr) {
				spdlog::error("Failed to get the background image source device context.");
			}
			else {
				BOOL result = ::AlphaBlend(
					targetDC->GetSafeHdc(), imagePart.left, imagePart.top,
					imagePart.Width(), imagePart.Height(), sourceDC,
					imagePart.left, imagePart.top,
					imagePart.Width(), imagePart.Height(), blend
				);
				if (result == FALSE) {
					spdlog::error("Failed to blend background image. error:{}", GetLastError());
				}
			}
			mImageBuffer.ReleaseDC();
		}

		ATL::CImage& backgroundBuffer = isAlternateColor ?
			mAlternateBackgroundBuffer : mBackgroundBuffer;
		auto blendBackground = [&](const CRect& backgroundRect, BYTE alpha) {
			if (backgroundRect.IsRectEmpty() || alpha == 0) {
				return;
			}
			blend.SourceConstantAlpha = alpha;
			HDC sourceDC = backgroundBuffer.GetDC();
			if (sourceDC == nullptr) {
				spdlog::error("Failed to get the background color source device context.");
			}
			else {
				BOOL result = ::AlphaBlend(
					targetDC->GetSafeHdc(), backgroundRect.left, backgroundRect.top,
					backgroundRect.Width(), backgroundRect.Height(), sourceDC,
					backgroundRect.left, backgroundRect.top, backgroundRect.Width(),
					backgroundRect.Height(), blend
				);
				if (result == FALSE) {
					spdlog::error("Failed to blend background color. error:{}", GetLastError());
				}
			}
			backgroundBuffer.ReleaseDC();
		};

		if (imagePart.IsRectEmpty()) {
			blendBackground(rect, 255);
		}
		else {
			CRect top(rect.left, rect.top, rect.right, imagePart.top);
			CRect bottom(rect.left, imagePart.bottom, rect.right, rect.bottom);
			CRect left(rect.left, imagePart.top, imagePart.left, imagePart.bottom);
			CRect right(imagePart.right, imagePart.top, rect.right, imagePart.bottom);
			blendBackground(top, 255);
			blendBackground(bottom, 255);
			blendBackground(left, 255);
			blendBackground(right, 255);
			blendBackground(imagePart, (BYTE)backgroundAlpha);
		}
	}

	ATL::CImage mImage;
	ATL::CImage mImageBuffer;
	ATL::CImage mBackgroundBuffer;
	ATL::CImage mAlternateBackgroundBuffer;
	CRect mImageRect;
	CString mFilePath;
	int mAlpha{0};
	int mPosition{POSITION_LEFT_TOP};
	int mWidth{0};
	int mHeight{0};
	bool mIsReady{false};
	bool mIsEmpty{false};
	bool mIsAlternateColor{false};
};

BGImageCandidateListRenderer::BGImageCandidateListRenderer() : in(new PImpl)
{
	const auto& settings = AppPreference::Get()->GetSettings();
	in->mAlpha = ClampAlpha(settings.Get(_T("BGImage:Alpha"), 0));
	in->mPosition = settings.Get(_T("BGImage:Position"), POSITION_LEFT_TOP);
	if (in->mPosition < POSITION_LEFT_TOP || in->mPosition > POSITION_SHORT_SIDE) {
		spdlog::warn("Invalid background image position. Falling back to top-left. position:{}", in->mPosition);
		in->mPosition = POSITION_LEFT_TOP;
	}
	spdlog::debug("Background image settings initialized. alpha:{}, position:{}", in->mAlpha, in->mPosition);
	in->LoadImage();
	SetIsDrawBackground(false);
}

BGImageCandidateListRenderer::~BGImageCandidateListRenderer()
{
}

/**
  候補欄の背景色を交互に描画する設定を保持する
  @param[in] isAlternateColor 交互色を使用する場合はtrue
*/
void BGImageCandidateListRenderer::SetIsAlternateColor(bool isAlternateColor)
{
	StandardCandidateListRenderer::SetIsAlternateColor(isAlternateColor);
	in->mIsAlternateColor = isAlternateColor;
}

void BGImageCandidateListRenderer::UpdateSize(int cx, int cy)
{
	// ウインドウサイズが変わった場合は、次回描画時に画像を再配置・再描画する。
	StandardCandidateListRenderer::UpdateSize(cx, cy);
	in->mWidth = cx;
	in->mHeight = cy;
	in->mIsReady = false;
}

void BGImageCandidateListRenderer::SetIsEmpty(bool isEmpty)
{
	// 候補項目がない場合も、背景画像と交互背景色だけは描画する。
	in->mIsEmpty = isEmpty;
	StandardCandidateListRenderer::SetIsEmpty(isEmpty);
}

void BGImageCandidateListRenderer::DrawItem(CWnd* listWnd, LPDRAWITEMSTRUCT drawItemStruct)
{
	CListCtrl* candidateList = static_cast<CListCtrl*>(listWnd);
	CRect windowRect;
	candidateList->GetWindowRect(&windowRect);
	if (in->mWidth != windowRect.Width() || in->mHeight != windowRect.Height()) {
		in->mWidth = windowRect.Width();
		in->mHeight = windowRect.Height();
		in->mIsReady = false;
	}

	if (!in->mIsReady) {
		in->mIsReady = in->CreateBuffers(in->mWidth, in->mHeight);
	}

	if (!in->mIsReady) {
		// 背景画像を利用できない場合は、既存の標準描画へ戻す。
		SetIsDrawBackground(true);
		StandardCandidateListRenderer::DrawItem(listWnd, drawItemStruct);
		SetIsDrawBackground(false);
		return;
	}

	CDC* pDC = CDC::FromHandle(drawItemStruct->hDC);
	CRect itemRect = drawItemStruct->rcItem;
	itemRect.right = in->mWidth;
	if (in->mIsEmpty) {
		CRect rest = itemRect;
		int itemId = 0;
		while (rest.top < in->mHeight) {
			// ダミー項目から始まる空欄を、通常項目と同じ規則で交互に塗る。
			in->DrawComposite(pDC, rest, in->mIsAlternateColor && (itemId % 2) != 0);
			rest.OffsetRect(0, rest.Height());
			++itemId;
		}
	}
	else {
		// 背景画像を先に合成し、その後に標準レンダラーで文字や選択状態を描画する。
		in->DrawComposite(pDC, itemRect,
			in->mIsAlternateColor && (drawItemStruct->itemID % 2) != 0);
	}

	StandardCandidateListRenderer::DrawItem(listWnd, drawItemStruct);

	if (!in->mIsEmpty && drawItemStruct->itemID == (UINT)candidateList->GetItemCount() - 1) {
		CRect rest = itemRect;
		rest.OffsetRect(0, rest.Height());
		int itemId = drawItemStruct->itemID + 1;
		while (rest.top < in->mHeight) {
			// 最終候補の下に続く余白も、次の行番号から交互色を継続する。
			in->DrawComposite(pDC, rest, in->mIsAlternateColor && (itemId % 2) != 0);
			rest.OffsetRect(0, rest.Height());
			++itemId;
		}
	}
}
