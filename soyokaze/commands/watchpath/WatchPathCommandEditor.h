#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/watchpath/WatchPathCommandParam.h"

// {9794471A-B8D8-470C-AABE-FFFF1B62C3E7}
constexpr launcherapp::core::IFID IFID_WATCHPATHCOMMANDEDITOR =
{ 0x9794471a, 0xb8d8, 0x470c, { 0xaa, 0xbe, 0xff, 0xff, 0x1b, 0x62, 0xc3, 0xe7 } };

namespace launcherapp {
namespace commands {
namespace watchpath {

class CommandEditor : virtual public launcherapp::core::CommandEditor
{
public:
	CommandEditor(CWnd* parentWnd = nullptr);
	~CommandEditor();

	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

	CString GetOriginalName();

// CommandEditor
	// 名前を上書きする
	void OverrideName(LPCTSTR name) override;
	// 元のコマンド名を設定する(そのコマンド名と同じ場合は「コマンド名重複」とみなさない)
	void SetOriginalName(LPCTSTR name) override;
	// コマンドを編集するためのダイアログを作成/取得する
	bool DoModal() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** obj) override;
	uint32_t AddRef() override;
	uint32_t Release() override;
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace watchpath
} // end of namespace commands
} // end of namespace launcherapp

