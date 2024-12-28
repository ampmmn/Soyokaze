#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/filter/FilterCommandParam.h"

// {2EE1A0BE-68AC-41D5-B62B-1E3015B04F6F}
constexpr launcherapp::core::IFID IFID_FILTERCOMMANDEDITOR =
{ 0x2ee1a0be, 0x68ac, 0x41d5, { 0xb6, 0x2b, 0x1e, 0x30, 0x15, 0xb0, 0x4f, 0x6f } };

namespace launcherapp {
namespace commands {
namespace filter {

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


} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

