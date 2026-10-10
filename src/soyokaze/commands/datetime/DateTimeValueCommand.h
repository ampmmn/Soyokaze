#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace datetime {

/**
  日時の文字列(現在日時、日付、時刻、曜日、日数オフセットの結果)を候補として表示するコマンド
	@note 候補として表示する文字列をそのままクリップボードへコピーする
*/
class DateTimeValueCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:

public:
	/**
	  コンストラクタ
	  @param[in] text     表示・コピーする文字列
	  @param[in] typeName 種別の表示名(日時/日付/時刻/曜日など)
	*/
	DateTimeValueCommand(const CString& text, const CString& typeName);
	virtual ~DateTimeValueCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(DateTimeValueCommand)

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace datetime
} // end of namespace commands
} // end of namespace launcherapp
