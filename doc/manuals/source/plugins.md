# Plugins

本アプリは専用のプラグインDLLを読み込み、検索候補となるコマンドを追加できます。
ここでは、C/C++でプラグインを実装するための仕様と実装方法を説明します。

```{only} soyokaze
プラグインのAPI定義は、配布物に含まれる
`plugin-include/soyokaze/PluginExportTable.h` が基準です。APIの追加や変更に追従するため、
プラグインのビルドには、使用するSoyokazeと同じバージョンのヘッダを使用してください。
```

```{only} not soyokaze
プラグインのAPI定義は、`PluginExportTable.h` が基準です。APIの追加や変更に追従するため、
プラグインのビルドには、使用するランチャーアプリ本体と同じバージョンのヘッダを使用してください。
```

本体のバージョンアップにより、`PluginExportTable.h`の定義は変更される場合があります。  
アプリ本体はプラグインの後方互換性を維持しないため、定義が変更になった場合は追従が必要です。

## 動作の概要

プラグインは、{{ project }}が入力中のキーワードを検索するときに呼び出されます。
プラグインは入力内容を調べ、検索結果をハンドルとして返します。検索結果1件につき1つの{{ project }}上のコマンドが生成され、ユーザーがそのコマンドを実行するとプラグインの処理が呼び出されます。

処理の流れは次のとおりです。

1. {{ project }}がプラグインDLLをロードする
2. `LNCRPLUGIN_Bind`を呼び出して関数テーブルを取得する
3. `Initialize`を呼び出してプラグインを初期化し、プラグイン情報を取得する
4. キーワード入力のたびに`Query`を呼び出す
5. `Query`が返した検索結果を候補として表示する
6. コマンド実行時に`CanExecute`と`Execute`を呼び出す
7. {{ project }}の終了時に`Finalize`を呼び出してDLLをアンロードする

DLLはアプリケーションの実行中に一度だけロードされます。プラグインの追加・削除を実行中に反映することはできません。

## 配置とロード

プラグインは、次のいずれかの`plugins`ディレクトリへ配置します。

- {{ project }}の実行ファイルがあるディレクトリ
- {{ project }}の設定ファイル保存先（ユーザーディレクトリ直下）

実行ファイルがあるディレクトリの`plugins`から先にロードされます。プラグインごとにサブディレクトリを作成してください。

```text
実行ファイルのディレクトリ/
  plugins/
    hello/
      hello.dll

ユーザーディレクトリ/
  plugins/
    another-plugin/
      another-plugin.dll
```

ロード時の規則は次のとおりです。

- `plugins`ディレクトリ直下のサブディレクトリを走査します
- 各サブディレクトリの直下にある`.dll`をロード対象とします
- 1つのサブディレクトリに複数のDLLを配置できます
- サブディレクトリをさらに再帰して探索することはありません
- `plugins`ディレクトリが存在しない場合、プラグインはロードされません
- `.dll`以外のファイルはロードされません
- 32bit版はサポートしていません。x64版の{{ project }}に対応するx64 DLLを作成してください

次のいずれかに該当するDLLはロードに失敗します。

- DLLをロードできない
- `LNCRPLUGIN_Bind`を取得できない
- `LNCRPLUGIN_Bind`がエラーを返す
- `LNCRPLUGIN_EXPORTTABLE`の関数ポインタに`nullptr`が含まれる
- `Initialize`の第2引数から取得したプラグイン情報をJSONとして解析できない
- プラグイン情報の`pluginApiVersion`が{{ project }}の`PLUGINVERSION`と一致しない
- `Initialize`がエラーを返す

ロードに失敗したDLLは保持されず、ロード済みのDLLも終了時まで再ロードされません。

## 公開するAPI

プラグインは`LNCRPLUGIN_Bind`という名前の関数をエクスポートします。
関数の宣言はヘッダに定義されています。

```cpp
int LNCRPLUGIN_API
LNCRPLUGIN_Bind(int version, LNCRPLUGIN_EXPORTTABLE* table);
```

`version`には{{ project }}が使用するプラグインバージョンが渡されます。プラグイン側では、利用している`PluginExportTable.h`の`PLUGINVERSION`に対応できるか確認してください。バージョンに対応できない場合は0以外を返します。

正常に関数テーブルを設定できた場合は0を返してください。
`table`には、プラグインが実装した`LNCRPLUGIN_EXPORTTABLE`の各関数ポインタを設定します。現在の実装では、全ての関数ポインタが設定されている必要があります。

