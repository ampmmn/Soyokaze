# pathfind

アプリ内の`ファイル名を指定して実行`機能を実装する。

## クラス構成

```plantuml

class AppPreference
class ExecuteHistory

namespace common {

class CandidateExclusionList

}

namespace pathfind {

class PathExeAdhocCommandProvider
class PathExecuteCommand
class LocalPathResolver

PathExeAdhocCommandProvider o.. PathExecuteCommand : 生成
PathExeAdhocCommandProvider "1" o.. "1" common.CandidateExclusionList : 生成


}

pathfind.PathExecuteCommand ..> AppPreference : 設定読み込み
pathfind.PathExecuteCommand ..> LocalPathResolver : パス解決

common.CandidateExclusionList ..> AppPrerefence : 設定読み込み
pathfind.PathExeAdhocCommandProvider ..> AppPreference : 設定読み込み

pathfind.PathExeAdhocCommandProvider ..> ExecuteHistory : 履歴登録/参照


```

## クラス

### PathExeAdhocCommandProvider

`PathExecuteCommand`を生成するためのクラス

- コンストラクタで`PathExecuteCommand`を生成しておき、常に持ち続ける
  - 入力キーワードに合致する要素が存在するかどうかの判定は`PathExecuteCommand`側で判断している
- 一時コマンド問い合わせメソッド`QueryAdhocCommands`の初回呼び出し時に設定を読む
- 入力キーワードが履歴に合致するかどうかは`PathExeAdhocCommandProvider`内で行っている

- パス検索結果が除外対象に含まれるかどうかを`CandidateExclusionList`に問い合わせる


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

### CandidateExclusionList

除外するファイルパスと、候補の表示名に照合するパターンを保持するリスト。

アプリ設定画面の`実行>除外する項目`で設定したパスと表示名パターンを`AppPreference`から取得し、リストとして保持する。

表示名パターンは大文字・小文字を区別せず部分一致で照合する。正規表現として無効な設定値はログを出力して読み飛ばす。


