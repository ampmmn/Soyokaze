#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace datetime {

// 時間差の単位種別
enum {
	TYPE_HOUR,
	TYPE_MINUTE,
	TYPE_SECOND,
};

/**
  時間差(HH:MM-HH:MM)の計算結果を候補として表示するコマンド
	@note 単位ごと(時間/分/秒)に1つのインスタンスを生成する
*/
class DateTimeCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:

public:
	/**
	  コンストラクタ
	  @param[in] ts       時間差
	  @param[in] unitType 表示単位(TYPE_HOUR/TYPE_MINUTE/TYPE_SECOND)
	*/
	DateTimeCommand(CTimeSpan ts, int unitType);
	virtual ~DateTimeCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(DateTimeCommand)

public:
	// 種別の表示名(時間)
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace datetime
} // end of namespace commands
} // end of namespace launcherapp