`LNCRPLUGIN_Bind`はCリンケージでエクスポートされます。ヘッダをC++からインクルードすれば、宣言に含まれる`extern "C"`が適用されます。関数名を変更したり、C++の名前修飾が付いた状態でエクスポートしたりしないでください。

## 関数テーブル

### `Initialize`

```cpp
int Initialize(LAUNCHER_FUNCTION_TABLE* table, const char** plugin_info);
```

プラグインのロード直後に一度呼び出されます。設定ファイルの読み込み、インデックスの作成、リソースの確保などをここで行います。

`plugin_info`には、プラグインの情報を表すJSON文字列へのポインタを設定してください。JSON文字列の所有権はプラグイン側にあり、{{ project }}本体は文字列を解放しません。{{ project }}本体は受け取ったJSONをコピーして管理します。

`LAUNCHER_FUNCTION_TABLE`は、{{ project }}本体がプラグインへ渡す関数テーブルです。プラグインから{{ project }}本体の機能を呼び出すための窓口であり、ログ出力、トースト通知、メインウィンドウのハンドル取得に使用できます。構造体の各メンバーには、本体側の機能を呼び出すための関数ポインタが設定されています。

この関数テーブルはプラグインが作成して{{ project }}へ返すものではありません。{{ project }}が`Initialize`の呼び出し時に作成し、引数としてプラグインへ渡します。プラグインで本体側APIを使用しない場合でも、`Initialize`の引数を受け取れるようにしてください。

`table`が指す`LAUNCHER_FUNCTION_TABLE`のデータは一時的なものです。後で使用する関数ポインタがある場合は、構造体の内容をプラグイン側でコピーして保持してください。

成功時は0、初期化に失敗した場合は0以外を返します。失敗したプラグインはロードされません。

#### プラグイン情報

プラグイン情報はJSONオブジェクトで指定します。`pluginApiVersion`は必須で、プラグインのビルドに使用した`PluginExportTable.h`の`PLUGINVERSION`と一致させてください。`pluginId`にはプラグインを識別する一意な文字列を指定してください。その他のキーは表示や管理に使用されます。

```json
{
  "displayName": "プラグイン表示名",
  "pluginId": "プラグインを識別する一意な文字列",
  "pluginVersion": "1.0.0",
  "pluginApiVersion": 102,
  "pluginDescription": "プラグインの概要",
  "pluginDeveloper": "制作者名など",
  "pluginLicenseName": "プラグインのライセンス",
  "url": "プラグインに関するURL"
}
```

上記以外のキーを追加することもできます。{{ project }}本体が認識しないキーは無視されます。JSONとして解析できない場合や、`pluginApiVersion`が{{ project }}本体の`PLUGINVERSION`と異なる場合、プラグインはロードされません。

### `Query`

```cpp
LNCRPLUGINMATCHHANDLE Query(void* ctx, MATCHER_FUNCTION_TABLE* table);
```

入力中のキーワードに対して検索を行い、検索結果を保持するハンドルを返します。検索結果がない場合は`nullptr`を返してください。

`ctx`は{{ project }}が管理する検索状態です。`MATCHER_FUNCTION_TABLE`の関数を呼び出すとき、必ずこの値を第1引数として渡します。`ctx`の内容をプラグイン側で解釈したり、保持したりしないでください。

`table`には、次のキーワード操作関数が設定されています。

|関数|内容|
|--|--|
|`Match`|指定したキーワードと一致するか調べる|
|`GetFirstWord`|入力中の最初のワードを取得する|
|`GetWholeString`|入力全体を取得する|
|`GetWordCount`|入力中のワード数を取得する|

`Match`のオフセットは、`Pattern::Match`のオフセットと同じ意味です。例えば、最初のワードをプラグインのコマンド名として扱う場合は、2番目以降のワードを検索するためにオフセット1を指定します。

### `GetMatchCount` / `GetMatchLevel`

```cpp
int GetMatchCount(LNCRPLUGINMATCHHANDLE handle);
int GetMatchLevel(LNCRPLUGINMATCHHANDLE handle, int index);
```

`GetMatchCount`は検索結果の件数を返します。`index`は0から始まる検索結果の番号です。

`GetMatchLevel`は検索結果の一致レベルを返します。値の意味は次のとおりです。

|値|意味|
|--:|--|
|5|完全一致|
|4|前方一致|
|3|部分一致|
|2|弱い一致|
|-1|不一致|

