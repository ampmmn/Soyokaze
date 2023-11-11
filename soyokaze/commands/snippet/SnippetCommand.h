#pragma once

#include "core/CommandIF.h"
#include <memory>

class HOTKEY_ATTR;

namespace soyokaze {
namespace commands {
namespace snippet {

class SnippetCommand : public soyokaze::core::Command
{
public:
	SnippetCommand();
	virtual ~SnippetCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	int EditDialog(const Parameter* param) override;
	soyokaze::core::Command* Clone() override;

	bool Save(CommandFile* cmdFile) override;

	bool Load(CommandFile* cmdFile, void* entry_);

	uint32_t AddRef() override;
	uint32_t Release() override;

	static CString GetType();

	static bool NewDialog(const Parameter* param);
	
public:
	SnippetCommand& SetName(LPCTSTR name);
	SnippetCommand& SetDescription(LPCTSTR description);
	SnippetCommand& SetText(const CString& text);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

