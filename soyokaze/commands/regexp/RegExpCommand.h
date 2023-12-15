#pragma once

#include "core/CommandIF.h"
#include <memory>

class HOTKEY_ATTR;

namespace soyokaze {
namespace commands {
namespace regexp {

class RegExpCommand : public soyokaze::core::Command
{
public:
	struct ATTRIBUTE {
		ATTRIBUTE();

		CString mPath;
		CString mParam;
		CString mDir;
		int mShowType;
	};


public:
	RegExpCommand();
	virtual ~RegExpCommand();

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
	bool IsPriorityRankEnabled() override;
	soyokaze::core::Command* Clone() override;

	bool Save(CommandFile* cmdFile) override;

	bool Load(CommandFile* cmdFile, void* entry_);

	uint32_t AddRef() override;
	uint32_t Release() override;

	static CString GetType();

	static bool NewDialog(const Parameter* param);

	// 管理者権限で実行しているか
	static bool IsRunAsAdmin();
	
public:
	RegExpCommand& SetName(LPCTSTR name);
	RegExpCommand& SetDescription(LPCTSTR description);
	RegExpCommand& SetAttribute(const ATTRIBUTE& attr);
	RegExpCommand& SetAttributeForParam0(const ATTRIBUTE& attr);
	RegExpCommand& SetPath(LPCTSTR path);
	RegExpCommand& SetRunAs(int runAs);

	RegExpCommand& SetMatchPattern(LPCTSTR pattern);

	void GetAttribute(ATTRIBUTE& attr);
	int GetRunAs();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

