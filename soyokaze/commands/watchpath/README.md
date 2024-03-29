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
class PathWatcher <<singleton>>

WatchPathCommandProvider -le> CommandRepository : 生成したコマンドを登録
WatchPathCommandProvider -down-> WatchPathCommand : 生成
WatchPathCommand -> WatchPathCommandEditDialog : 設定ダイアログ表示
WatchPathCommand -do-> PathWatcher : パス更新監視を依頼する
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
  - 負荷対策のため、UNCパスのチェックについては1分に1回に限定している。

