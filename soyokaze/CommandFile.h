#pragma once

#include "core/CommandIF.h"
#include <vector>

/**
 * コマンドデータをファイルに読んだり書いたりするためのクラス
 */
class CommandFile
{
public:
	CommandFile();
	~CommandFile();

	void SetFilePath(const CString& filePath);

public:
	bool Load(std::vector<soyokaze::core::Command*>& commands);
	bool Save(const std::vector<soyokaze::core::Command*>& commands);


	static void TrimComment(CString& s);

protected:
	CString mFilePath;
};

