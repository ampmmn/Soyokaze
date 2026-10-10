#include "stdafx.h"
#include <gtest/gtest.h>
#include <windows.h>
#include <filesystem>
#include <string>

/**
  ユニットテスト実行中の保存先(USERPROFILE)を一時フォルダに差し替える環境
	テストが実際のユーザー設定(~/.soyokaze)を書き換えないようにするため、テスト全体の開始前に差し替え、終了後に元へ戻す
*/
class UserProfileIsolationEnvironment : public ::testing::Environment
{
public:
	void SetUp() override
	{
		// 現在のUSERPROFILEを退避する
		size_t reqLen = 0;
		if (_tgetenv_s(&reqLen, nullptr, 0, _T("USERPROFILE")) == 0 && reqLen > 0) {
			std::vector<TCHAR> buf(reqLen);
			_tgetenv_s(&reqLen, buf.data(), buf.size(), _T("USERPROFILE"));
			mOriginal = buf.data();
		}

		// 一時フォルダ配下にプロセス単位の作業ディレクトリを作成し、USERPROFILEをそこへ向ける
		std::filesystem::path tempRoot = std::filesystem::temp_directory_path() / (L"SoyokazeUnitTest_" + std::to_wstring(GetCurrentProcessId()));
		std::filesystem::create_directories(tempRoot);
		mTempRoot = tempRoot.wstring();
		_tputenv_s(_T("USERPROFILE"), mTempRoot.c_str());
	}

	void TearDown() override
	{
		// USERPROFILEを元に戻す(空文字列の設定は変数の削除になる)
		_tputenv_s(_T("USERPROFILE"), mOriginal.c_str());

		// 作業ディレクトリを削除する(削除できなくてもテスト結果には影響させない)
		std::error_code ec;
		std::filesystem::remove_all(mTempRoot, ec);
	}

private:
	std::wstring mOriginal;
	std::wstring mTempRoot;
};

namespace {
[[maybe_unused]]
const auto* userprofile_env = ::testing::AddGlobalTestEnvironment(new UserProfileIsolationEnvironment());
}