### `GetName` / `GetDescription` / `GetGuide` / `GetTypeDisplayName`

```cpp
int GetName(LNCRPLUGINMATCHHANDLE handle, int index,
            char* buffer, size_t length);
int GetDescription(LNCRPLUGINMATCHHANDLE handle, int index,
            char* buffer, size_t length);
int GetGuide(LNCRPLUGINMATCHHANDLE handle, int index,
            char* buffer, size_t length);
int GetTypeDisplayName(LNCRPLUGINMATCHHANDLE handle, int index,
            char* buffer, size_t length);
```

これらの関数は、検索結果ごとの文字列を取得します。

|関数|画面上での用途|
|--|--|
|`GetName`|候補の名前|
|`GetDescription`|候補の説明|
|`GetGuide`|候補を実行するときのガイド表示|
|`GetTypeDisplayName`|コマンド種別の表示|

文字列はUTF-8の`char`配列として返してください。必要なバッファ長を調べるときは`buffer`に`nullptr`、`length`に0を指定します。この場合、終端の`\0`を含む必要なバイト数を返します。

バッファを渡した場合は、文字列を`\0`で終端してください。バッファが不足する場合も、可能な範囲で終端文字を含めてコピーします。エラー時は-1を返します。

`GetGuide`で取得した文字列は、コマンド実行時のアクション表示に使用されます。

### `CanExecute`

```cpp
int CanExecute(LNCRPLUGINMATCHHANDLE handle, int index);
```

検索結果を現在実行できるかどうかを返します。0は実行不可、0以外は実行可能です。

実行不可の場合、{{ project }}は`GetErrorString`で理由を取得して表示します。

### `Execute`

```cpp
int Execute(LNCRPLUGINMATCHHANDLE handle, int index,
            int argc, char** argv);
```

検索結果を実行します。成功時は0、失敗時は0以外を返してください。

`argv`には実行時引数だけが含まれます。コマンド名は含まれません。

例えば、入力が次のような場合を考えます。

```text
mygrep keyword1 keyword2
```

プラグインに渡される値は次のとおりです。

```text
argc    = 2
argv[0] = "keyword1"
argv[1] = "keyword2"
```

引数がない場合は`argc`が0になります。`argv`は終端の`nullptr`を持つダミー配列として渡されます。

### `GetErrorString`

```cpp
int GetErrorString(LNCRPLUGINMATCHHANDLE handle, int index,
                   char* buffer, size_t length);
```

`CanExecute`または`Execute`が失敗したときに表示するエラー文字列を返します。文字列の取得方法は`GetName`などの文字列取得関数と同じです。

### `GetIcon`

```cpp
int GetIcon(LNCRPLUGINMATCHHANDLE handle, int index, HICON* icon);
```

検索結果に表示するアイコンを返します。成功時は0を返し、`icon`にアイコンハンドルを設定します。失敗時は0以外を返してください。

返したアイコンの所有権はプラグイン側にあります。アプリ側はアイコンハンドルの解放を行わないため、プラグイン側で適切に解放してください。通常は`Finalize`で破棄します。

`GetIcon`に失敗した場合、{{ project }}はデフォルトアイコンを使用します。

### `CloseHandle`

```cpp
void CloseHandle(LNCRPLUGINMATCHHANDLE handle);
```

`Query`が返した検索ハンドルを破棄します。検索ハンドルに確保したメモリや検索結果の内部データは、この関数で解放してください。

1つの検索ハンドルに複数の検索結果を含めることができます。{{ project }}は検索結果を表示している間、ハンドルを保持します。`CloseHandle`が呼ばれるまで、ハンドルとその配下の検索結果を有効にしておいてください。

### `Finalize`

```cpp
void Finalize(void);
```

{{ project }}の終了時に一度呼び出されます。`Initialize`で確保したリソース、`GetIcon`で作成したアイコン、設定ファイルやインデックスなどを解放してください。

`Finalize`の後にプラグインDLLがアンロードされます。関数から戻った後に、プラグイン内の関数やデータを参照しないでください。

## 本体側API

`LAUNCHER_FUNCTION_TABLE`には、{{ project }}本体が提供する次の関数が含まれます。プラグインは`Initialize`で受け取った構造体を保存し、必要なタイミングで各関数ポインタを呼び出します。

