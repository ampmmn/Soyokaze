# pathfind

アプリ内の`ファイル名を指定して実行`機能を実装する。

## クラス構成

```plantuml

class AppPrerefence
class ExecuteHistory

namespace pathfind {

class PathExeAdhocCommandProvider
class PathExecuteCommand
class ExcludePathList
class LocalPathResolver

PathExeAdhocCommandProvider o.. PathExecuteCommand : 生成
PathExeAdhocCommandProvider "1" o.. "1" ExcludePathList : 生成

PathExecuteCommand ..> ExcludePathList : 参照

}

pathfind.PathExecuteCommand ..> AppPrerefence : 設定読み込み
pathfind.PathExecuteCommand ..> LocalPathResolver : パス解決

pathfind.ExcludePathList ..> AppPrerefence : 設定読み込み
pathfind.PathExeAdhocCommandProvider ..> AppPrerefence : 設定読み込み

pathfind.PathExeAdhocCommandProvider ..> ExecuteHistory : 履歴登録/参照


```

## クラス

### PathExeAdhocCommandProvider

`PathExecuteCommand`を生成するためのクラス

- コンストラクタで`PathExecuteCommand`を生成しておき、常に持ち続ける
  - 入力キーワードに合致する要素が存在するかどうかの判定は`PathExecuteCommand`側で判断している
- 一時コマンド問い合わせメソッド`QueryAdhocCommands`の初回呼び出し時に設定を読む
- 入力キーワードが履歴に合致するかどうかは`PathExeAdhocCommandProvider`内で行っている

- 過去に追加された履歴情報が、除外対象に含まれるかどうかを`ExcludePathList`に問い合わせる


```plantuml

class CommandProvider
class AdhocCommandProviderBase
class PathExeAdhocCommandProvider

PathExeAdhocCommandProvider -up-|> AdhocCommandProviderBase
AdhocCommandProviderBase -up-|> CommandProvider

```

### PathExecuteCommand

アプリ内の`ファイル名を指定して実行`機能を実装したクラス。

- `PathExecuteCommand::Match`メソッドの中で、入力キーワードに合致する実行やURLがあるかを探す
- 合致するものがあれば、それを候補として返す

- 入力キーワードに合致するexeファイルを探す処理は`LocalPathResolver`を利用する

#### Match内の処理

1. まず入力キーワードがURLなら(http://, https://)、URLとみなす。このときはWholeMatchあつかい
2. 入力キーワードが絶対パス表記かつ存在するパスであるなら、このときはWholeMatchあつかい
3. 1と2に該当しない場合は、`LocalPathResolver`を使ってパス解決を試みる。  
パス解決できたら、得られた絶対パスを候補とする。このときもWholeMatchあつかいにする。

1/2/3いずれも該当しない場合はMismatchとする

### ExcludePathList

除外するexeファイルのパスを保持するリスト。  

アプリ設定画面の`実行>除外するファイル`で設定したパスを`AppReference`から取得し、リストとして保持する。


