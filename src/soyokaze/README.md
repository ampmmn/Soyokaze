# ソースフォルダ

ランチャーアプリ本体のソースファイルを置くためのディレクトリ

## サブディレクトリ構成

|サブディレクトリ名|説明|
|----|--------|
|[app](app)|アプリケーションクラスまわり|
|[commands](commands)|コマンドごとの処理|
|[control](control)|複数の処理から参照されるGUI共通部品|
|[hotkey](hotkey)|ホットキー回りの処理|
|[icon](icon)|アイコンリソースのロード処理など|
|[logger](logger)|ロガー|
|[mainwindow](mainwindow)|入力欄ウインドウまわり|
|[matcher](matcher)|入力キーワードのマッチングを行うための処理|
|[res](res)|リソース置き場(.ico/.cur)など|
|[setting](setting)|設定情報の保存・読み込み|
|[settingwindow](settingwindow)|アプリ設定画面|
|[tasktray](tasktray)|タスクトレイ登録処理など|
|[utility](utility)|細かい部品|

## フォルダ直下に置いているもの

- framework.h
  - IDEでプロジェクト作成したときに自動生成されたファイル
- pch.cpp
  - プリコンパイル用
- pch.h
  - プリコンパイル用
- resource.h
  - リソースIDが記載されたヘッダファイル。IDEが生成する
- SharedHwnd.h
  - プロセスをまたいでメインウインドウのHWNDを取得するための部品
- Soyokaze.rc
  - リソーススクリプト
- soyokaze.vcxproj
  - プロジェクトファイル
- soyokaze.vcxproj.filters
  - フィルタ情報
- targetver.h
  - IDEでプロジェクト作成したときに自動生成されたファイル

