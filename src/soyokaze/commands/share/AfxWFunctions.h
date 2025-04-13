#pragma once

#include <string>

// あふの自窓のディレクトリパスを取得
bool AfxW_GetCurrentDir(std::wstring& curDir);
// あふの自窓のディレクトリパスを変更
bool AfxW_SetCurrentDir(const std::wstring& path);

