#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/onenote/OneNoteBook.h"
#include "commands/onenote/OneNoteSection.h"
#include "commands/onenote/OneNotePage.h"
#include <memory>

namespace launcherapp { namespace commands { namespace onenote {

class CommandParam;

class OneNoteCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase
{
public:
	OneNoteCommand();
	OneNoteCommand(CommandParam* param, const OneNoteBook& book, const OneNoteSection& section, const OneNotePage& page);
	~OneNoteCommand() override;

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(OneNoteCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::onenote

