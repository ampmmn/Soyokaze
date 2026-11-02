#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include "commands/core/PreviewProvider.h"
#include <memory>
#include <string>

namespace launcherapp {
namespace commands {
namespace decodestring {

class EscapedCharCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::core::PreviewProvider
{
public:

public:
	EscapedCharCommand();
	virtual ~EscapedCharCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;
	/**
	  エスケープ解除結果を表示するプレビューウインドウを取得する
	  @return プレビューウインドウ
	*/
	launcherapp::mainwindow::preview::PreviewWindow* FetchPreviewWindow() override;
	/**
	  プレビュー提供インターフェースを取得する
	  @return true:取得成功  false:未対応のインターフェース
	  @param[in] ifid インターフェース識別子
	  @param[out] cmd 取得したインターフェース
	*/
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	static bool ScanAsU4(std::string::iterator& it, std::string::iterator itEnd, std::string& dst);
	static bool ScanAsU8(std::string::iterator& it, std::string::iterator itEnd, std::string& dst);
	static bool ScanAsHex(std::string::iterator& it, std::string::iterator itEnd, std::string& dst);
	static bool ScanAsOctal(std::string::iterator& it, std::string::iterator itEnd, std::string& dst);

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(EscapedCharCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace decodestring
} // end of namespace commands
} // end of namespace launcherapp

