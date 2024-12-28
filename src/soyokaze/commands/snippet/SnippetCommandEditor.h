#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/snippet/SnippetCommandParam.h"

// {19961E58-56DD-425B-BC41-5498AD688523}
constexpr launcherapp::core::IFID IFID_SNIPPETCOMMANDEDITOR =
{ 0x19961e58, 0x56dd, 0x425b, { 0xbc, 0x41, 0x54, 0x98, 0xad, 0x68, 0x85, 0x23 } };

namespace launcherapp {
namespace commands {
namespace snippet {

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


} // end of namespace snippet
} // end of namespace commands
} // end of namespace launcherapp

