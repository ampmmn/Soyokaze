#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/core/CommandFile.h"
#include "commands/core/CommandIF.h"
#include "features/keywordmanager/CommandImportExport.h"

namespace {

class TestCommand : public launcherapp::core::Command
{
public:
	TestCommand(const CString& name, bool isSaveSuccessful = true) :
		mName(name),
		mIsSaveSuccessful(isSaveSuccessful)
	{
	}

	bool QueryInterface(const launcherapp::core::IFID&, void**) override { return false; }
	uint32_t AddRef() override { return ++mRefCount; }
	uint32_t Release() override { return --mRefCount; }
	CString GetName() override { return mName; }
	CString GetDescription() override { return _T(""); }
	CString GetTypeDisplayName() override { return _T(""); }
	bool CanExecute(String*) override { return false; }
	bool CanResolve() override { return false; }
	bool Resolve(CString&) override { return false; }
	bool IsAcceptArguments() override { return false; }
	bool GetAction(const HOTKEY_ATTR&, launcherapp::actions::core::Action**) override { return false; }
	HICON GetIcon() override { return nullptr; }
	int Match(Pattern*) override { return Pattern::Mismatch; }
	bool IsAllowAutoExecute() override { return false; }
	bool GetHotKeyAttribute(CommandHotKeyAttribute&) override { return false; }
	launcherapp::core::Command* Clone() override { return nullptr; }
	bool Save(CommandEntryIF* entry) override
	{
		if (mIsSaveSuccessful == false) {
			return false;
		}
		entry->Set(_T("value"), CString(_T("serialized")));
		return true;
	}
	bool Load(CommandEntryIF*) override { return false; }

private:
	CString mName;
	bool mIsSaveSuccessful;
	uint32_t mRefCount{1};
};

class TemporaryFile
{
public:
	~TemporaryFile()
	{
		if (mPath.IsEmpty() == FALSE) {
			DeleteFile(mPath);
			DeleteFile(mPath + _T(".tmp"));
		}
	}

	bool CreatePath()
	{
		TCHAR tempPath[MAX_PATH_NTFS]{};
		DWORD length = GetTempPath(_countof(tempPath), tempPath);
		if (length == 0 || length >= _countof(tempPath)) {
			return false;
		}

		TCHAR tempFile[MAX_PATH_NTFS]{};
		if (GetTempFileName(tempPath, _T("soy"), 0, tempFile) == 0) {
			return false;
		}
		if (DeleteFile(tempFile) == FALSE) {
			return false;
		}

		mPath = tempFile;
		return true;
	}

	const CString& GetPath() const
	{
		return mPath;
	}

private:
	CString mPath;
};

}

TEST(CommandImportExportTest, ExportsCommandDataToFile)
{
	TemporaryFile file;
	ASSERT_TRUE(file.CreatePath());

	TestCommand command(_T("sample"));
	CommandImportExport commandImportExport;
	auto result = commandImportExport.ExportCommands({&command}, file.GetPath());
	ASSERT_EQ(CommandImportExport::ExportError::None, result.mError);

	CommandFile commandFile;
	commandFile.SetFilePath(file.GetPath());
	ASSERT_TRUE(commandFile.Load());
	ASSERT_EQ(1, commandFile.GetEntryCount());
	EXPECT_EQ(CString(_T("serialized")), CommandFile::Get(commandFile.GetEntry(0), _T("value"), _T("")));
}

TEST(CommandImportExportTest, ReportsCommandSerializationFailure)
{
	TemporaryFile file;
	ASSERT_TRUE(file.CreatePath());

	TestCommand command(_T("sample"), false);
	CommandImportExport commandImportExport;
	auto result = commandImportExport.ExportCommands({&command}, file.GetPath());

	EXPECT_EQ(CommandImportExport::ExportError::CommandSaveFailed, result.mError);
	EXPECT_EQ(CString(_T("sample")), result.mCommandName);
}

TEST(CommandImportExportTest, ReportsFileSaveFailure)
{
	TemporaryFile file;
	ASSERT_TRUE(file.CreatePath());

	TestCommand command(_T("sample"));
	CommandImportExport commandImportExport;
	auto result = commandImportExport.ExportCommands({&command}, file.GetPath() + _T(".missing\\commands.ini"));

	EXPECT_EQ(CommandImportExport::ExportError::FileSaveFailed, result.mError);
}

TEST(CommandImportExportTest, FailedImportLoadLeavesNoCandidates)
{
	TemporaryFile file;
	ASSERT_TRUE(file.CreatePath());

	CommandImportExport commandImportExport;
	EXPECT_FALSE(commandImportExport.LoadCommands(file.GetPath()));
	EXPECT_TRUE(commandImportExport.GetImportCandidates().empty());
	EXPECT_TRUE(commandImportExport.GetSkippedEntryNames().empty());
}
