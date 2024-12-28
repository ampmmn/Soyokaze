#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"

// {FB178447-C374-4E02-B0CF-034C5933C01A}
constexpr launcherapp::core::IFID IFID_URLDIRECTORYINDEXCOMMANDEDITOR =
{ 0xfb178447, 0xc374, 0x4e02, { 0xb0, 0xcf, 0x3, 0x4c, 0x59, 0x33, 0xc0, 0x1a } };

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

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


} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp

