#pragma once

#include <memory>

namespace launcherapp { namespace commands { namespace clipboardhistory {

class PreviewWindow
{
private:
	PreviewWindow();
	~PreviewWindow();

public:
	static PreviewWindow* Get();

public:
	bool Show();
	bool Hide();
	void SetPreviewText(const CString& text, uint64_t date);
	void Destroy();

	void SetEnable(bool isEnable);
	void Disable();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}} // end of namespace launcherapp::commands::clipboardhistory