|関数|内容|
|--|--|
|`InfoLog`|情報ログを出力する|
|`WarnLog`|警告ログを出力する|
|`ErrorLog`|エラーログを出力する|
|`PopupMessage`|トースト通知を表示する|
|`GetMainWindowHandle`|メインウィンドウの`HWND`を取得する|
|`LoadIconFromPath`|ファイルパスに関連付けられたアイコンを取得する|
|`LoadExtensionIcon`|ファイル拡張子に関連付けられたアイコンを取得する|
|`HasIcon`|アイコンが本体側で管理されているか確認する|
|`OpenFolder`|設定されたファイラーでフォルダを開く|

アイコン関連の関数の宣言は次のとおりです。

```cpp
HICON LoadIconFromPath(const char* path);
HICON LoadExtensionIcon(const char* fileExt);
int HasIcon(HICON icon);
int OpenFolder(const char* path);
```

ログ出力関数と`PopupMessage`は、UTF-8文字列を受け取ります。
`LoadIconFromPath`と`LoadExtensionIcon`の引数もUTF-8文字列です。取得したアイコンは本体側が所有するため、プラグイン側で破棄しないでください。
`HasIcon`は、本体側で管理されているアイコンの場合に1、それ以外の場合に0を返します。
`OpenFolder`の引数はUTF-8文字列です。設定されたファイラーが利用できない場合はExplorerでフォルダを開きます。戻り値は成功時に0、失敗時に0以外です。

```cpp
static LAUNCHER_FUNCTION_TABLE gLauncher{};
static const char* gPluginInfo = R"({
  "pluginId": "sample-plugin",
  "pluginApiVersion": 102
})";

int Initialize(LAUNCHER_FUNCTION_TABLE* table, const char** plugin_info)
{
    if (table == nullptr || plugin_info == nullptr) {
        return 1;
    }

    // 関数テーブルは呼び出し後も使用するため、コピーして保持する
    gLauncher = *table;
    *plugin_info = gPluginInfo;
    gLauncher.InfoLog("sample plugin initialized");
    gLauncher.PopupMessage("Sample plugin is ready");
    return 0;
}
```

`GetMainWindowHandle`で得た`HWND`を、Windows APIを呼び出すときの親ウィンドウとして利用できます。

```cpp
HWND mainWindow = gLauncher.GetMainWindowHandle();
MessageBoxW(mainWindow, L"プラグインからのメッセージ", L"Sample", MB_OK);
```

## 最小実装例

以下は、入力が`hello`のときに1件の候補を返し、実行時にログを出力する最小構成の例です。実際のDLLでは、検索結果をプラグイン固有の構造体で保持し、`Query`で動的に生成してください。

この例では、説明を簡潔にするため、検索ハンドルを1件の検索結果そのものとして扱っています。

