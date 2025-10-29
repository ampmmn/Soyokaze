#include "pch.h"
#include "GuideCtrl.h"
#include "commands/core/CommandIF.h"
#include "actions/core/Action.h"
#include "mainwindow/LauncherMainWindowIF.h"
#include "gui/ColorSettings.h"

using LauncherMainWindowIF= launcherapp::mainwindow::LauncherMainWindowIF;
using Command = launcherapp::core::Command;
using Action = launcherapp::actions::core::Action;

namespace launcherapp { namespace mainwindow { namespace guide {

struct GUIDE_ITEM
{
	uint32_t mModifier{0};
	CString mGuideStr;
	CRect mRect;
};

static const std::map<uint32_t, LPCTSTR>& GetKeyModifierMap()
{
	static std::map<uint32_t, LPCTSTR> entries ={
		{ 0, _T("⏎") },
		{ Command::MODIFIER_SHIFT, _T("S-⏎") },
		{ Command::MODIFIER_CTRL, _T("C-⏎") },
		{ Command::MODIFIER_ALT, _T("A-⏎") },
		{ Command::MODIFIER_WIN, _T("W-⏎") },
		{ Command::MODIFIER_SHIFT | Command::MODIFIER_CTRL, _T("S-C-⏎") },
		{ Command::MODIFIER_SHIFT | Command::MODIFIER_ALT, _T("S-A-⏎") },
		{ Command::MODIFIER_SHIFT | Command::MODIFIER_WIN, _T("S-W-⏎") },
		{ Command::MODIFIER_CTRL | Command::MODIFIER_ALT, _T("C-A-⏎") },
		{ Command::MODIFIER_CTRL | Command::MODIFIER_WIN, _T("C-W-⏎") },
		{ Command::MODIFIER_ALT | Command::MODIFIER_WIN, _T("A-W-⏎") },
	};
	return entries;
}


struct GuideCtrl::PImpl
{
	void UpdateEntries(Command* cmd);
	void UpdateBuffer();

	bool UpdateCursorState(CPoint pos);

	bool RecreateBuffer(CWnd* wnd, CSize size);

