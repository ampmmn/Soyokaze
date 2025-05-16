#pragma once

#include "commands/core/CommandQueryRequest.h"
#include <vector>

namespace launcherapp { namespace mainwindow {

class CommandLauncher
{
public:

public:
	virtual ~CommandLauncher() {}

	// $BFI$_9~$_(B
	virtual bool Load() = 0;
	// $B8!:w%j%/%(%9%H<B;\(B
	virtual void Query(const launcherapp::commands::core::CommandQueryRequest& req) = 0;
	// $B%3%^%s%I<B9T(B
	virtual bool Execute(const CString& str) = 0;
	// $B%U%!%$%k$,%I%m%C%W$5$l$?(B
	virtual void DropFiles(const std::vector<CString>& files) = 0;
	// URL$B$,%I%m%C%W$5$l$?(B
	virtual void DropURL(const CString& urlString) = 0;
	// $B%&%$%s%I%&(B
	virtual void CaptureWindow(HWND hwnd) = 0;

};



}} // end of namespace launcherapp::mainwindow

