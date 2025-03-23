#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/alias/AliasCommandParam.h"

// {21D50C39-0D6B-43E9-9E6D-13375E4EC580}
constexpr launcherapp::core::IFID IFID_ALIASCOMMANDEDITOR =
{ 0x21d50c39, 0xd6b, 0x43e9, { 0x9e, 0x6d, 0x13, 0x37, 0x5e, 0x4e, 0xc5, 0x80 } };

namespace launcherapp {
namespace commands {
namespace alias {

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


} // end of namespace alias
} // end of namespace commands
} // end of namespace launcherapp