	LauncherMainWindowIF* mMainWnd{nullptr};
	std::vector<GUIDE_ITEM> mEntries;
	ATL::CImage mBuffer;
	uint32_t mLastModifierFlag{0};
	uint32_t mMouseModifier{0xffffffff};
	UINT_PTR mTimerId{0};
	UINT mNotifyMessageId{ 0 };
};

void GuideCtrl::PImpl::UpdateEntries(Command* cmd)
{
	if (cmd == nullptr) {
		mEntries.clear();
		return;
	}

	// キー別のアクション名のリストを作成する
	std::vector<GUIDE_ITEM> entries;
	for (auto item : GetKeyModifierMap()) {
		RefPtr<Action> action;
		auto modifierFlag = item.first;
		if (cmd->GetAction(modifierFlag, &action) == false) {
			continue;
		}
		if (action->IsVisible() == false) {
			continue;
		}

		GUIDE_ITEM entry{ modifierFlag, action->GetDisplayName(), CRect(0,0,0,0) };
		entries.push_back(entry);
		if (entries.size() >= 4) {
			break;
		}
	}

	mEntries.swap(entries);
}

static COLORREF GetAlterColor(COLORREF cr)
{
	BYTE rgb[] = { GetRValue(cr), GetGValue(cr), GetBValue(cr) };
	// (基準とする色から6%ほど弱めた感じにしてみる)
	if (*(std::max_element(rgb, rgb+3)) > 128) {
		// 明るい寄り(というかたいてい白のはず..)の場合は黒方向に近づける
		rgb[0] = BYTE(rgb[0] * 0.94);
		rgb[1] = BYTE(rgb[1] * 0.94);
		rgb[2] = BYTE(rgb[2] * 0.94);
	}
	else {
		// 暗い寄り(ハイコントラストモードで動いている場合とか..)の場合は白方向に近づける
		rgb[0] = BYTE(rgb[0] + BYTE((255 - rgb[0]) * 0.06));
		rgb[1] = BYTE(rgb[1] + BYTE((255 - rgb[1]) * 0.06));
		rgb[2] = BYTE(rgb[2] + BYTE((255 - rgb[2]) * 0.06));
	}

	return RGB(rgb[0], rgb[1], rgb[2]);
}

void GuideCtrl::PImpl::UpdateBuffer()
{
	if (mBuffer.IsNull()) {
		return;
	}
	CSize sizeBuff(mBuffer.GetWidth(), mBuffer.GetHeight());

	CDC* dc = CDC::FromHandle(mBuffer.GetDC());


	// 色設定を取得
	auto colorSettings = ColorSettings::Get();
	auto colorScheme = colorSettings->GetCurrentScheme();

	// 背景を描画(背景色で塗りつぶす)
	COLORREF crBr = colorScheme->GetBackgroundColor();
	CBrush br;
	br.CreateSolidBrush(crBr);
	CBrush* orgBr = dc->SelectObject(&br);
	dc->PatBlt(0, 0, sizeBuff.cx, sizeBuff.cy, PATCOPY);
	dc->SelectObject(orgBr);

	// 背景色を変える要素のブラシ
	CBrush brAlter;
	brAlter.CreateSolidBrush(GetAlterColor(crBr));

	// テキストカラー
	auto orgTxtColor = dc->SetTextColor(colorScheme->GetTextColor());

	auto orgFont = dc->SelectObject(mMainWnd->GetMainWindowFont());

	// フォントの高さを得る
	TEXTMETRIC tm;
	dc->GetTextMetrics(&tm);
	int fontH = tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;

	constexpr int MARGIN_RIGHT = 5;
	constexpr int MARGIN_ITEM = 10;

	CRect rcItem(CPoint(0, 0), sizeBuff);
	rcItem.top = (sizeBuff.cy - fontH) / 2;
	rcItem.bottom = rcItem.top + fontH;
	rcItem.right -= MARGIN_RIGHT;

	CString str;

	auto& keyMap = GetKeyModifierMap();

	auto orgMode = dc->SetBkMode(TRANSPARENT);
	for (auto it = mEntries.rbegin(); it != mEntries.rend(); ++it) {
		auto& item = *it;

		// 描画するテキスト生成
		auto itFind = keyMap.find(item.mModifier);
		if (itFind == keyMap.end()) {
			continue;
		}
		str.Format(_T("%s:%s"), (LPCTSTR)itFind->second, (LPCTSTR)item.mGuideStr);

		// テキストの幅を得る
		CSize size;
		GetTextExtentPoint32(dc->GetSafeHdc(), str, str.GetLength(), &size);
		rcItem.left = rcItem.right - size.cx;

		// ToDo: マッチするキー押下状態がない場合はデフォルト要素を強調

		// キー押下状態に応じて背景色を変える
		bool useAlterBGColor = itFind->first == mLastModifierFlag;
		bool isOnCursor = item.mModifier == mMouseModifier;

		// 枠の領域を求める
		CRect rcFrame(rcItem);
		rcFrame.InflateRect(MARGIN_ITEM/2,0,MARGIN_ITEM/2,0);

		// 枠描画用のペン・背景色ブラシを設定
		auto orgPen = dc->SelectObject(isOnCursor ? GetStockObject(BLACK_PEN) : GetStockObject(NULL_PEN));
		orgBr = dc->SelectObject(useAlterBGColor ? &brAlter : &br);

		// 枠を描画
		dc->RoundRect(&rcFrame, CPoint(MARGIN_ITEM/2, MARGIN_ITEM/2));

		// 元のペンとブラシに戻す
		dc->SelectObject(orgBr);
		dc->SelectObject(orgPen);

		// ガイドテキストを描画
		dc->DrawText(str, rcItem, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);
		it->mRect = rcItem;

		rcItem.right = rcItem.left - MARGIN_ITEM;
	}

	dc->SetTextColor(orgTxtColor);
	dc->SetBkMode(orgMode);
	dc->SelectObject(orgFont);
	mBuffer.ReleaseDC();
}

bool GuideCtrl::PImpl::UpdateCursorState(CPoint pos)
{
	uint32_t modifierFlag = 0xffffffff;
	for (auto& entry : mEntries) {
		if (entry.mRect.PtInRect(pos) == FALSE) {
			continue;
		}
		modifierFlag = entry.mModifier;
		break;
	}
	if (modifierFlag == mMouseModifier) {
		return false;
	}

	mMouseModifier = modifierFlag;
	return true;
}

bool GuideCtrl::PImpl::RecreateBuffer(CWnd* wnd, CSize size)
{
	// 新しいサイズに合わせてビットマップ作成
	if (mBuffer.IsNull() == false) {
		mBuffer.Destroy();
	}
	mBuffer.Create(size.cx, size.cy, 24);

	// デバイスコンテキスト作成
	CDC* dc = CDC::FromHandle(mBuffer.GetDC());

	// GDIオブジェクトを作成、選択
	auto orgBmp = dc->SelectObject((HBITMAP)mBuffer);

	// 色設定を取得
	auto colorSettings = ColorSettings::Get();
	auto colorScheme = colorSettings->GetCurrentScheme();

	CBrush br;
	br.CreateSolidBrush(colorScheme->GetBackgroundColor());
	CBrush* orgBr = dc->SelectObject(&br);

	// 背景を描画
	dc->PatBlt(0, 0, size.cx, size.cy, PATCOPY);

	// 後始末
	dc->SelectObject(orgBr);
	dc->SelectObject(orgBmp);

	mBuffer.ReleaseDC();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



GuideCtrl::GuideCtrl() : in(new PImpl)
{
}

GuideCtrl::~GuideCtrl()
{
}

BEGIN_MESSAGE_MAP(GuideCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


bool GuideCtrl::Initialize()
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = GuideCtrl::WindowProc;
	wc.hInstance     = GetModuleHandle(nullptr);
	wc.hIcon         = nullptr;
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"LauncherAppGuide";

	if (RegisterClassEx(&wc) == FALSE) {
		spdlog::error("Failed to regsiter guidectrl class.");
		return false;
	}
	return true;
}

void GuideCtrl::SetMainWindow(LauncherMainWindowIF* mainWnd)
{
	in->mMainWnd = mainWnd;
}

void GuideCtrl::SetClickNotifyMessageId(UINT msgId)
{
	in->mNotifyMessageId = msgId;
}

bool GuideCtrl::Draw(Command* cmd)
{
	// コマンドが持つアクションからキー別のアクション名のリストを作成する
	in->UpdateEntries(cmd);

	// キー毎のアクション名のリストでバッファを更新する
	in->UpdateBuffer();

	Invalidate();

	return false;
}

LRESULT GuideCtrl::WindowProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
	return ::DefWindowProc(h, msg, wp, lp);
}

void GuideCtrl::OnPaint()
{
	CRect rc;
	GetClientRect(&rc);

	if (in->mTimerId == 0) {
		// 初回呼び出し時にタイマーを作成する(キー押下状態を拾うためのタイマー)
		::SetTimer(GetSafeHwnd(), 1, 100, 0);
		in->mTimerId = 1;
	}
	if (in->mBuffer.IsNull()) {
		in->RecreateBuffer(this, rc.Size());
	}

	CPaintDC dc(this);
	dc.BitBlt(0, 0, rc.Width(), rc.Height(), CDC::FromHandle(in->mBuffer.GetDC()), 0, 0, SRCCOPY);
	in->mBuffer.ReleaseDC();
}

void GuideCtrl::OnTimer(UINT_PTR timerId)
{
	// キー押下状態を覚えておく(画面描画に反映する)
	uint32_t modifierFlag = 0;
	if (GetKeyState(VK_MENU) & 0x8000)  { modifierFlag |= Command::MODIFIER_ALT; }
	if (GetKeyState(VK_SHIFT) & 0x8000) { modifierFlag |= Command::MODIFIER_SHIFT; }
	if (GetKeyState(VK_CONTROL) & 0x8000)  { modifierFlag |= Command::MODIFIER_CTRL; }
	if (GetKeyState(VK_LWIN) & 0x8000)   { modifierFlag |= Command::MODIFIER_WIN; }

	bool isStateChanged = (modifierFlag != in->mLastModifierFlag);
	in->mLastModifierFlag = modifierFlag;

	// カーソル状態を更新する
	if (in->mMouseModifier != 0xffffffff) {
		CPoint pos;
		GetCursorPos(&pos);
		ScreenToClient(&pos);
		if (in->UpdateCursorState(pos)) {
			isStateChanged = true;
		}
	}

	if (isStateChanged) {
		in->UpdateBuffer();
		Invalidate();
	}
}

void GuideCtrl::OnSize(UINT type, int cx, int cy)
{
	if (cx == 0 || cy == 0) {
		return;
	}

	in->RecreateBuffer(this, CSize(cx, cy));
}

void GuideCtrl::OnLButtonDown(UINT flags, CPoint pt)
{
	__super::OnLButtonDown(flags, pt);

	uint32_t modifier = 0xffffffff;
	for (auto& entry : in->mEntries) {
		if (entry.mRect.PtInRect(pt) == FALSE) {
			continue;
		}
		modifier = entry.mModifier;
		break;
	}

	if (modifier != 0xffffffff && in->mNotifyMessageId != 0) {
		GetParent()->PostMessage(in->mNotifyMessageId, (WPARAM)modifier, 0);
	}
}

void GuideCtrl::OnMouseMove(UINT flags, CPoint pt)
{
	__super::OnMouseMove(flags, pt);

	if (in->UpdateCursorState(pt) == false) {
		return ;
	}

	// 再描画
	in->UpdateBuffer();
	Invalidate();
}

}}}



