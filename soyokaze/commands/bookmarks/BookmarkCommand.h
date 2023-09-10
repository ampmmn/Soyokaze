#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace bookmarks {

class BookmarkCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	BookmarkCommand(const CString& type, const CString& name, const CString& url);
	virtual ~BookmarkCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze

