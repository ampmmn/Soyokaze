#pragma once

#pragma once

#include "commands/onenote/OneNoteBook.h"
#include "commands/onenote/OneNotePage.h"
#include <memory>
#include <vector>

namespace launcherapp { namespace commands { namespace onenote {

class OneNoteAppProxy
{
public:
	OneNoteAppProxy();
	~OneNoteAppProxy();

	bool GetHierarchy(std::vector<OneNoteBook>& books);
	bool NavigateTo(LPCWSTR id);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::onenote

