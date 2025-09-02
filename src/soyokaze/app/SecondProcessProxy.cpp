#include "pch.h"
#include "SecondProcessProxy.h"
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
bool SecondProcessProxy::SendCommandString(const String& commandStr, bool isPasteOnly)
{
	SPDLOG_DEBUG("args commandStr:{}", commandStr.c_str());

	std::vector<uint8_t> stm(sizeof(SEND_COMMAND_PARAM) + sizeof(char) * commandStr.size());
	auto p = (SEND_COMMAND_PARAM*)stm.data();
	p->mIsPasteOnly = isPasteOnly;
	memcpy(p->mText, commandStr.data(), sizeof(char) * commandStr.size());

	auto queue = InterProcessMessageQueue::GetInstance();
	return queue->Enqueue(EVENT_ID::SEND_COMMAND, stm.data(), stm.size());
}

bool SecondProcessProxy::SendCaretRange(int startPos, int length)
{
	SPDLOG_DEBUG("args startPos:{0} length:{1}", startPos, length);

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
bool SecondProcessProxy::RegisterPath(const String& pathStr)
{
	SPDLOG_DEBUG("args path:{}", pathStr.c_str());

	String name = PathFindFileNameA(pathStr.c_str());

	static const std::regex ext_pattern(R"(^(.*?)(\.[^.]*$))");
	auto ret = std::regex_replace(name, ext_pattern, "$1");

	// 空白を置換
	name.Replace(' ', '_');
	return SendCommandString(fmt::format("new \"{0}\" \"{1}\"", ret, pathStr), false);
}

bool SecondProcessProxy::ChangeDirectory(const String& pathStr)
{
	SPDLOG_DEBUG("args startPos:{}", pathStr);

	std::vector<uint8_t> stm(sizeof(CHANGE_DIRECTORY_PARAM) + sizeof(char) * pathStr.GetLength());
	auto p = (CHANGE_DIRECTORY_PARAM*)stm.data();
	memcpy(p->mDirPath, pathStr.data(), sizeof(char) * pathStr.GetLength());

	auto queue = InterProcessMessageQueue::GetInstance();
	return queue->Enqueue(EVENT_ID::CHANGE_DIRECTORY, stm.data(), stm.size());
}

bool SecondProcessProxy::Hide()
{
	auto queue = InterProcessMessageQueue::GetInstance();
	return queue->Enqueue(EVENT_ID::HIDE, "", 0);
}

/**
 * @return true: アクティブ化した  false: 先行プロセスはない
 */
bool SecondProcessProxy::Show()
{
	SPDLOG_DEBUG("start");

	// 先行プロセスを有効化する
	auto queue = InterProcessMessageQueue::GetInstance();
	return queue->Enqueue(EVENT_ID::ACTIVATE_WINDOW, "", 0);
}



}


