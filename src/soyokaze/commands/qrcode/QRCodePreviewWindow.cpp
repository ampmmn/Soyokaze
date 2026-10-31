#include "pch.h"
#include "QRCodePreviewWindow.h"
#include "QRCodeGenerator.h"
#include "SharedHwnd.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace qrcode {

namespace {

class QRCodePreviewWnd : public CWnd
{
public:
	/**
	  表示中のQRコードを描画する
	*/
	afx_msg void OnPaint()
	{
		CPaintDC dc(this);
		CRect rect;
		GetClientRect(&rect);
		dc.FillSolidRect(rect, RGB(255, 255, 255));
		if (mGenerator) {
			rect.DeflateRect(8, 8);
			int sideLength = (std::min)(rect.Width(), rect.Height());
			if (sideLength <= 0) {
				return;
			}

			// 描画可能領域に収まる最大の正方形を中央に配置する
			int left = rect.left + (rect.Width() - sideLength) / 2;
			int top = rect.top + (rect.Height() - sideLength) / 2;
			CRect imageRect(left, top, left + sideLength, top + sideLength);
			mGenerator->Draw(dc.GetSafeHdc(), imageRect);
		}
	}

	/**
	  描画対象のQRコードジェネレーターを設定する
	  @param[in] generator QRコード画像を保持するジェネレーター
	*/
	void SetGenerator(QRCodeGenerator* generator)
	{
		mGenerator = generator;
		if (GetSafeHwnd()) {
			Invalidate(FALSE);
		}
	}

	DECLARE_MESSAGE_MAP()

private:
	QRCodeGenerator* mGenerator{nullptr};
};

BEGIN_MESSAGE_MAP(QRCodePreviewWnd, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

} // 無名名前空間

struct QRCodePreviewWindow::PImpl
{
	/**
	  プレビュー用ウインドウを作成する
	  @return true:成功  false:失敗
	*/
	bool Create()
	{
		SharedHwnd owner;
		HWND priorFocus = ::GetFocus();
		LPCTSTR className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, nullptr,
			static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
		if (mWindow.CreateEx(WS_EX_TOOLWINDOW, className, _T("QRCodePreview"), WS_POPUP,
			CRect(0, 0, 240, 240), CWnd::FromHandle(owner.GetHwnd()), 0) == FALSE) {
			return false;
		}
		mWindow.SetGenerator(&mGenerator);
		if (IsWindow(priorFocus)) {
			::SetFocus(priorFocus);
		}
		return true;
	}

	QRCodePreviewWnd mWindow;
	QRCodeGenerator mGenerator;
	bool mIsDestroyed{false};
};

/**
  プレビューウインドウを初期化する
*/
QRCodePreviewWindow::QRCodePreviewWindow() : in(std::make_unique<PImpl>())
{
}

/**
  プレビューウインドウの内部リソースを解放する
*/
QRCodePreviewWindow::~QRCodePreviewWindow()
{
}

/**
  プレビューウインドウの共有インスタンスを取得する
  @return 共有インスタンス
*/
QRCodePreviewWindow* QRCodePreviewWindow::Get()
{
	static QRCodePreviewWindow instance;
	return &instance;
}

/**
  プレビューウインドウを表示する
*/
void QRCodePreviewWindow::Enter()
{
	if (in->mIsDestroyed) {
		return;
	}
	if (in->mWindow.GetSafeHwnd() == nullptr && in->Create() == false) {
		return;
	}
	in->mWindow.ShowWindow(SW_SHOW);
}

/**
  プレビューウインドウを非表示にする
*/
void QRCodePreviewWindow::Leave()
{
	if (in->mWindow.GetSafeHwnd()) {
		in->mWindow.ShowWindow(SW_HIDE);
	}
}

/**
  プレビューウインドウのハンドルを取得する
  @return ウインドウハンドル
*/
HWND QRCodePreviewWindow::GetHwnd()
{
	if (in->mIsDestroyed) {
		return nullptr;
	}
	if (in->mWindow.GetSafeHwnd() == nullptr && in->Create() == false) {
		return nullptr;
	}
	return in->mWindow.GetSafeHwnd();
}

bool QRCodePreviewWindow::SetCode(const CString& text, int errorCorrectionLevel)
{
	if (in->mIsDestroyed || in->mGenerator.Generate(text, errorCorrectionLevel) == false) {
		return false;
	}
	in->mWindow.SetGenerator(&in->mGenerator);
	return true;
}

/**
  プレビューウインドウを破棄する
*/
void QRCodePreviewWindow::Destroy()
{
	in->mIsDestroyed = true;
	if (in->mWindow.GetSafeHwnd()) {
		in->mWindow.DestroyWindow();
	}
}

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
