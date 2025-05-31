#pragma once

#include "commands/specialfolderfiles/SpecialFolderFile.h"

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {

class SpecialFolderFileFind
{
public:
	SpecialFolderFileFind();
	~SpecialFolderFileFind();

public:
	bool FindShortcutFiles(std::vector<ITEM>& items);

	void EnableStartMenu(bool isEnable);
	void EnableRecent(bool isEnable);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;


};


} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

