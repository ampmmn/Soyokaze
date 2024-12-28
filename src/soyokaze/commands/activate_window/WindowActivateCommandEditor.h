#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/activate_window/WindowActivateCommandParam.h"


// {F70A5DC1-96DC-4FA8-B647-51AEB09504C2}
constexpr launcherapp::core::IFID IFID_WINDOWACTIVATECOMMANDEDITOR =
{ 0xf70a5dc1, 0x96dc, 0x4fa8, { 0xb6, 0x47, 0x51, 0xae, 0xb0, 0x95, 0x4, 0xc2 } };

namespace launcherapp {
namespace commands {
namespace activate_window {

class WindowActivateCommandEditor : virtual public launcherapp::core::CommandEditor
{
public:
	WindowActivateCommandEditor(CWnd* parentWnd = nullptr);
	~WindowActivateCommandEditor();

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


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

