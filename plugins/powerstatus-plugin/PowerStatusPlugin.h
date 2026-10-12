#pragma once

#include <soyokaze/PluginExportTable.h>

namespace powerstatus {

/**
   プラグインを初期化し、バッテリー残量の監視を開始する
   @param[in]  table      本体側の関数テーブル
   @param[out] pluginInfo プラグイン情報JSON文字列の格納先
   @return 成功時は0、引数が不正な場合は0以外
*/
int Initialize(LAUNCHER_FUNCTION_TABLE* table, const char** pluginInfo);

/**
   プラグインを終了し、バッテリー残量の監視を停止する
*/
void Finalize();

} // powerstatus名前空間
