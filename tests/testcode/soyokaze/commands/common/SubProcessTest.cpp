#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/common/SubProcess.h"
#include "actions/core/ActionParameter.h"

using launcherapp::commands::common::SubProcess;
using launcherapp::actions::core::ParameterBuilder;

TEST(SubProcessTest, SetAdditionalEnvironmentRejectsInvalidName)
{
	SubProcess exec(ParameterBuilder::EmptyParam());

	// 名前に空白や等号、引用符が含まれる場合は登録しない
	EXPECT_FALSE(exec.SetAdditionalEnvironment(_T("FOO=BAR"), _T("1")));
	EXPECT_FALSE(exec.SetAdditionalEnvironment(_T("FOO BAR"), _T("1")));
	EXPECT_FALSE(exec.SetAdditionalEnvironment(_T("FOO\"BAR"), _T("1")));
	EXPECT_FALSE(exec.SetAdditionalEnvironment(_T(""), _T("1")));

	// 正常使用できる名称は登録できる
	EXPECT_TRUE(exec.SetAdditionalEnvironment(_T("FOO"), _T("1")));
	EXPECT_TRUE(exec.SetAdditionalEnvironment(_T("FOO_BAR"), _T("1")));
}

TEST(SubProcessTest, RunFailsWhenPathIsEmpty)
{
	SubProcess exec(ParameterBuilder::EmptyParam());
	SubProcess::ProcessPtr process;

	EXPECT_FALSE(exec.Run(_T(""), process));

	// 失敗した場合もプロセスオブジェクトが返る(エラーメッセージを取得できるようにするため)
	EXPECT_TRUE(process != nullptr);
}

TEST(SubProcessTest, RunRejectsEnvReferenceForBatchFile)
{
	SubProcess exec(ParameterBuilder::EmptyParam());
	SubProcess::ProcessPtr process;

	// バッチファイル経由で実行する場合、環境変数の参照を含む引数は拒否される
	EXPECT_FALSE(exec.Run(_T("C:\\test.bat"), _T("%PATH%"), process));

	EXPECT_TRUE(process != nullptr);
	EXPECT_FALSE(process->GetErrorMessage().IsEmpty());
}

TEST(SubProcessTest, InstanceWithoutProcessHandleCannotWait)
{
	SubProcess::Instance process(nullptr);

	EXPECT_FALSE(process.Wait(0));
}
