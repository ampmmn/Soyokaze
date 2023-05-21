# Soyokaze

bluewind風のランチャーソフト。

![画面](image/soyokaze-window.png)

細かいところは省いているものの、bluewindの主要な機能は実装している(つもり)

## bluewindとは

bluewindのReadMeから抜粋
```
初心者から上級者まで幅広く使えるコマンドラインランチャーです。
簡単なキーワードをファイルやURLに関連付け、
キーボードからそのキーワードを打ち込むことで
アプリケーションを起動させたり、Webサイトを開いたりできます。
```

一次配布元サイトが消滅(&更新停止)して久しい。

個人的にとても気に入っているツールで今でも普通に使えるけど、
今どきの環境だと動作があやしいところがある。

## 動作環境

下記環境での動作を確認済

- Windows 10(64bit)

## インストール手順

1. zipをファイルを展開して任意のフォルダに展開する
1. 展開先フォルダにある`soyokaze.exe`を実行する
1. 設定はユーザフォルダ(たいていは C:/Users/(ユーザ名))直下の`.soyokaze`フォルダに保存される  
初回実行時に`.soyokaze`フォルダを作成する
  * レジストリを一切変更しない

## アンインストール手順

アンインストーラはないため、以下すべてを手動で行う。

1. アプリケーションの設定-ショートカット設定を表示し、すべてのショートカットを削除する
1. `soyokaze.exe`を終了する
1. インストール時に`soyokaze.exe`を置いたフォルダごと削除する
1. 設定フォルダ(C:/Users/(ユーザ名)/.soyokaze)を削除する

## ファイル構成

- soyokaze.exe
- help.html
- LICENSE

## 主な特徴

- bluewindでいうところの常駐モードのみ
  - ホットキーでウインドウの呼び出しができる。初期値はAlt-Space
- bluewindとは異なり、表示方法を選べない(bluewindでいうところの「通常の表示」のみ)

- レジストリをいじらない
- ファイル名やURLを直接指定しての実行が可能
- グループ分け/連続起動/複数回起動 などはできない
- パラメータにキーワードを使用することができる

## 最初から入っているキーワード

- new
  - 新規コマンド登録画面を表示する
- edit
  - 既存コマンド編集画面を表示する
- manager
  - キーワードマネージャ画面を表示する
- setting
  - アプリケーション設定画面を表示する
- exit
  - `soyokaze`を終了
- registwin
  - 直前にアクティブなウインドウをコマンドとして登録する
- maindir
  - `soyokaze.exe`のあるフォルダを表示する
- userdir
  - 設定ファイルの保存先フォルダ(C:/Users/(ユーザ名)/.soyokaze)を表示する
- reload
  - 設定ファイルの再読み込みを行う  
(テキストエディタで直接キーワード編集を行ったときにリロードするためもの)
- version
  - バージョン情報ダイアログを表示する

## 差異

### オリジナルにないもの

- コマンド名に紐づけて実行するファイルを引数あり/なしで、実行する内容をわけることができる
  - 想定する使い方としては、引数なし→トップページ表示  引数あり→検索実行、みたいな

- コマンド設定時のパラメータとして「管理者として実行」を指定することができる

- 64bitモジュール

### オリジナルと比べてできないこと

個人的に不要な機能はもろもろカットしている

- 入力や補完まわりの細かなカスタマイズ
- タイマー実行
- コマンドのグループ化
- アイコンの変更(任意のアイコンの設定)
- コマンドごとのホットキー設定
- コマンドから別コマンドを参照して実行する
- コマンドの実行完了を待機する

他にもあると思うが割愛

## ライセンス

[MIT License](./LICENSE)


## 画面など

画面のつくりはbluewindにある程度似せて作っている

### 入力画面

![](image/soyokaze-window.png)

### キーワードマネージャ

![](image/keywordmanager.png)

### コマンド登録・編集画面

![](image/edit.png)

### 設定画面

![](image/setting.png)

![](image/shortcut_setting.png)

## ビルド

### 開発環境

- VisualStudio2019
- C++のプロジェクト
- MFCを使っている

今のところ(本体機能に関して)他のライブラリを用いていないので、単体でビルドできるはず

ユニットテストフレームワークとしてgoogletest(1.13.0)を用いている。

### ビルド方法

- VisualStudioでソリューションファイル`Soyokaze.sln`を開く
  - メニューの `ファイル`→`開く`→`プロジェクト/ソリューション`

- プロジェクト設定の下記項目を変更する必要があるのでビルド環境にあわせて設定する
  - `構成プロパティ`→`全般`→`Windows SDKバージョン`
  - `構成プロパティ`→`全般`→`プラットフォームツールセット`

- ビルドを実行する
  - メニューの `ビルド`→`ソリューションのビルド`

- ビルドが完了すると下記の場所にモジュールがてきる
  - `x64`/(`Debug` or `Release` or `ReleaseStatic`)
    - 64bit版モジュールの場合

### ソリューション構成について

- Debug → デバッグ情報あり
- Release → デバッグ情報なし、最適化してる、共有DLLでMFCを使う
- ReleaseStatic → デバッグ情報なし、最適化してる、スタティックライブラリでMFCを使う
- UnitTest → ユニットテストのビルド用
  - 本体のexe(soyokaze.exe)を.libとしてビルドし、ユニットテスト側の.exeにリンクする構成

## ToDo

- マニュアル拡充
- 設計資料を残す
- ユニットテスト拡充

## ChangeLog

### 0.1.1

2023/05/21

- 入力画面のアイコンからウインドウを指定してコマンドとして登録する機能を追加
- キーワードマッチング処理回りの改善
  - 完全一致、前方一致、部分一致、スキップマッチング、の順で候補を表示するようにした
- 設定値を正しく保存できていないバグを修正

### 0.1.0

2023/05/17

- アプリ名を`Soyokaze`に変更
- バージョン情報ダイアログにライセンスへのハイパーリンクとビルド日時を表示するようにした

### 0.0.10

2023/05/16

- 入力画面のウインドウにファイルやURLをドロップしてコマンド登録できる機能を実装
- アプリ設定画面の構成をbluewindのような形に変更(左側にツリー、右に設定欄)

### 0.0.9

2023/05/11

- アクティブウインドウ登録コマンド(`registwin`)を実装
- コンテキストメニューの「表示」を「隠す」とトグルにするようにした
- 入力画面からコンテキストメニューを表示できるようにした

### 0.0.8

2023/05/09

- ショートカット登録機能を実装
  - 送る/スタートメニュー/デスクトップ/スタートアップ
- ウインドウのトグル表示機能を実装
- 細かいバグ修正

### 0.0.7

2023/05/07

- ホットキーの登録まわりのバグ修正

### 0.0.6

2023/05/07

- ソースの文字エンコーディングをUTF-8にした
- 細かいバグ修正


### 0.0.5

2023/05/07

タスクトレイメニューからのヘルプ表示が機能してなかったので修正

### 0.0.4

2023/05/07

いろいろ機能追加

### 0.0.3

2023/05/04

いろいろ機能追加

* 大文字小文字を無視した比較
* 候補欄をつける
* マッチングの際に履歴を考慮する
* 自前のキーバインド(ホットキー)処理
* カレントディレクトリの設定
* パラメータ指定実行のサポート
  * あり/なしで分岐させることができる
* アプリ共通設定を持たせる
* 環境変数展開
* フォルダを開く
* アイコン表示できるようにした

### 0.0.2

2023/04/??

* コマンド実行(プロセス起動)時の表示方法(`SW_SHOW`など)

### 0.0.1

2023/04/23

* とりあえずつくってみた


