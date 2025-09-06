#include "stdafx.h"
#include "gtest/gtest.h"
#include "app/CommandLineProcessor.h"
#include "app/SecondProcessProxyIF.h"

using namespace launcherapp;

class TestProxy : public SecondProcessProxyIF
{
public:
	// コマンド文字列を通知する
	bool SendCommandString(const String& commandStr, bool isPasteOnly)
	{
		mHist += "S";  // Send
		mCommandStr = commandStr;
		mIsPaste= isPasteOnly;
		return true;
	}
	// 選択範囲を通知する
	bool SendCaretRange(int startPos, int length)
	{
		mHist += "C";  // Caret
		mStartPos = startPos;
		mLength = length;
		return true;
	}
	// パス登録
	bool RegisterPath(const String& pathStr)
	{
		mHist += "R";  // Register
		mRegisterPath = pathStr;
		return true;
	}
	// カレントディレクトリを変更する
	bool ChangeDirectory(const String& pathStr)
	{
		mHist += "D";   // Directory
		mDir = pathStr;
		return true;
	}
	// ウインドウを消す
	bool Hide()
	{
		mHist += "H";   // Hide
		return true;
	}
	// ウインドウを表示する
	bool Show()
	{
		mHist += "A";   // Activate
		return true;
	}

public:
	String mHist;
	String mCommandStr;
	String mRegisterPath;
	String mDir;
	int mStartPos = -1;
	int mLength = -1;
	bool mIsPaste = false;
};



// 何もオプションを指定しない場合は単にアプリを表示する
TEST(CommandLineProcessor, Show)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe") };
	processor.Run(1, args, &proxy);

	EXPECT_EQ("A", proxy.mHist);
}

// /Hideオプションを指定された場合はアプリを非表示にする
TEST(CommandLineProcessor, Hide)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("/Hide") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("H", proxy.mHist);
}

// /Pasteオプションが指定された場合は範囲選択
TEST(CommandLineProcessor, Paste1)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("/Paste"), (TCHAR*)_T("hoge") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("AS", proxy.mHist);
	EXPECT_EQ("hoge", proxy.mCommandStr);
	EXPECT_TRUE(proxy.mIsPaste);
}

// /Paste=オプションが指定された場合は範囲選択
TEST(CommandLineProcessor, Paste2)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("/Paste=hogehoge") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("AS", proxy.mHist);
	EXPECT_EQ("hogehoge", proxy.mCommandStr);
	EXPECT_TRUE(proxy.mIsPaste);
}

// パス登録
TEST(CommandLineProcessor, Register)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	// FIXME:環境によっては、システムドライブがC:\でない場合もある
	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("C:\\windows") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("R", proxy.mHist);
	EXPECT_EQ("C:\\windows", proxy.mRegisterPath);
	EXPECT_FALSE(proxy.mIsPaste);
}

// キャレット選択
TEST(CommandLineProcessor, Caret1)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("/Paste=hogehoge"), (TCHAR*)_T("/SelStart=1") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("ASC", proxy.mHist);
	EXPECT_EQ(1, proxy.mStartPos);
	EXPECT_EQ(0, proxy.mLength);
}

// キャレット選択
TEST(CommandLineProcessor, Caret2)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("/Paste=hogehoge"), (TCHAR*)_T("/SelStart=2"), (TCHAR*)_T("/SelLength=2") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("ASC", proxy.mHist);
	EXPECT_EQ(2, proxy.mStartPos);
	EXPECT_EQ(2, proxy.mLength);
}

// キャレット選択
TEST(CommandLineProcessor, Caret3)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("/Paste=hogehoge"), (TCHAR*)_T("/SelStart=-1") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("ASC", proxy.mHist);
	EXPECT_EQ(-1, proxy.mStartPos);
}

// コマンド実行
TEST(CommandLineProcessor, RunCommand1)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("-c"), (TCHAR*)_T("home") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("S", proxy.mHist);
	EXPECT_EQ("home", proxy.mCommandStr);
	EXPECT_FALSE(proxy.mIsPaste);
}

// コマンド実行
TEST(CommandLineProcessor, RunCommand2)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("/Runcommand=fuga") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("S", proxy.mHist);
	EXPECT_EQ("fuga", proxy.mCommandStr);
	EXPECT_FALSE(proxy.mIsPaste);
}

// コマンド実行
TEST(CommandLineProcessor, RunCommand3)
{
	TestProxy proxy;
	CommandLineProcessor processor;

	TCHAR* args[] = { (TCHAR*)_T("hoge.exe"), (TCHAR*)_T("/Runcommand=fuga"), (TCHAR*)_T("/Runcommand=bar") };
	processor.Run(sizeof(args)/sizeof(TCHAR*), args, &proxy);

	EXPECT_EQ("SS", proxy.mHist);
	EXPECT_EQ("bar", proxy.mCommandStr);
	EXPECT_FALSE(proxy.mIsPaste);
}

