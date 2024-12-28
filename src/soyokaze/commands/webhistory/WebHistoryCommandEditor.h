#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/webhistory/WebHistoryCommandParam.h"

// {03765FB4-8798-4746-8F27-A5CBC910445C}
constexpr launcherapp::core::IFID IFID_WEBHISTORYCOMMANDEDITOR =
{ 0x3765fb4, 0x8798, 0x4746, { 0x8f, 0x27, 0xa5, 0xcb, 0xc9, 0x10, 0x44, 0x5c } };

namespace launcherapp {
namespace commands {
namespace webhistory {

class CommandEditor : virtual public launcherapp::core::CommandEditor
{
public:
	CommandEditor(CWnd* parentWnd = nullptr);
	~CommandEditor();

	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

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


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

