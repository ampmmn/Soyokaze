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
	std::unique_ptr<CandidateListRenderer> mRenderer;
	bool mIsDrawBackground{true};

	/**
	  設定された背景画像を読み込む
	  @return true:成功 false:失敗
	*/
	bool LoadImage()
	{
		mImage.Destroy();
		mHasLastWriteTime = false;
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

		WIN32_FILE_ATTRIBUTE_DATA fileData{};
		if (GetFileAttributesEx(mFilePath, GetFileExInfoStandard, &fileData) != FALSE) {
			mLastWriteTime = fileData.ftLastWriteTime;
			mHasLastWriteTime = true;
		}

		spdlog::debug(_T("Background image loaded. path:{}, size:({},{})"),
			(LPCTSTR)mFilePath, mImage.GetWidth(), mImage.GetHeight());
		return true;
	}

	/**
	  背景画像ファイルの更新日時が変化している場合は画像を再読み込みする
	*/
	void ReloadImageIfUpdated()
	{
		if (!mHasLastWriteTime) {
			return;
		}

		WIN32_FILE_ATTRIBUTE_DATA fileData{};
		if (GetFileAttributesEx(mFilePath, GetFileExInfoStandard, &fileData) == FALSE ||
			CompareFileTime(&mLastWriteTime, &fileData.ftLastWriteTime) == 0) {
			return;
		}

		mImage.Destroy();
		DestroyBuffers();
		mIsReady = false;
		LoadImage();
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

		BOOL result = mImageBuffer.Create(width, height, 24);
		if (result == FALSE) {
			spdlog::error("Failed to create background image buffer. size:({},{})", width, height);
			DestroyBuffers();
			return false;
		}
		result = mBackgroundBuffer.Create(width, height, 24);
		if (result == FALSE) {
			spdlog::error("Failed to create regular background color buffer. size:({},{})", width, height);
			DestroyBuffers();
			return false;
		}
		result = mAlternateBackgroundBuffer.Create(width, height, 24);
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

		CRect imagePart;
		imagePart.IntersectRect(rect, mImageRect);

		ATL::CImage& backgroundBuffer = isAlternateColor ?  mAlternateBackgroundBuffer : mBackgroundBuffer;

		// リストの描画
		HDC sourceDC = backgroundBuffer.GetDC();
		BOOL result = ::BitBlt(targetDC->GetSafeHdc(),
		                       rect.left, rect.top,
		                       rect.Width(), rect.Height(), sourceDC,
		                       rect.left, rect.top, SRCCOPY);
		if (result == FALSE) {
			spdlog::error("Failed to blend background color. error:{}", GetLastError());
		}
		backgroundBuffer.ReleaseDC();

		// アプリ設定上の透過度(0～100)をALphaBlendの値域(0～255)に正規化する
		int imageAlpha = MulDiv(255, 100 - mAlpha, 100);
		if (imageAlpha == 0 || imagePart.IsRectEmpty()) {
			return;
		}

		// 背景画像の描画
		sourceDC = mImageBuffer.GetDC();
		BLENDFUNCTION blend{AC_SRC_OVER, 0, (BYTE)imageAlpha, 0};
		result = ::AlphaBlend(targetDC->GetSafeHdc(),
		                      imagePart.left, imagePart.top,
		                      imagePart.Width(), imagePart.Height(), sourceDC,
		                      imagePart.left, imagePart.top,
		                      imagePart.Width(), imagePart.Height(), blend);
		if (result == FALSE) {
			spdlog::error("Failed to blend background image. error:{}", GetLastError());
		}
		mImageBuffer.ReleaseDC();
	}

	ATL::CImage mImage;
	ATL::CImage mImageBuffer;
	ATL::CImage mBackgroundBuffer;
	ATL::CImage mAlternateBackgroundBuffer;
	CRect mImageRect;
	CString mFilePath;
	FILETIME mLastWriteTime{};
	int mAlpha{0};
	int mPosition{POSITION_LEFT_TOP};
	int mWidth{0};
	int mHeight{0};
	bool mIsReady{false};
	bool mHasLastWriteTime{false};
	bool mIsEmpty{false};
	bool mIsAlternateColor{false};
};

BGImageCandidateListRenderer::BGImageCandidateListRenderer(
	std::unique_ptr<CandidateListRenderer> renderer
) : in(new PImpl)
{
	in->mRenderer = std::move(renderer);
	ASSERT(in->mRenderer != nullptr);

	const auto& settings = AppPreference::Get()->GetSettings();
	in->mAlpha = ClampAlpha(settings.Get(_T("BGImage:Alpha"), 0));
	in->mPosition = settings.Get(_T("BGImage:Position"), POSITION_LEFT_TOP);
	if (in->mPosition < POSITION_LEFT_TOP || in->mPosition > POSITION_SHORT_SIDE) {
		spdlog::warn("Invalid background image position. Falling back to top-left. position:{}", in->mPosition);
		in->mPosition = POSITION_LEFT_TOP;
	}
	spdlog::debug("Background image settings initialized. alpha:{}, position:{}", in->mAlpha, in->mPosition);
	in->LoadImage();
	in->mRenderer->SetIsDrawBackground(false);
}

