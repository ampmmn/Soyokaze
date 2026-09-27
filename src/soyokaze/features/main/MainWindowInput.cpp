#include "pch.h"
#include "MainWindowInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace mainwindow {

struct MainWindowInput::PImpl
{
	CString mCommandStr;
	
};

MainWindowInput::MainWindowInput() : in(new PImpl)
{
}

MainWindowInput::~MainWindowInput()
{
}

bool MainWindowInput::HasKeyword()
{
	if (in->mCommandStr.IsEmpty() || in->mCommandStr == _T("\r\n")) {
		return false;
	}
	return true;
}

int MainWindowInput::GetLength()
{
	return in->mCommandStr.GetLength();
}

void MainWindowInput::Clear()
{
	in->mCommandStr.Empty();
}

void MainWindowInput::RemoveLastWord()
{
	CString str = in->mCommandStr;

	// 右端の空白をカット
	str.TrimRight();

	// 次に空白が現れる位置を末尾からさがす
	int n = 0;

	int len = str.GetLength();
	for (int i = len-1; i >= 0; --i) {

		TCHAR c = str[i];

		bool isSpace = c == _T(' ');
		bool isPathSeparator = i != len-1 && (c == _T('\\') || c == _T('/'));
		if (isSpace == false && isPathSeparator == false) {
			continue;
		}

		n = i;
		break;
	}

	// 先端まで到達した場合は全クリア
	if (n == 0) {
		in->mCommandStr.Empty();
		return;
	}
	// 見つかった位置でカットする
	in->mCommandStr = in->mCommandStr.Mid(0, n+1);
}

void MainWindowInput::SetKeyword(const CString& wholeText, bool withSpace)
{
	spdlog::debug("MainWindowInput::SetKeyword start");

	in->mCommandStr = wholeText;
	if (withSpace) {
		in->mCommandStr += _T(" ");
	}
}

// テキストを得る
const CString& MainWindowInput::GetKeyword()
{
	return in->mCommandStr;
}

// キーワード文字列に(空白区切りで)引数を追加
void MainWindowInput::AddArgument(const CString& arg)
{
	spdlog::debug("MainWindowInput::AddArgument start");

	int len = in->mCommandStr.GetLength();
	if (len > 0 && in->mCommandStr[len-1] != _T(' ')) {
		in->mCommandStr += _T(" ");
	}
	in->mCommandStr += arg;
}

// Ctrl-Backspaceを押下時に入力される不可視文字(0x7E→Backspace)を消す
bool MainWindowInput::ReplaceInvisibleChars()
{
	spdlog::debug("MainWindowInput::ReplaceInvisibleChars start");

	if (in->mCommandStr.Find((TCHAR)0x7F) == -1) {
		return false;
	}

	TCHAR bsStr[] = { (TCHAR)0x7F, (TCHAR)0x00 };
	in->mCommandStr.Replace(bsStr, _T(""));
	return true;
}

CString& MainWindowInput::CommandStr()
{
	return in->mCommandStr;
}

}
}

