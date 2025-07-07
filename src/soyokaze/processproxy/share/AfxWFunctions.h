#pragma once

#include <string>
#include <vector>

// あふwの自窓のディレクトリパスを取得
bool AfxW_GetCurrentDir(std::wstring& curDir);
// あふwの自窓のディレクトリパスを変更
bool AfxW_SetCurrentDir(const std::wstring& path);
// あふwの選択中のファイルパスを取得(単一)
bool AfxW_GetSelectionPath(std::wstring& selPath, int index);
// あふwの選択中のファイルパスを取得(すべて)
bool Afxw_GetAllSelectionPath(std::vector<std::wstring>& paths);
