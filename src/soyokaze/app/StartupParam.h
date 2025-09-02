#pragma once

#include <memory>

// ランチャーアプリ起動時パラメータ
class StartupParam
{
public:
	StartupParam(int argc, TCHAR* argv[]);
	~StartupParam();

	// 実行コマンドを指定するオプションが指定されたか?
	bool HasRunCommand(String& commands);
	//
	void ShiftRunCommand();

	// 登録するパスが指定されたか?
	bool HasPathToRegister(String& pathToRegister);
	//
	bool HasHideOption();
	//
	bool HasPasteOption(String& value);
	//
	bool GetSelectRange(int& startPos, int& selLength);

	// ディレクトリ変更するオプションが指定されたか?
	bool HasChangeDirectoryOption(String& dirPath);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

