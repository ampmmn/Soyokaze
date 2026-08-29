# プラグイン機能

plugin-include/soyokaze/PluginExportTable.h にプラグインが実装してほしいAPIの定義を実装している。

これに従って実装したDLLを動的にロードし、既存のコマンドのインタフェースの仕組みに従ってプラグインとして機能できるようにする。  
これを実現するため、以下のような仕様で新規コマンドを実装している。

## 新規コマンドの詳細

- 名前空間は `launcherapp::commands::plugin` とする

- src/soyokaze/commands/plugin 以下にプラグインの制御を行うクラスを新設し、実装する。
  - 下記2つは必須とするが、必要に応じて機能を別のクラスに分割してもよい

- PluginProvider
  - プラグインコマンドを提供するクラス
  - インスタンスは1つだけ
  - AdhocCommandProviderBaseを実装する
  - コマンドの保存や読み込みをサポートしない
     - LoadCommands/IsPrivate/NewDialogなどはデフォルト実装のままとする
  - プラグインから取得したLNCRPLUGIN_EXPORTTABLEを保持する

- PluginCommand
  - このクラスで PluginExportTable.h に定義された関数を呼び出し、launcherapp::core::Command として機能するようにする
  - キーワードマッチング結果1件につき1つのインスタンスが生成される
    - PluginExportTable.hのLNCRPLUGIN_EXPORTTABLE::Queryによって得られた検索結果1件に対して、PluginCommandの1インスタンスを生成する
  - AdhocCommandBaseを実装する。このクラスは一時的なcommandを表す
  - PluginCommand::GetActionで生成したActionのPerformメソッド内でLNCRPLUGIN_EXPORTTABLE::Executeを実行する
    - CommandとActionの関係は他のコマンドクラスを参照のこと

## PluginProvider詳細

- PImplイディオムで内部データを隠蔽する

- PImplクラス側でAppPreferenceListenerIFを実装する

- PluginProvider::PrepareAdhocCommandsのタイミングでプラグインDLLのロードを行い、LNCRPLUGIN_EXPORTTABLEを取得する。プラグインロード処理の詳細は後述する。
  - 一度ロードしたら、以後はアプリ終了時までDLLの再ロード、アンロードはしない
  - プラグインの動的な追加はサポートしない

- ロード処理はPImpl側に実装する

- OnAppExitのタイミングでプラグインのアンロードを行う

- PluginProvider::QueryAdhocCommandsのなかで各プラグインに対するマッチングを行う
  - QueryAdhocCommandsの引数(pattern)への参照をMATCHER_FUNCTION_TABLEで定義する各関数への第一引数として渡す
  - MATCHER_FUNCTION_TABLEの各関数はPluginProvider内で定義する。この関数のなかで対応するPatternクラス側の同名の関数につなげる
  - LNCRPLUGIN_EXPORTTABLE::Queryを呼び、戻り値でLNCRPLUGINMATCHHANDLEを得る。この結果に基づき、PluginCommandを生成する

## PluginCommand詳細

- Commandクラスの各メソッド内において、LNCRPLUGIN_EXPORTTABLE側の対応するメソッドを呼ぶ。
  - GetGuideは対応するメソッドがない。GetGuideはコマンドがPluginCommandがGetActionメソッド内で生成するActionインスタンスのGetDisplayNameの戻り値を生成するために使用する。LNCRPLUGIN_EXPORTTABLE::GetGuideの出力が、Action::GetDisplayNameの戻り値となる。
  - GetAction呼び出し時にGuideを取得し、Action内にコピーを保持する

- GetIconに失敗した場合は、デフォルトアイコンへフォールバックする(IconLoader::LoadDefaultIcon)

- LNCRPLUGIN_EXPORTTABLEで定義する関数のchar*型の引数はUTF-8エンコーディングされたマルチバイト文字列を想定する。  
一方でPluginCommandの層では文字列をwchar_tとして扱うため、関数を呼ぶ際にはwchar_t → char(UTF-8)への変換を適宜行うこと。

