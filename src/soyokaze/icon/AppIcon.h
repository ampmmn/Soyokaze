#pragma once

#include <memory>

namespace launcherapp {
namespace icon {


class AppIcon
{
	AppIcon();
	~AppIcon();

public:
	static AppIcon* Get();

	HICON DefaultIconHandle();
	HICON IconHandle();
	bool Import(const CString& iconFilePath);
	void Reset();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};


}
}

