#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/py_extension/PyExtensionCommandParam.h"

// {CB298947-AF26-4402-A4A8-A4B64C56C043}
constexpr launcherapp::core::IFID IFID_PYEXTENSIONCOMMANDEDITOR =
{ 0xcb298947, 0xaf26, 0x4402, { 0xa4, 0xa8, 0xa4, 0xb6, 0x4c, 0x56, 0xc0, 0x43 } };

namespace launcherapp { namespace commands { namespace py_extension {

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


}}} // end of namespace launcherapp::commands::py_extension

