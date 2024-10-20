#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/regexp/RegExpCommandParam.h"

// {A6BACFA3-C188-4CB9-BD54-7577A967A773}
constexpr launcherapp::core::IFID IFID_REGEXPCOMMANDEDITOR =
{ 0xa6bacfa3, 0xc188, 0x4cb9, { 0xbd, 0x54, 0x75, 0x77, 0xa9, 0x67, 0xa7, 0x73 } };

namespace launcherapp {
namespace commands {
namespace regexp {

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


} // end of namespace regexp
} // end of namespace commands
} // end of namespace launcherapp