```cpp
#include "PluginExportTable.h"
#include <algorithm>
#include <cstring>
#include <string>

namespace {

LAUNCHER_FUNCTION_TABLE gLauncher{};
const char* gPluginInfo = R"({
  "displayName": "Sample Plugin",
  "pluginId": "sample-plugin",
  "pluginVersion": "1.0.0",
  "pluginApiVersion": 102,
  "pluginDescription": "Sample plugin command"
})";

struct Match {
    std::string name = "hello";
    std::string description = "Sample plugin command";
    std::string guide = "Run the sample plugin";
    std::string type = "Sample";
};

int CopyString(const std::string& value, char* buffer, size_t length)
{
    const size_t required = value.size() + 1;
    if (buffer == nullptr || length == 0) {
        return static_cast<int>(required);
    }

    const size_t copied = std::min(value.size(), length - 1);
    std::memcpy(buffer, value.data(), copied);
    buffer[copied] = '\0';
    return static_cast<int>(copied);
}

int Initialize(LAUNCHER_FUNCTION_TABLE* table, const char** plugin_info)
{
    if (table == nullptr || plugin_info == nullptr) {
        return 1;
    }
    gLauncher = *table;
    *plugin_info = gPluginInfo;
    return 0;
}

LNCRPLUGINMATCHHANDLE Query(void* ctx, MATCHER_FUNCTION_TABLE* table)
{
    if (ctx == nullptr || table == nullptr || table->GetWholeString == nullptr) {
        return nullptr;
    }

    const char* input = table->GetWholeString(ctx);
    if (input == nullptr || std::strcmp(input, "hello") != 0) {
        return nullptr;
    }
    return new Match();
}

int GetMatchCount(LNCRPLUGINMATCHHANDLE)
{
    return 1;
}

int GetMatchLevel(LNCRPLUGINMATCHHANDLE, int)
{
    return 5;
}

int GetName(LNCRPLUGINMATCHHANDLE handle, int, char* buffer, size_t length)
{
    return CopyString(static_cast<Match*>(handle)->name, buffer, length);
}

int GetDescription(LNCRPLUGINMATCHHANDLE handle, int, char* buffer, size_t length)
{
    return CopyString(static_cast<Match*>(handle)->description, buffer, length);
}

int GetGuide(LNCRPLUGINMATCHHANDLE handle, int, char* buffer, size_t length)
{
    return CopyString(static_cast<Match*>(handle)->guide, buffer, length);
}

int GetTypeDisplayName(LNCRPLUGINMATCHHANDLE handle, int, char* buffer, size_t length)
{
    return CopyString(static_cast<Match*>(handle)->type, buffer, length);
}

int CanExecute(LNCRPLUGINMATCHHANDLE, int)
{
    return 1;
}

int Execute(LNCRPLUGINMATCHHANDLE, int, int argc, char** argv)
{
    if (gLauncher.InfoLog != nullptr) {
        gLauncher.InfoLog(argc == 0 ? "hello executed" : argv[0]);
    }
    return 0;
}

int GetErrorString(LNCRPLUGINMATCHHANDLE, int, char*, size_t)
{
    return 0;
}

int GetIcon(LNCRPLUGINMATCHHANDLE, int, HICON*)
{
    return 1;
}

void CloseHandle(LNCRPLUGINMATCHHANDLE handle)
{
    delete static_cast<Match*>(handle);
}

void Finalize()
{
}

} // 名前空間

extern "C" int LNCRPLUGIN_API
LNCRPLUGIN_Bind(int version, LNCRPLUGIN_EXPORTTABLE* table)
{
    if (table == nullptr || version != PLUGINVERSION) {
        return 1;
    }

    *table = {
        &Initialize,
        &Finalize,
        &Query,
        &GetMatchCount,
        &GetMatchLevel,
        &CloseHandle,
        &GetName,
        &GetDescription,
        &GetGuide,
        &GetTypeDisplayName,
        &CanExecute,
        &Execute,
        &GetErrorString,
        &GetIcon,
    };
    return 0;
}
```

この例をDLLとしてビルドするには、{{ project }}から取得した`PluginExportTable.h`をインクルードし、x64のDLLプロジェクトとしてビルドします。生成したDLLを`plugins`配下のプラグイン用サブディレクトリに配置し、{{ project }}を再起動してください。

## 文字コードとABI

`PluginExportTable.h`で定義された`char*`型の文字列はUTF-8です。Windowsのワイド文字列を扱う場合は、プラグイン側でUTF-8との変換を行ってください。

プラグインと{{ project }}本体は、同じ`PluginExportTable.h`の定義、同じCPUアーキテクチャ、互換性のあるビルド環境で使用してください。32bit DLLをx64版{{ project }}からロードすることはできません。

`LNCRPLUGINMATCHHANDLE`の実体はプラグイン側が定義します。ハンドルはプラグイン間で共通に見えても、異なるプラグインのハンドルを別のプラグインの関数に渡すことはできません。

## 実装時の注意

- `Initialize`に渡された`LAUNCHER_FUNCTION_TABLE`は、必要に応じてコピーして保持する
- `Query`が返したハンドルは、`CloseHandle`が呼ばれるまで有効にする
- `GetMatchCount`が返す件数と、各APIに渡される`index`の範囲を一致させる
- 文字列はUTF-8で返し、必要なバッファ長には終端文字を含める
- `GetIcon`で返したアイコンはプラグイン側で適切に解放する
- `LoadIconFromPath`と`LoadExtensionIcon`で返したアイコンは本体側で管理されるため、プラグイン側で破棄しない
- `HasIcon`の戻り値は、管理対象の場合が1、管理対象外の場合が0
- `Execute`の戻り値は、成功時だけ0にする
- `Finalize`から戻った後にプラグインのリソースを参照しない
- APIのバージョンが合わない場合は`LNCRPLUGIN_Bind`でエラーを返す

プラグインは{{ project }}のプロセス内で動作します。プラグイン内で発生した未処理例外やアクセス違反は、本体の動作にも影響します。外部ファイルの読み込みやWindows APIの呼び出しなど、プラグイン固有の処理については適切にエラー処理を行ってください。

