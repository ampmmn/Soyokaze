#pragma once

#include "commands/core/CommandIF.h"

// テスト用の共通部品

struct DummyCommand : public launcherapp::core::Command
{
	DummyCommand() : mDesc(_T("dummy description"))
	{
	}
	DummyCommand(LPCTSTR p) : mDesc(p)
	{
	}

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override
	{
		return false;
	}


	CString GetName() override
	{
		return _T("Dummy name");
	}

	CString GetDescription() override
	{
		return mDesc;
	}

	CString GetTypeDisplayName() override
	{
		return _T("");
	}

	CString GetGuideString() override
	{
		return _T("");
	}

	bool CanExecute() override
	{
		return true;
	}

	BOOL Execute(Parameter* param) override
	{
		return TRUE;
	}

	CString GetErrorString() override
	{
		return _T("");
	}

	HICON GetIcon() override
	{
		return nullptr;
	}

	int Match(Pattern* pattern) override
	{
		return Pattern::WholeMatch;
	}

	bool IsAllowAutoExecute() override
	{
		return false;
	}


	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override
	{
		return false;
	}

	Command* Clone() override
	{
		return new DummyCommand();
	}

	bool Save(CommandEntryIF* entry) override
	{
		return false;
	}

	bool Load(CommandEntryIF* entry) override
	{
		return false;
	}

	uint32_t AddRef() override
	{
		return ++mRefCount;
	}

	uint32_t Release() override
	{
		auto n = --mRefCount;
		if (n == 0) {
			delete this;
		}
		return n;
	}

	CString mDesc;
	uint32_t mRefCount{1};
};

