# workspaceプラグイン

## 概要

指定したディレクトリ階層以下にあるフォルダ、ファイルのインデックスを生成し、検索するプラグイン  

プラグイン初期化時にインデックスを作成する。  
キーワードマッチング要求のタイミングで、インデックスに基づきマッチングを行い、合致した要素の情報を返す。

実行するタイミングで、該当する要素を開く。フォルダ、ファイルごとに実行するコマンドを定義しておき、その定義に従う。

## 詳細

プラグイン初期化(Initialize)のタイミングで、インデックスを作成する。  
インデックス作成は時間がかかる可能性があるため、別スレッドで行う。インデックス構築が完了するまでは検索を実行しない。  
一度作成したら更新はしない。更新したい場合はアプリ本体プロセスを再起動する。

インデックス作成対象のパスは専用の設定ファイルで定義する。

検索コマンドの第一ワードが発動ワードと一致しない場合はインデックスとのマッチングを行わない。発動ワードは設定ファイルで指定可能。  
発動ワードを設定しないことにより、無条件に検索を実行させることも可能

## 設定ファイルの仕様

- YAML形式
- ファイル名は`settings.yaml`とする
- パス指定は全般的に大文字小文字を区別しない(Windowsパス仕様に従う)

- ルート配列の各要素でインデックスを定義できる
- directoryキーはインデックス作成対象パス。複数指定可能
- search-limitはワークスペースごとの1回の検索結果の上限数。上限数に達したらそのインデックスに対する検索を打ち切る
- max-depthはディレクトリ深度。0でディレクトリ直下のみを検索対象としサブディレクトリを検索しない。-1で制限なし。それ以外の負の値はエラー。
- search-trigger-wordは検索を発動ワード。指定がない場合は無条件で発動する。
  - 指定がない場合はMatch関数のoffsetを0にする必要あり
- include-extはインデックス作成対象に含めるファイル拡張子
  - 全拡張子を含める場合は "*" とする
  - include-extが空の場合はフォルダのみ検索を行う
- exclude-fileはインデックス作成対象から除外するファイル名パターン(部分一致)
  - 単純な部分一致のみをサポート。正規表現やワイルドカード指定は不可
- exclude-dirはインデックス作成対象から除外するディレクトリ名パターン(部分一致)
  - 単純な部分一致のみをサポート。正規表現やワイルドカード指定は不可
- executeはファイル、フォルダごとの実行コマンドライン
  - これは必須(folder/fileとも)。キーがない、空の場合はエラーとし、そのworkspaceを設定しない。
- ファイルを実行するときにCtrlキーが押されている場合は、実行コマンドの代わりに対象ファイルのフォルダを本体側で設定したファイラーで開く

```yaml
- directory:
    - c:/path/to/directory
    - c:/path2/to/directory2
  search-limit: 8
  max-depth: 3
  search-trigger-word: ws
  include-ext:
    - .cpp
    - .h
  exclude-file:
    - hoge
  exclude-dir:
    - .git
  execute:
    folder:
      - explore.exe
      - "{path}"
    file:
        cpp:
          - C:/Users/htmny/local/apps/vim/gvim.exe
          - --remote-silent
          - "{path}"
        default:
          - C:/Users/htmny/local/apps/vim/gvim.exe
          - --remote-silent
          - "{path}"
```

## 検索結果の仕様

- 名前は <search-trigger-word> <検索ルートディレクトリからの相対パス>とする。
 - 発動ワードがない場合は相対パスのみを表示
- 説明は<フルパス>とする
- アイコンはファイルやフォルダに関連付けられたアイコンを本体の`LAUNCHER_FUNCTION_TABLE::LoadIconFromPath`で取得する
- 取得したHICONは本体側が管理するため、プラグイン側で破棄しない
- キャッシュしたアイコンは`LAUNCHER_FUNCTION_TABLE::HasIcon`で有効性を確認し、無効な場合は再取得する

### 例

directoryが "c:/path/to/directory" で、ヒットしたファイルのフルパスが "c:/path/to/directory/sub/file.txt"、"search-trigger-word"が "ws"の場合

名前は "ws sub/file.txt"、説明は "c:/path/to/directory/sub/file.txt"とする。

なお、ここではパス区切り文字をスラッシュとしているが、実際はWindowsの実際のパス区切り文字となる


