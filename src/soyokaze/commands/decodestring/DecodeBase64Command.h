#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include "commands/core/PreviewProvider.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace decodestring {

class DecodeBase64Command :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::core::PreviewProvider
{
public:

public:
	DecodeBase64Command();
	virtual ~DecodeBase64Command();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;
	/**
	  デコード結果を表示するプレビューウインドウを取得する
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

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(DecodeBase64Command)

public:
	static CString TypeDisplayName();
};


} // end of namespace decodestring
} // end of namespace commands
} // end of namespace launcherapp

