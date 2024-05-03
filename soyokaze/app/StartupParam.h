#pragma once

#include <memory>

// ランチャーアプリ起動時パラメータ
class StartupParam
{
public:
	StartupParam(int argc, TCHAR* argv[]);
	~StartupParam();

	// 実行コマンドを指定するオプションが指定されたか?
	bool HasRunCommand(CString& commands);
	//
	void ShiftRunCommand();

	// 登録するパスが指定されたか?
	bool HasPathToRegister(CString& pathToRegister);
	//
	bool HasHideOption();
	//
	bool HasPasteOption(CString& value);
	//
	bool GetSelectRange(int& startPos, int& selLength);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

