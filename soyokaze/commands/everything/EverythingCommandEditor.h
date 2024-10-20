#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/everything/EverythingCommandParam.h"

// {BA585ACD-2485-4F28-984E-AF39894D90B5}
constexpr launcherapp::core::IFID IFID_EVERYTHINGCOMMANDEDITOR =
{ 0xba585acd, 0x2485, 0x4f28, { 0x98, 0x4e, 0xaf, 0x39, 0x89, 0x4d, 0x90, 0xb5 } };

namespace launcherapp {
namespace commands {
namespace everything {

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


} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

