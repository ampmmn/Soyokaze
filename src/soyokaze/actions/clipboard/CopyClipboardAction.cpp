#include "pch.h"
#include "CopyClipboardAction.h"
#include "commands/common/Clipboard.h"
#include "SharedHwnd.h"

namespace launcherapp { namespace actions { namespace clipboard {

using namespace launcherapp::commands::common;

CopyAction::CopyAction()
{
}

CopyAction::~CopyAction()
{
}

// Action
// アクションの内容を示す名称
CString CopyAction::GetDisplayName()
{
	return _T("クリップボードにコピー");
}

// アクションを実行する
bool CopyAction::Perform(Parameter* param, String* errMsg)
{
	// クリップボードにコピー
	bool isOK = Clipboard::Copy(param->GetWholeString());
	if (isOK == false && errMsg) {
		*errMsg = "クリップボードのコピーに失敗しました";
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CopyTextAction::CopyTextAction(const CString& text, bool enablePaste) :
 	mText(text), mEnablePaste(enablePaste)
{
}

CopyTextAction::~CopyTextAction()
{
}

void CopyTextAction::EnablePasteAfterCopy(bool isEnabled)
{
	mEnablePaste = isEnabled;
}

void CopyTextAction::SetDisplayName(const CString& name)
{
	mDisplayName = name;
}

// Action
// アクションの内容を示す名称
CString CopyTextAction::GetDisplayName()
{
	if (mDisplayName.IsEmpty()) {
		return mEnablePaste ? _T("コピーして貼り付け") : _T("クリップボードにコピー");
	}
	else {
		return mDisplayName;
	}
}

// アクションを実行する
bool CopyTextAction::Perform(Parameter* param, String* errMsg)
{
	UNREFERENCED_PARAMETER(param);

	// クリップボードにコピー
	bool isOK = Clipboard::Copy(mText);
	if (isOK == false && errMsg) {
		*errMsg = "クリップボードのコピーに失敗しました";
		return false;
	}

	//
	if (mEnablePaste) {

		// ランチャーアプリのウインドウが消えるのを待つ
		SharedHwnd h;
		while(IsWindowVisible(h.GetHwnd())) {
			Sleep(50);
		}

		// Shift-Insertキー押下による疑似的なペースト
		INPUT inputs[5] = {0};

		// Ctrlキー押下
		inputs[0].type = INPUT_KEYBOARD;
		inputs[0].ki.wVk = VK_CONTROL; // 仮想キーコード: Ctrl
		inputs[0].ki.dwFlags = KEYEVENTF_KEYUP; // 離上イベント

		// Shiftキー押下
		inputs[1].type = INPUT_KEYBOARD;
		inputs[1].ki.wVk = VK_SHIFT; // 仮想キーコード: Shift

		// Insertキー押下
		inputs[2].type = INPUT_KEYBOARD;
		inputs[2].ki.wVk = VK_INSERT; // 仮想キーコード: Insert

		// Insertキー離上
		inputs[3].type = INPUT_KEYBOARD;
		inputs[3].ki.wVk = VK_INSERT;
		inputs[3].ki.dwFlags = KEYEVENTF_KEYUP; // 離上イベント

		// Shiftキー離上
		inputs[4].type = INPUT_KEYBOARD;
		inputs[4].ki.wVk = VK_SHIFT;
		inputs[4].ki.dwFlags = KEYEVENTF_KEYUP; // 離上イベント

		// イベント送信
		SendInput(5, inputs, sizeof(INPUT));

	}

	return true;
}

}}}

