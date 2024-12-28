#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/align_window/AlignWindowCommandParam.h"


// {9A7479ED-F96F-4ED6-8670-89829B892D23}
constexpr launcherapp::core::IFID IFID_ALIGNWINDOWCOMMANDEDITOR =
{ 0x9a7479ed, 0xf96f, 0x4ed6, { 0x86, 0x70, 0x89, 0x82, 0x9b, 0x89, 0x2d, 0x23 } };

namespace launcherapp {
namespace commands {
namespace align_window {

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


} // end of namespace align_window
} // end of namespace commands
} // end of namespace launcherapp

