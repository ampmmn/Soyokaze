#include "pch.h"
#include "SecondProcessProxy.h"
#include "SharedHwnd.h"
#include "resource.h"
#include "mainwindow/LauncherMainWindow.h"
#include "mainwindow/interprocess/InterProcessMessageQueue.h"

using namespace launcherapp::mainwindow::interprocess;

namespace launcherapp {

SecondProcessProxy::SecondProcessProxy()
{
}

SecondProcessProxy::~SecondProcessProxy()
{
}

/**
 *  先行プロセスに対しコマンド文字列を送る
 *  (先行プロセス側でコマンドを実行する)
 */
bool SecondProcessProxy::SendCommandString(const CString& commandStr, bool isPasteOnly)
{
	SPDLOG_DEBUG(_T("args commandStr:{0}"), (LPCTSTR)commandStr);

	std::vector<uint8_t> stm(sizeof(SEND_COMMAND_PARAM) + sizeof(TCHAR) * commandStr.GetLength());
	auto p = (SEND_COMMAND_PARAM*)stm.data();
	p->mIsPasteOnly = isPasteOnly;
	memcpy(p->mText, (LPCTSTR)commandStr, sizeof(TCHAR) * commandStr.GetLength());

	auto queue = InterProcessMessageQueue::GetInstance();
	return queue->Enqueue(EVENT_ID::SEND_COMMAND, stm.data(), stm.size());
}

bool SecondProcessProxy::SendCaretRange(int startPos, int length)
{
	SPDLOG_DEBUG(_T("args startPos:{0} length:{1}"), startPos, length);

	std::vector<uint8_t> stm(sizeof(SET_CARETRANGE_PARAM));
	auto p = (SET_CARETRANGE_PARAM*)stm.data();
	p->mStartPos = startPos;
	p->mLength = length;

	auto queue = InterProcessMessageQueue::GetInstance();
	return queue->Enqueue(EVENT_ID::SET_CARETRANGE, stm.data(), stm.size());
}

/**
 *  指定されたパスをコマンドとして登録する
 *  @return true: 成功 false:失敗
 *  @param pathStr  登録対象のファイルパス
 */
bool SecondProcessProxy::RegisterPath(const CString& pathStr)
{
	SPDLOG_DEBUG(_T("args path:{0}"), (LPCTSTR)pathStr);

	CString name = PathFindFileName(pathStr);
	PathRemoveExtension(name.GetBuffer(name.GetLength()));
	name.ReleaseBuffer();

	// 空白を置換
	name.Replace(_T(' '), _T('_'));

	CString commandStr(_T("new "));
	commandStr += _T("\"") + name + _T("\" \"") + pathStr + _T("\"");
	return SendCommandString(commandStr, false);
}

bool SecondProcessProxy::ChangeDirectory(const CString& pathStr)
{
	SPDLOG_DEBUG(_T("args startPos:{}"), (LPCTSTR)pathStr);

	std::vector<uint8_t> stm(sizeof(CHANGE_DIRECTORY_PARAM) + sizeof(TCHAR) * pathStr.GetLength());
	auto p = (CHANGE_DIRECTORY_PARAM*)stm.data();
	memcpy(p->mDirPath, (LPCTSTR)pathStr, sizeof(TCHAR) * pathStr.GetLength());

	auto queue = InterProcessMessageQueue::GetInstance();
	return queue->Enqueue(EVENT_ID::CHANGE_DIRECTORY, stm.data(), stm.size());
}

bool SecondProcessProxy::Hide()
{
	auto queue = InterProcessMessageQueue::GetInstance();
	return queue->Enqueue(EVENT_ID::HIDE, _T(""), 0);
}

/**
 * @return true: アクティブ化した  false: 先行プロセスはない
 */
bool SecondProcessProxy::Show()
{
	SPDLOG_DEBUG(_T("start"));

	// 先行プロセスを有効化する
	auto queue = InterProcessMessageQueue::GetInstance();
	return queue->Enqueue(EVENT_ID::ACTIVATE_WINDOW, _T(""), 0);
}



}