- Command::GetAction内でActionインスタンスを生成する
  - このActionインスタンスのPerformメソッドの中で、LNCRPLUGIN_EXPORTTABLE::Executeを呼ぶ形とする
    - Performメソッドは実行時引数を文字列で受け取る仕様であるが、LNCRPLUGIN_EXPORTTABLE::Executeはmain関数のような引数(int, char*[])をとる。  
そこで、Performメソッドの層でデータ形式の変換を行い、Executeを呼び出す。(一時的な引数配列を生成し、これをargc,argvの様な形で渡せるようにする)
    - Executeの戻り値が非0だった場合、Performの戻り値はfalseとする

## プラグインロード処理詳細

- 起動後、1回だけプラグインロード処理をおこなう

- pluginsディレクトリがない場合はプラグインのロードは行わない
  - pluginsディレクトリがない旨はwarnログを出力する

- PluginProviderがDLLのロードを行う。
- 対象は.dllのみとする。他のファイルはLoadLibraryの対象としない。
- 同一ディレクトリに複数の.dllがあってもよい

- ユーザディレクトリ直下にあるpluginsディレクトリ直下のフォルダを走査し、フォルダ直下にあるすべてのDLLからプラグインのロードを試みる
以下のような階層を想定する。
- 再帰的な読み取りはしない。下記の階層に限定する。

```
userdir/
  plugins/
    plugin1/
      plugin1.dll
    plugin2/
      plugin2.dll
```

- PluginExportTable.hで定義しているLNCRPLUGIN_Bind関数の関数ポインタの取得を試みる。
  - 取得できなければ取得できなかった旨をwarnログとして出力しておく
  - LNCRPLUGIN_EXPORTTABLEに含まれる各メンバーが非nullptrであることをチェックする。一つでもnullptrがあったら、ロード失敗とみなす。(即FreeLibraryする)

- LNCRPLUGIN_EXPORTTABLEの構造体データを取得できたら、ProviderはDLLハンドルとLNCRPLUGIN_EXPORTTABLEを保持する。

- PluginProviderはLNCRPLUGIN_EXPORTTABLEを生成したら、プラグインを初期化するメソッドを呼び出す。LNCRPLUGIN_EXPORTTABLE::Initialize。
  - Initializeがエラーを返したら、そのプラグインDLLはProvider側では保持しない。即座にFreeLibraryとする。

## 検索ハンドルの寿命について

LNCRPLUGINMATCHHANDLEは複数の検索結果で共有されるため、std::shared_ptrでラップする  
カスタムデリータを定義し、カスタムデリータのなかでLNCRPLUGIN_EXPORTTABLE::CloseHandleを呼ぶ。

このstd::shared_ptrを各PluginCommandが持つ形とする

## DLLアンロードとCommand/Actionの寿命について

DLLはアプリ実行中に1回だけロードする方針とする。
DLLをアンロードするときはアプリ終了処理時となる。
このため、DLLアンロードが発生する時点でCommand/Actionはいなくなっている想定としてよい。

## プラグインアンロード処理詳細

- ProviderはLNCRPLUGIN_EXPORTTABLE::Finalizeをよぶ
- 解放処理を行った後は、自身が管理するリストからこれらの要素を削除する

- 基本的にはOnAppExitが実行されるタイミングでは検索は実行されていない想定
  - 万一、実行されていたとしても、プラグインアンロードが行われる段階ではアプリは修了しようとしているタイミングであるため、最悪落ちても支障はない

### DLLの解放について

DLLの解放(FreeLibrary)はPluginProviderのデストラクタで行う

## Executeのargv仕様について

実行時引数のみをargv配列に含める。先頭のコマンド名はargv配列には含めない。
引数がない場合、argcは0、argvは要素0のダミー配列となる

例えば、入力文字列が以下の様な形だった場合

```
mygrep keyword1 keyword2
```
argcは2, argv[0]は"keyword1", argv[1]は"keyword2"となる。