BGImageCandidateListRenderer::~BGImageCandidateListRenderer()
{
}

/**
  描画対象の候補リストを内部レンダラーへ設定する
  @param[in] candidates 候補リスト
*/
void BGImageCandidateListRenderer::SetCandidateList(CandidateList* candidates)
{
	in->mRenderer->SetCandidateList(candidates);
}

/**
  候補欄の背景色を交互に描画する設定を保持する
  @param[in] isAlternateColor 交互色を使用する場合はtrue
*/
void BGImageCandidateListRenderer::SetIsAlternateColor(bool isAlternateColor)
{
	in->mRenderer->SetIsAlternateColor(isAlternateColor);
	in->mIsAlternateColor = isAlternateColor;
}

/**
  コマンド種別を表示するかどうかを内部レンダラーへ設定する
  @param[in] isShowCommandType 表示する場合はtrue
*/
void BGImageCandidateListRenderer::SetIsShowCommandType(bool isShowCommandType)
{
	in->mRenderer->SetIsShowCommandType(isShowCommandType);
}

/**
  アイコンを描画するかどうかを内部レンダラーへ設定する
  @param[in] isDrawIcon 描画する場合はtrue
*/
void BGImageCandidateListRenderer::SetIsDrawIcon(bool isDrawIcon)
{
	in->mRenderer->SetIsDrawIcon(isDrawIcon);
}

/**
  背景画像が利用できない場合に背景色を描画するかどうかを設定する
  @param[in] isDrawBackground 描画する場合はtrue
*/
void BGImageCandidateListRenderer::SetIsDrawBackground(bool isDrawBackground)
{
	in->mIsDrawBackground = isDrawBackground;
}

/**
  テキスト寸法とアイコンサイズを内部レンダラーへ設定する
  @param[in] textHeight テキストの高さ
  @param[in] textLineHeight 行送りを含むテキストの高さ
  @param[in] iconSize アイコンのサイズ
*/
void BGImageCandidateListRenderer::SetTextMetrics(int textHeight, int textLineHeight, int iconSize)
{
	in->mRenderer->SetTextMetrics(textHeight, textLineHeight, iconSize);
}

/**
  内部レンダラーが使用する画像リストを取得する
  @return 描画に使用する画像リスト
*/
CImageList* BGImageCandidateListRenderer::GetImageList()
{
	return in->mRenderer->GetImageList();
}

void BGImageCandidateListRenderer::UpdateSize(int cx, int cy)
{
	// ウインドウサイズが変わった場合は、次回描画時に画像を再配置・再描画する。
	in->ReloadImageIfUpdated();
	in->mRenderer->UpdateSize(cx, cy);
	in->mWidth = cx;
	in->mHeight = cy;
	in->mIsReady = false;
}

/**
  候補欄が空かどうかを内部レンダラーへ通知する
  @param[in] isEmpty 候補欄が空の場合はtrue
*/
void BGImageCandidateListRenderer::SetIsEmpty(bool isEmpty)
{
	// 候補項目がない場合も、背景画像と交互背景色だけは描画する。
	in->mIsEmpty = isEmpty;
	in->mRenderer->SetIsEmpty(isEmpty);
}

/**
  1ページ内に表示できる項目数を取得する
  @return ページ内の項目数
*/
int BGImageCandidateListRenderer::GetItemCountInPage() const
{
	return in->mRenderer->GetItemCountInPage();
}

/**
  内部レンダラーが必要とする候補項目の高さを取得する
  @return 候補項目の高さ
*/
int BGImageCandidateListRenderer::GetItemHeight() const
{
	return in->mRenderer->GetItemHeight();
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
		// 背景画像を利用できない場合は、内部レンダラーの背景付き描画へ戻す。
		in->mRenderer->SetIsDrawBackground(in->mIsDrawBackground);
		in->mRenderer->DrawItem(listWnd, drawItemStruct);
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
		// 背景画像を先に合成し、その後に内部レンダラーで文字や選択状態を描画する。
		in->DrawComposite(pDC, itemRect,
			in->mIsAlternateColor && (drawItemStruct->itemID % 2) != 0);
	}

	in->mRenderer->SetIsDrawBackground(false);
	in->mRenderer->DrawItem(listWnd, drawItemStruct);

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
