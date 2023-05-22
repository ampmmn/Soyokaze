#pragma once

#include <memory>

// Soyokaze起動時パラメータ
class StartupParam
{
public:
	StartupParam(int argc, TCHAR* argv[]);
	~StartupParam();

	// 実行コマンドを指定するオプションが指定されたか?
	bool HasRunCommand(CString& commands);
	// 登録するパスが指定されたか?
	bool HasPathToRegister(CString& pathToRegister);
	//
	bool HasHideOption();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

