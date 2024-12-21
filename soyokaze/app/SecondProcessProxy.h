#pragma once

#include "app/SecondProcessProxyIF.h"

namespace launcherapp {

class SecondProcessProxy : public SecondProcessProxyIF
{
public:
	SecondProcessProxy();
	~SecondProcessProxy();

	bool SendCommandString(const CString& commandStr, bool isPasteOnly) override;
	bool SendCaretRange(int startPos, int length) override;
	bool RegisterPath(const CString& pathStr) override;
	bool ChangeDirectory(const CString& pathStr) override;
	bool Hide() override;
	bool Show() override;
};

}

