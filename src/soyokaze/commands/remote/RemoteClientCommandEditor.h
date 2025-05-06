#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/remote/RemoteClientCommandParam.h"

// {B098E5EF-7DC5-4555-8DCA-995C6E15E699}
constexpr launcherapp::core::IFID IFID_REMOTECLIENTCOMMANDEDITOR =
{ 0xb098e5ef, 0x7dc5, 0x4555, { 0x8d, 0xca, 0x99, 0x5c, 0x6e, 0x15, 0xe6, 0x99 } };

namespace launcherapp { namespace commands { namespace remote {

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

}}} // end of namespace launcherapp::commands::remote
