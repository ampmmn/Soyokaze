#include "pch.h"
#include "MainWindowLayout.h"
#include "resource.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "SharedHwnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct MainWindowLayout::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override
	{
	}
	void OnAppNormalBoot() override {}

	void OnAppPreferenceUpdated() override
	{
		AppPreference* pref = AppPreference::Get();
		mIsShowGuide = pref->IsShowGuide();

		SharedHwnd hwnd;
		mThisPtr->RecalcControls(hwnd.GetHwnd());
	}

	void OnAppExit() override
	{
	}

	bool IsShowGuide()
	{
		if (mIsFirstCall) {
			AppPreference* pref = AppPreference::Get();
			mIsShowGuide = pref->IsShowGuide();
			mIsFirstCall = false;
		}
		return mIsShowGuide;
	}

	void MapWindowRect(HWND hwndFrom, HWND hwndTo, RECT* rc)
	{
		CPoint points[] = { CPoint(rc->left, rc->top), CPoint(rc->right, rc->bottom) };

		MapWindowPoints(hwndFrom, hwndTo, points, 2);

		rc->left = points[0].x;
		rc->top = points[0].y;
		rc->right = points[1].x;
		rc->bottom = points[1].y;
	}

	MainWindowLayout* mThisPtr = nullptr;
	bool mIsFirstCall = true;
	bool mIsShowGuide = false;

};

MainWindowLayout::MainWindowLayout() : in(new PImpl)
{
	in->mThisPtr = this;
}

MainWindowLayout::~MainWindowLayout()
{
}

void MainWindowLayout::RecalcWindowSize(HWND hwnd, UINT side, LPRECT rect)
{
	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(side);
	UNREFERENCED_PARAMETER(rect);
	// ToDo: 必要に応じて実装
}

void MainWindowLayout::RecalcControls(HWND hwnd)
{
	CWnd* parent = CWnd::FromHandle(hwnd);

	// ウインドウ全体サイズとクライアント領域の差から外枠サイズを得る
	CRect rcParentC;
	parent->GetClientRect(&rcParentC);

	CRect rcParent;
	parent->GetWindowRect(&rcParent);

	CSize sizeFrame(rcParent.Width() - rcParentC.Width(), rcParent.Height() - rcParentC.Height());

	CPoint pt;
	CRect rc;

	auto iconLabel = parent->GetDlgItem(IDC_STATIC_ICON);
	if (iconLabel == nullptr) {
		// ウインドウ初期化が終わっていない場合はコントロールを取得できないのでここで抜ける
		return;
	}

	// アイコン表示欄(移動もリサイズもしない)
	CPoint ptIcon(2,2);
	iconLabel->SetWindowPos(nullptr, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	CRect rcIcon;
	iconLabel->GetWindowRect(&rcIcon);

	// コメント欄のリサイズ
	auto comment = parent->GetDlgItem(IDC_STATIC_DESCRIPTION);

	comment->GetWindowRect(&rc);

	CPoint ptComment(ptIcon.x + rcIcon.Width() + 2, 2);

	int cx = rcParentC.Width() - ptComment.x - 2;
	int cy = rc.Height();

	spdlog::debug("comment xywh=({},{},{},{})", ptComment.x, ptComment.y, cx, cy);
	comment->SetWindowPos(nullptr, ptComment.x, ptComment.y, cx, cy, SWP_NOZORDER);
	CRect rcComment;
	comment->GetWindowRect(&rcComment);

	comment->InvalidateRect(nullptr);
	comment->UpdateWindow();

	// ガイド欄を表示している場合はガイド欄のリサイズ
	auto guide = parent->GetDlgItem(IDC_STATIC_GUIDE);

	bool isShowGuide = in->IsShowGuide();
	if (isShowGuide) {

		guide->ShowWindow(SW_SHOW);

		CPoint ptGuide(ptComment.x, ptComment.y + rcComment.Height()+1);

		// 幅,高さ(cx,cy)はコメント欄と同じ
		guide->SetWindowPos(nullptr, ptGuide.x, ptGuide.y, cx, cy, SWP_NOZORDER);

		guide->InvalidateRect(nullptr);
		guide->UpdateWindow();
	}
	else {
		guide->ShowWindow(SW_HIDE);
	}

	// 入力欄
	auto edit = parent->GetDlgItem(IDC_EDIT_COMMAND);
	edit->GetWindowRect(&rc);

	CPoint ptEdit(isShowGuide ? CPoint(ptIcon.x, ptIcon.y + rcIcon.Height() + 1)
	                          : CPoint(ptComment.x, ptComment.y + rcComment.Height() + 1));		           

	cx = rcParentC.Width() - ptEdit.x - 2;
	cy = rc.Height();

	edit->SetWindowPos(nullptr, ptEdit.x, ptEdit.y, cx, cy, SWP_NOZORDER);

	edit->InvalidateRect(nullptr);
	edit->UpdateWindow();

	CRect rcEdit;
	edit->GetWindowRect(&rcEdit);

	// 候補欄
	auto listCtrl = parent->GetDlgItem(IDC_LIST_CANDIDATE);

	listCtrl->GetWindowRect(&rc);

	CPoint ptList(ptIcon.x, ptEdit.y + rcEdit.Height() + 1);

	cx = rcParentC.Width() - ptList.x - 2;
	cy = rcParentC.Height() - ptList.y - 4;

	listCtrl->SetWindowPos(nullptr, ptList.x, ptList.y, cx, cy, SWP_NOZORDER);

	listCtrl->InvalidateRect(nullptr);
	listCtrl->UpdateWindow();
}


