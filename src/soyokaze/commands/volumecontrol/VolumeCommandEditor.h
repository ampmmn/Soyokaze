#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/volumecontrol/VolumeCommandParam.h"

// {88E6B194-B135-490D-A0A6-A7FC96B4E007}
constexpr launcherapp::core::IFID IFID_VOLUMECOMMANDEDITOR =
{ 0x88e6b194, 0xb135, 0x490d, { 0xa0, 0xa6, 0xa7, 0xfc, 0x96, 0xb4, 0xe0, 0x7 } };

namespace launcherapp {
namespace commands {
namespace volumecontrol {

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


} // end of namespace volumecontrol
} // end of namespace commands
} // end of namespace launcherapp

