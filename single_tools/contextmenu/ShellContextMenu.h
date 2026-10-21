#pragma once

#include <windows.h>

#include <memory>
#include <string>
#include <vector>

/**
  シェルのコンテキストメニューを表示する
*/
class ShellContextMenu
{
public:
	ShellContextMenu();
	~ShellContextMenu();

	ShellContextMenu(const ShellContextMenu&) = delete;
	ShellContextMenu& operator=(const ShellContextMenu&) = delete;

	/**
	  指定されたパスのコンテキストメニューを表示する
	  @param[in] paths    対象のパスの一覧
	  @param[in] position 表示位置(スクリーン座標)
	  @return true:メニューからコマンドが選択された false:選択されなかった
	*/
	bool Show(const std::vector<std::wstring>& paths, const POINT& position);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
