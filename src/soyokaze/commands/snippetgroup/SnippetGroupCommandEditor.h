#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/snippetgroup/SnippetGroupParam.h"

// {7074C7C2-A627-4F3C-8B63-1B50E5BAD125}
constexpr launcherapp::core::IFID IFID_SNIPPETGROUPCOMMANDEDITOR =
{ 0x7074c7c2, 0xa627, 0x4f3c, { 0x8b, 0x63, 0x1b, 0x50, 0xe5, 0xba, 0xd1, 0x25 } };

namespace launcherapp {
namespace commands {
namespace snippetgroup {

class CommandEditor : virtual public launcherapp::core::CommandEditor
{
	using CommandParam = SnippetGroupParam;
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


} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

