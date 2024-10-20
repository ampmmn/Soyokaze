#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/simple_dict/SimpleDictParam.h"

// {CE91BB08-87AC-4153-A758-529E990FAFC7}
constexpr launcherapp::core::IFID IFID_SIMPLEDICTCOMMANDEDITOR =
{ 0xce91bb08, 0x87ac, 0x4153, { 0xa7, 0x58, 0x52, 0x9e, 0x99, 0xf, 0xaf, 0xc7 } };

namespace launcherapp {
namespace commands {
namespace simple_dict {

class CommandEditor : virtual public launcherapp::core::CommandEditor
{
	using CommandParam = SimpleDictParam;
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


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

