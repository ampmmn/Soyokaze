// あ
#pragma once

#include "core/UnknownIF.h"
#include "commands/core/CommandEntryIF.h"
#include "actions/core/ActionParameterIF.h"
#include "actions/core/Action.h"
#include "matcher/Pattern.h"
#include "hotkey/HotKeyAttribute.h"

class CommandHotKeyAttribute;

namespace launcherapp {
namespace core {


class Command : virtual public UnknownIF
{
protected:
	using Parameter = launcherapp::actions::core::Parameter;
	using Action = launcherapp::actions::core::Action;

public:
	// 修飾キー押下状態を示すビットフラグ
	enum {
		MODIFIER_SHIFT = 1,
		MODIFIER_CTRL = 2,
		MODIFIER_ALT = 4,
		MODIFIER_WIN = 8,
	};

public:
	// コマンド名を取得
	virtual CString GetName() = 0;
	// コマンドの説明を取得
	virtual CString GetDescription() = 0;
	// コマンド種類を表す表示名称
	virtual CString GetTypeDisplayName() = 0;
	// コマンドが実行可能かどうかを取得
	virtual bool CanExecute(String* reasonMsg) = 0;

	// 修飾キー押下状態に対応した実行アクションを取得する
	virtual bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) = 0;

	virtual HICON GetIcon() = 0;
	virtual int Match(Pattern* pattern) = 0;
	// 完全一致かつ候補が一つのときに自動実行を許すか
	virtual bool IsAllowAutoExecute() = 0;

	// ホットキー設定を取得する
	// 戻り値は ホットキーを設定する機能を有する場合はtrue、そうでなければfalse
	virtual bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) = 0;

	virtual Command* Clone() = 0;

	virtual bool Save(CommandEntryIF* entry) = 0;
	virtual bool Load(CommandEntryIF* entry) = 0;
};


} // end of namespace core
} // end of namespace launcherapp
