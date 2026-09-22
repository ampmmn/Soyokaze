# プロジェクトの概要

Windows環境で動作するコマンドラインランチャーです。アプリケーション名は`Soyokaze`です。

https://github.com/ampmmn/Soyokaze

言語や使用しているフレームワークは以下の通りです。

- C++で実装しています
- MFCを使用しています
- ユニットテストフレームワークとしてGoogle Testを使用しています
- 変更内容はプロジェクトルートにある CHANGELOG.md に記載します

- アプリ側のソースを生成したときは対応するクラスのテストコードも併せて生成してください

## 言語について

- 回答、ソースコードに挿入するコメントは特に指示しない限り基本的に日本語でお願いします

## プロジェクトのディレクトリ構成

- src  
ソースコード一式
- doc  
マニュアル用原稿、そのMakefile、配布バイナリに含める設定ファイルなど
- notes
内部用の設計ドキュメント
- tests  
ユニットテストコード
- plugin-include  
プラグイン用の公開ヘッダファイル
- externals  
サードパーティライブラリ
- image  
マニュアル
- tool  
内部用のツール、スクリプト

## コーディング規約

- Use CRLF line endings.
- Write all code comments in Japanese.
- Use UTF-8 encoding for any content containing Japanese characters.

- クラス名、関数名は`CamelCase`を使用します
  - 例: `class AliasCommand`  `bool AliasCommand::CreateNewInstanceFrom()`

- メンバ変数名は`m`で始まり、`CamelCase`を使用します。

- ユニットテストコードのファイル名はテスト対象ソースコードのファイル名の末尾に`Test`を付与します。  
  - 例: `ShortcutFile.cpp` の場合、 `ShortcutFileTest.cpp`

- 関数を追加する際には関数コメントを付与してください。形式としDoxygenで識別可能な形式で以下のような形式を採用しています。て
```
/**
  ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
 	@return true:成功  false:失敗
 	@param[in]  editor    Editorオブジェクト
 	@param[out] newCmdPtr 生成されたオブジェクト
*/
```
- 処理内容の理解を助けるため、生成したコードの要所で内容を説明した簡潔なコメントを入れてください

- PImplイディオムを用いてprivateなメンバ変数を隠蔽します。隠蔽する際のデータ構造として`PImpl`という名前の構造体を作成し、それを`std::unique_ptr`を使ってインスタンス化します。メンバ変数は`in`とします。

- このプロジェクトではプロコンパイル済ヘッダを使用しています。ヘッダ名は`pch.h`です。

- ブロックを開始するときの波括弧は改行せずに同じ行に記述してください
  - 例: `if (...) {`

- ブロックの中身が1行のコードであっても波括弧を省略しないでください

## プロジェクトのビルド

環境変数pythonLocationでPythonのパスを設定し、msbuildコマンドを実行する。

```
set pythonLocation=%USERPROFILE%\AppData\Local\Programs\Python\Python313
msbuild /m /p:Configuration=ReleaseStatic /p:Platform=x64 src\Soyokaze.sln
```

- msbuildへのパスが通っていない場合、下記のbatファイルを実行すると、ビルドに必要な環境変数が定義される。
`C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat`

- ローカルで実行するとき、ユーザーがビルド生成物を実行したままになっていて、そのせいでエラーになることがある。その場合はConfigurationを変えて試すとよい
  - Release
  - ReleaseStatic
  - Debug


## crlf-normalizer

When the user requests converting newline characters, normalizing CRLF, or fixing inconsistent line endings,
use the `crlf-normalizer` skill.

Ask clarifying questions if the target files or glob patterns are unclear.

