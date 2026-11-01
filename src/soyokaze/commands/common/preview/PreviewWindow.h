#pragma once

#include "features/main/preview/PreviewWindow.h"
#include <memory>
#include <vector>

namespace launcherapp { namespace commands { namespace common { namespace preview {

class PreviewWindow : public launcherapp::mainwindow::preview::PreviewWindow
{
private:
	PreviewWindow();
	~PreviewWindow();

public:
	static PreviewWindow* Get();

public:
	void Enter() override;
	void Leave() override;
	HWND GetHwnd() override;
	/**
	  情報欄の内容と本文をプレビューへ設定する
	  @param[in] text プレビューする本文
	  @param[in] infoText 情報欄に表示する文字列
	  @param[in] keywords 強調表示するキーワード
	*/
	void SetPreviewText(const CString& text, const CString& infoText, const std::vector<CString>& keywords);
	void Destroy();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}} // end of namespace launcherapp::commands::common::preview

