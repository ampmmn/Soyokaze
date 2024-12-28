#pragma once

#include "commands/core/UnknownIF.h"
#include "commands/core/CommandEditorIF.h"

namespace launcherapp {
namespace core {

class Editable : virtual public UnknownIF
{
public:
	// コマンドは編集可能か?
	virtual bool IsEditable() = 0;
	// コマンドは削除可能か?
	virtual bool IsDeletable() = 0;
	// コマンドを編集するためのダイアログを作成/取得する
	virtual bool CreateEditor(HWND parent, CommandEditor** editor) = 0;
	// ダイアログ上での編集結果をコマンドに適用する
	virtual bool Apply(CommandEditor* editor) = 0;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	virtual bool CreateNewInstanceFrom(CommandEditor* editor, Command** newCmd) = 0;

};

}  // end of namespace core
}  // end of namespace launcherapp
