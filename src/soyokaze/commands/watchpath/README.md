# watchpath

フォルダの更新を検知するコマンド  
当初はフォルダもファイルも検知できればと思い、フォルダ名を`watchpath`としたが、途中でフォルダ監視のみを実装する方針に切り替えた


## クラス

```plantuml

hide circle

class CommandRepository
class WatchPathCommandProvider
class WatchPathCommandEditDialog
class WatchPathCommand
class WatchPathCommandParam
class PathWatcher <<singleton>>
class PathWatcherItem
class WatchTarget <<interface>>
class LocalPathTarget
class UNCPathTarget

WatchPathCommandProvider -le> CommandRepository : 生成したコマンドを登録
WatchPathCommandProvider -down-> WatchPathCommand : 生成
WatchPathCommand -> WatchPathCommandEditDialog : 設定ダイアログ表示
WatchPathCommand -do-> PathWatcher : パス更新監視を依頼する
WatchPathCommand "1" o.. "1" WatchPathCommandParam
WatchPathCommandEditDialog ..> WatchPathCommandParam : 編集

WatchPathCommand ..> PathWatcherItem : つくる
PathWatcher .ri.> PathWatcherItem : 参照

PathWatcher "1" o.. "0..*" WatchTarget : 利用

LocalPathTarget -up-|> WatchTarget : ローカルパス用
UNCPathTarget -up-|> WatchTarget : UNCパス用

note right of WatchTarget
フォルダ監視コマンドのぶんインスタンスが作られる
end note

```

### WatchPathCommandProvider

`WatchPathCommand`を `CommandRepository`に登録するためのプロバイダークラス

### WatchPathCommand

- 1つのコマンドにつき1つのパスを監視する

### WatchPathCommandEditDialog

コマンドの新規作成/編集ダイアログを実装したクラス

### PathWatcher

`WatchPathCommand`で監視するパスの実際の監視処理を行うクラス。  
シングルトン。パスをmapで管理する  
スレッドを内部で作成し、スレッド側で一定間隔で更新があったかどうかを調べて、更新があったら更新された旨を通知する。

### PathWatcherItem

WatchPathCommmandがPathWatcherに対して監視対象を登録するさいに、監視対象の諸情報を渡すための入れ物クラス

1つの監視対象につき1つのPathWatcherItemが対応している。  
役割的にはWatchPathCommandParamに統合してもいいかもしれない

### WatchTarget

監視対象ディレクトリを表すインタフェースクラス。  
監視対象ディレクトリの更新を確認したり、更新があった場合に更新内容の情報を呼び出し元に返す機能を持つ

### LocalPathTarget

ローカルパス(例`C:\Path\To\Dir`)に対する監視処理を実装したクラス。
`ReadDirectoryChangesW`を用いて実装している。

### UNCPathTarget

UNCパス(例:`\\server\Path\To\Dir`)に対する監視処理を実装したクラス。  
UNCパスに対しては`ReadDirectoryChangesW`が安定的に機能しなかったので、自前での監視処理を実装している。


## シーケンス

### フォルダの監視

```plantuml

title PathWatcherの更新監視処理

hide footbox
participant App

participant PathWatcher
participant "PathWatcher::PImpl" as in
participant thread

[-> PathWatcher ++ : RegisterPath(path)

	PathWatcher -> in ++ : Register(path)
	return

	PathWatcher -> in ++ : StartWatch()

alt 初回のみ
		in -> thread ++ : run()
		in <-- thread
end
	PathWatcher <-- in --
[<-- PathWatcher --

note right of thread : バックグラウンドで更新を監視する
loop IsAbort() == false

thread  -> in ++ : GetTargetObjects()
return objects

thread -> thread ++ : WaitForMultipleObjects(objects)
thread <-- thread -- : [ret,index]

alt ret == 変更あり
	thread -> thread ++ : NotifyTarget(index)
		thread -> App ++ : PopupMessage("更新あり")
		return
	thread <-- thread --
end

end

```


## メモ

- ローカルパスの監視をするために`ReadDirectoryChangesW`APIを利用している
- 一方、ネットワークパス(UNCパス)の監視は自前で行っている

- `ReadDirectoryChangesW`APIはローカルパスの更新検知については確実にやってくれてそうだけど、ネットワークパスの更新検知はあてにならない感じだったため

- 自前の監視はフォルダ内のファイル一覧を列挙して最終更新日時を取得し、前回の内容と比較する、というもの
  - チェックのたびに毎回ファイル一覧の走査が発生している
  - ~~負荷対策のため、UNCパスのチェックについては1分に1回に限定している。~~ → コマンド設定で間隔を設定できるようにした

