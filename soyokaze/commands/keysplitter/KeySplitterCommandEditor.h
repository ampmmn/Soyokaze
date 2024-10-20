#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/keysplitter/KeySplitterParam.h"

// {4F380107-18F0-4F72-A85E-3545BB326AE9}
constexpr launcherapp::core::IFID IFID_KEYSPLITTERCOMMANDEDITOR =
{ 0x4f380107, 0x18f0, 0x4f72, { 0xa8, 0x5e, 0x35, 0x45, 0xbb, 0x32, 0x6a, 0xe9 } };

namespace launcherapp {
namespace commands {
namespace keysplitter {

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

