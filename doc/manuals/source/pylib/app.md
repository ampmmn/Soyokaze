# App Module

アプリ本体の機能にアクセスする機能を提供するモジュール。
importしなくても使うことができる。

## Methods

### popup_message

メッセージを表示する

```python
app.popup_message("こんにちは")
```

#### Arguments

- messsage(String)  
表示するメッセージ

#### Return Value

正常終了時0、エラー時1(引数が不正)

### run_command

ランチャーアプリのコマンドを実行する  
入力欄に文字列を入力して実行したときの動作をPythonスクリプトから呼び出すことができる。

```python
# バージョン情報を表示する
app.run_command("version")
```

#### Arguments

- input_text(String)  
コマンド文字列

#### Return Value

正常終了時0、エラー時1(引数が不正)

### expand_macro

アプリ内の[マクロ](/macros)を展開して文字列として返す。

```python
# エクスプローラで表示しているディレクトリを得る
explore_dir = app.expand_macro("${explorer location_path}")
```

#### Arguments

- input_text(String)  
コマンド文字列


#### Return Value

マクロの展開結果(文字列)。  
引数が不正な場合は空文字を返す


