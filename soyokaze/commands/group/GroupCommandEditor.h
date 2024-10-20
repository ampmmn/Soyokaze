#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/group/CommandParam.h"

// {A4CCDBE5-4D49-40CA-9BA7-927FE7D19923}
constexpr launcherapp::core::IFID IFID_GROUPCOMMANDEDITOR =
{ 0xa4ccdbe5, 0x4d49, 0x40ca, { 0x9b, 0xa7, 0x92, 0x7f, 0xe7, 0xd1, 0x99, 0x23 } };

namespace launcherapp {
namespace commands {
namespace group {

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


} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

