#pragma once

#include "features/main/preview/PreviewWindow.h"
#include <memory>

namespace launcherapp { namespace commands { namespace clipboardhistory {

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
	void SetPreviewText(const CString& text, uint64_t date);
	void Destroy();

	void SetEnable(bool isEnable);
	void Disable();
	bool IsEnable() const;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}} // end of namespace launcherapp::commands::clipboardhistory

