#pragma once

#include "commands/core/CommandIF.h"
#include "commands/core/EditableIF.h"

#include "utility/RefPtr.h"

#pragma warning( disable : 4250)

namespace launcherapp {
namespace commands {
namespace common {

class UserCommandBase :
 	virtual public launcherapp::core::Command,
 	virtual public launcherapp::core::Editable

{
public:
	UserCommandBase();
	virtual ~UserCommandBase();

// Command
	CString GetErrorString() override;

// Editable
	// コマンドは編集可能か?
	bool IsEditable() override;
	// コマンドは削除可能か?
	bool IsDeletable() override;
	// コマンドを編集するためのダイアログを作成/取得する
	bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// ダイアログ上での編集結果をコマンドに適用する
	bool Apply(launcherapp::core::CommandEditor* editor) override;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	bool CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmd) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

protected:
	static bool GetNamedParamString(Parameter* param, LPCTSTR name, CString& value);

protected:
	uint32_t mRefCount = 1;
};

}
}
}


