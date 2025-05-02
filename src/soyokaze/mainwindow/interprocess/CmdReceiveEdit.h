#pragma once

namespace launcherapp {
namespace mainwindow {
namespace interprocess {

struct SEND_COMMAND_PARAM;
struct SET_CARETRANGE_PARAM;
struct CHANGE_DIRECTORY_PARAM;

}
}
}

class CmdReceiveEdit : public CEdit
{
public:
	CmdReceiveEdit();	// 標準コンストラクター
	virtual ~CmdReceiveEdit();

	void Init();

protected:
	BOOL OnSendCommand(launcherapp::mainwindow::interprocess::SEND_COMMAND_PARAM*);
	BOOL OnSetCaretRange(launcherapp::mainwindow::interprocess::SET_CARETRANGE_PARAM*);
	BOOL OnChangeDirectory(launcherapp::mainwindow::interprocess::CHANGE_DIRECTORY_PARAM*);
// 実装
protected:
	afx_msg void OnTimer(UINT_PTR timerId);
	DECLARE_MESSAGE_MAP()
};
