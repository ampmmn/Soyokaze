#pragma once

#include "commands/core/CommandEditorIF.h"

// {E99D22AD-7332-4714-A622-20BF5B84878E}
constexpr launcherapp::core::IFID IFID_BUILTINCOMMANDEDITOR =
{ 0xe99d22ad, 0x7332, 0x4714, { 0xa6, 0x22, 0x20, 0xbf, 0x5b, 0x84, 0x87, 0x8e } };

namespace launcherapp {
namespace commands {
namespace builtin {

class BuiltinCommandEditor : virtual public launcherapp::core::CommandEditor
{
public:
	BuiltinCommandEditor(const CString& name, const CString& description, bool canEditEnable, bool canEditConfirm, CWnd* parentWnd);
	~BuiltinCommandEditor();

	CString GetName();
	void SetEnable(bool isEnable);
	bool GetEnable();
	void SetConfirm(bool isConfirm);
	bool GetConfirm();

// CommandEditor
	// コマンドは編集可能か?
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


} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

