#pragma once

#include <windows.h>
#include <string>
#include <vector>

IDispatch* GetShellApplicationDispatch();
// Exploreで表示中のパスを取得
bool Expr_GetCurrentDir(std::wstring& curDir);
// Exploreで選択中のパスを取得
bool Expr_GetSelectionPath(std::wstring& selPath, int index);
bool Expr_GetAllSelectionPath(std::vector<std::wstring>& selPaths);



