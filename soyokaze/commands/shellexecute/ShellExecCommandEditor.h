#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/shellexecute/ShellExecCommandParam.h"

// {FA047B49-DF4A-41EB-8A3C-3F102E951F62}
constexpr launcherapp::core::IFID IFID_SHELLEXECCOMMANDEDITOR =
{ 0xfa047b49, 0xdf4a, 0x41eb, { 0x8a, 0x3c, 0x3f, 0x10, 0x2e, 0x95, 0x1f, 0x62 } };

namespace launcherapp {
namespace commands {
namespace shellexecute {

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


} // end of namespace shellexecute
} // end of namespace commands
} // end of namespace launcherapp

