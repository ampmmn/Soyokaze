# commands

コマンド種別ごとの処理を置くためのディレクトリ。  
種別ごとにサブディレクトリを作成している。

## ディレクトリの一覧

|ディレクトリ名|説明|
|-|----|
|[activate_window](./activate_window)|ウインドウ切り替え系のコマンド<br>Excelのシート切り替えもここに含む|
|[align_window](./align_window)|ウインドウ整列コマンド|
|[bookmarks](./bookmarks)|ブラウザのブックマーク・履歴コマンド|
|[builtin](./builtin)|システムコマンド一式|
|[calculator](./calculator)|簡易電卓機能|
|[color](./color)|色を表示する機能|
|[common](./common)|commands以下の共通ソース|
|[core](./core)|コマンド回りのデータ管理|
|[controlpanel](./controlpanel)|コントロールパネル項目のコマンド|
|[decodestring](./decodestring)|文字列をデコードする系コマンド(DecodeURIなど)|
|[ejectvolume](./ejectvolume)|ボリュームの取り外しを行うコマンド|
|[env](./env)|環境変数を表示するコマンド|
|[everything](./everything)|everything検索コマンド|
|[filter](./filter)|フィルタコマンド|
|[getip](./getip)|IPアドレスを取得する|
|[group](./group)|グループコマンド|
|[history](./history)|実行履歴コマンド<br>(ブラウザ履歴とは別。このアプリ内のコマンド実行履歴。)|
|[mailto](./mailto)|メール送信コマンド|
|[mmc](./mmc)|MMCスナップインコマンド|
|[outlook](./outlook)|OutlookのInboxにあるメールを候補として表示するコマンド|
|[pathconvert](./pathconvert)|パス変換系コマンド<br>(file:///...や/c/path/to/file... とローカルパスの相互変換)|
|[pathfind](./pathfind)|アプリ内の`ファイル名を指定して実行`コマンド。<br>環境変数PATH以下にある`exe`ファイル、またはURLをランチャーから実行するコマンド|
|[presentation](./presentation)|PowerPointプレゼンテーション上のスライドタイトルでジャンプするコマンド|
|[regexp](./regexp)|正規表現コマンド|
|[shellexecute](./shellexecute)|通常コマンド|
|[simple_dict](./simple_dict)|簡易辞書コマンド|
|[snippet](./snippet)|定型文コマンド|
|[snippetgroup](./snippetgroup)|定型文グループコマンド|
|[specialfolderfiles](./specialfolderfiles)|特殊フォルダの項目を実行できるようにするためのコマンド<br>(最近使ったファイル、スタートメニューなど)|
|[timespan](./timespan)|経過時間を計算して表示するコマンド|
|[unitconvert](./unitconvert)|単位変換系コマンド|
|[uwp](./uwp)|UWPアプリを実行できるようにするためのコマンド|
|[vmware](./vmware)|VMWare(Player)の実行履歴からVMを実行できるようにするためのコマンド|
|[volumecontrol](./volumecontrol)|音量調節(音量変更/ミュート状態の変更)をするためのコマンド|
|[watchpath](./watchpath)|フォルダ更新検知コマンド|
|[websearch](./websearch)|Web検索コマンド|

