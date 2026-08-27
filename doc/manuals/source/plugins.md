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
3. `Initialize`を呼び出してプラグインを初期化する
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
int Initialize(LAUNCHER_FUNCTION_TABLE* table);
```

プラグインのロード直後に一度呼び出されます。設定ファイルの読み込み、インデックスの作成、リソースの確保などをここで行います。

`LAUNCHER_FUNCTION_TABLE`は、{{ project }}本体がプラグインへ渡す関数テーブルです。プラグインから{{ project }}本体の機能を呼び出すための窓口であり、ログ出力、トースト通知、メインウィンドウのハンドル取得に使用できます。構造体の各メンバーには、本体側の機能を呼び出すための関数ポインタが設定されています。

この関数テーブルはプラグインが作成して{{ project }}へ返すものではありません。{{ project }}が`Initialize`の呼び出し時に作成し、引数としてプラグインへ渡します。プラグインで本体側APIを使用しない場合でも、`Initialize`の引数を受け取れるようにしてください。

`table`が指す`LAUNCHER_FUNCTION_TABLE`のデータは一時的なものです。後で使用する関数ポインタがある場合は、構造体の内容をプラグイン側でコピーして保持してください。

成功時は0、初期化に失敗した場合は0以外を返します。失敗したプラグインはロードされません。

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

返したアイコンの所有権はプラグイン側にあります。不要になったアイコンはプラグイン側で破棄してください。通常は`Finalize`で破棄します。

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

ログ出力関数と`PopupMessage`は、UTF-8文字列を受け取ります。

```cpp
static LAUNCHER_FUNCTION_TABLE gLauncher{};

int Initialize(LAUNCHER_FUNCTION_TABLE* table)
{
    if (table == nullptr) {
        return 1;
    }

    // 関数テーブルは呼び出し後も使用するため、コピーして保持する
    gLauncher = *table;
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

int Initialize(LAUNCHER_FUNCTION_TABLE* table)
{
    if (table == nullptr) {
        return 1;
    }
    gLauncher = *table;
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
        &Finalize,
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
- `GetIcon`で返したアイコンはプラグイン側で破棄する
- `Execute`の戻り値は、成功時だけ0にする
- `Finalize`から戻った後にプラグインのリソースを参照しない
- APIのバージョンが合わない場合は`LNCRPLUGIN_Bind`でエラーを返す

プラグインは{{ project }}のプロセス内で動作します。プラグイン内で発生した未処理例外やアクセス違反は、本体の動作にも影響します。外部ファイルの読み込みやWindows APIの呼び出しなど、プラグイン固有の処理については適切にエラー処理を行ってください。

