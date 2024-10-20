#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/websearch/WebSearchCommandParam.h"

// {1DE85D1C-CDDF-4A07-92E4-FE899A18DDCA}
constexpr launcherapp::core::IFID IFID_WEBSEARCHCOMMANDEDITOR =
{ 0x1de85d1c, 0xcddf, 0x4a07, { 0x92, 0xe4, 0xfe, 0x89, 0x9a, 0x18, 0xdd, 0xca } };

namespace launcherapp {
namespace commands {
namespace websearch {

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


} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

