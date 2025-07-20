#pragma once

#include "commands/core/CommandIF.h"
#include "commands/core/IFIDDefine.h"
#include "utility/RefPtr.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace common {


class AdhocCommandBase : virtual public launcherapp::core::Command
{
public:
	AdhocCommandBase(LPCTSTR name = _T(""), LPCTSTR description = _T(""));
	virtual ~AdhocCommandBase();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	//CString GetTypeDisplayName() override;
	bool CanExecute() override;
	BOOL Execute(Parameter* param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsAllowAutoExecute() override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;
protected:
	CString mName;
	CString mDescription;
	CString mErrMsg;
};


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

#define DECLARE_ADHOCCOMMAND_UNKNOWNIF(clsName) \
	public: \
		uint32_t AddRef() override; \
		uint32_t Release() override; \
	protected: \
		uint32_t mRefCount = 1;

#define IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(clsName) \
	uint32_t clsName::AddRef() { return (uint32_t)InterlockedIncrement(&mRefCount); } \
	uint32_t clsName::Release() { \
		auto n = InterlockedDecrement(&mRefCount); \
		if (n == 0) { delete this; } \
		return n; \
	}

