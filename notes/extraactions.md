# 追加アクション機能まわりのメモ

ここではファイル種別`ファイル`や`フォルダ`に対する追加アクションを実行する機能回りのメモを記載しておく

- この機能を実装したのは`0.49.0`
- 機能を実現するために既存インタフェースクラスの仕様変更を含む、わりと大きめの修正を入れた
- 変更量がわりと多く、あとで絶対忘れるのでここにメモっておく

## 変更点の概要

- 既存I/Fクラスの変更および実装クラス側での追従
- 追加アクションを扱うための新規インタフェースの追加
- `explorepath`以下のコマンドに対して新規インタフェースを実装
- 現在選択中の候補が持つ追加アクションをホットキー登録する処理の追加
- ガイド欄に追加アクションを表示するための処理の追加
- アクション設定のための設定ページ追加

## 既存I/Fクラスの変更および実装クラス側での追従

CommandインタフェースのGetActionメソッドの第一引数の型を変更した。  
従来はuint32_t型で修飾キーのフラグを指定する(修飾キー+`Enter`で実行したときのアクションオブジェクトを返すという)意味としていた。  
これをHOTKEY_ATTRオブジェクト(アクションを実行するために必要なキー入力操作)を渡す、という形に変更した。

```c++
virtual bool GetAction(uint32_t modifiers, Action** action) = 0;
↓
virtual bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) = 0;
```

これにより、Enterキー押下に限らず、任意のキー入力(ただし、1ストロークのみ)を扱えるようにした。

あわせて、Commandインタフェースを実装しているすべてのコマンドクラスに対して、
引数の型変更を追従するための修正を行った。

## 追加アクションを扱うための新規インタフェースの追加

追加アクションを扱うための `ExtraActionHotKeySettings` インタフェースを追加した。  
(commands/core/ExtraActionHotKeySettings.h)

コマンドクラスがこのインタフェースを実装すると、追加のアクションを実行するためのホットキー設定を取得するメソッドを実装できる。

```
	// ホットキー設定の数を取得
	virtual int GetHotKeyCount() = 0;
	// ホットキー設定を取得
	virtual bool GetHotKeyAttribute(int index, HOTKEY_ATTR& hotkeyAttr) = 0;
```

`GetHotKeyCount`で追加アクションの数、`GetHotKeyAttribute`で追加アクションに紐づけられたキー入力を返す。

コマンドは、このインタフェースを実装したうえで、QueryInterfaceメソッドでIID_EXTRAACTIONHOTKEYSETTINGSに対して、
このインタフェースを返すよう実装する。

なお、アプリバージョン`0.49.0`時点では、このインタフェースを実装しているのは、`explorepath`コマンドのみ

## `explorepath`以下のコマンドに対して新規インタフェースを実装

前セクションで説明したインタフェースクラスを`explorepath`コマンドに実装した。

このコマンドが返す「追加のアクション」は、アプリケーション設定で管理することにした。  
(アプリケーション設定の`実行`>`ファイルとフォルダ`という画面を新設し、ここで追加アクションを登録することにした)

登録した情報はアプリケーション設定として保存される。関連する設定のキーについては後述する。


設定ページで設定した内容を保存し、コマンド側で設定情報を取得できるようにするために、パラメータの読み書きクラスを追加した。  
この設定情報読み書きクラスは `ExplorePathExtraActionSettings` 

```plantuml
hide circle

class ExtraActionHotKeySettings
class Command

class ExplorePathCommand

class ExplorePathExtraActionSettings

class AppSettingExtraActionForExplorePath

ExplorePathCommand .up.|> Command
ExplorePathCommand .up.|> ExtraActionHotKeySettings

ExplorePathCommand ..> ExplorePathExtraActionSettings : 設定参照
AppSettingExtraActionForExplorePath ..> ExplorePathExtraActionSettings : 設定保存

```

## アクション設定のための設定ページ追加

前セクションのクラス図に記載した AppSettingExtraActionForExplorePath クラスがアプリケーション設定画面クラス。

設定画面で`追加` `編集` ボタンを押したときに、追加アクションの内容について設定する画面は ExtraActionDialog クラスで実装している。

```plantuml
hide circle

class ExplorePathExtraActionSettings
class AppSettingExtraActionForExplorePath
class ExtraActionDialog

AppSettingExtraActionForExplorePath .do.> ExtraActionDialog : 追加 or 編集
AppSettingExtraActionForExplorePath .up.> ExplorePathExtraActionSettings : 設定取得,保存

```


## 現在選択中の候補が持つ追加アクションをホットキー登録する処理の追加

現在選択中の候補が持つ追加アクションをホットキーの登録と解除をする責務を持つクラスとしてCommandActionHandlerRegistryクラスを新設した。  
(mainwindow/CommandActionHandlerRegistry)

候補欄で選択されたコマンドが追加アクションを持つ(=ExtraActionHotKeySettingsを実装している)場合、
そのコマンドからホットキー設定情報を取得し、CommandHotKeyManagerクラスに対してハンドラを登録する。  
また、候補欄での選択が解除されるタイミングでCommandHotKeyManagerクラスに対して登録したハンドラを解除する。

候補欄の選択イベントを受け取るために、CandidateListListenerIFインタフェースを実装している。  
また、CommandActionHandlerRegistryのインスタンス化はLauncherMainWindow内で行う  
(LauncherMainWindow周りは肥大化しているので整理・分離したいところ・・)

CommandHotKeyManagerに対してハンドラ登録するにあたっては、CommandHotKeyHandlerIF派生オブジェクトを作成して登録する必要があるが、既存のCommandHotKeyHandlerIF実装クラスに今回の用途に適したものがなかったため、新たに ExtraActionHotKeyHandler クラスを新設した。

(既存のNamedCommandHotKeyHandlerはハンドラ実行のタイミングで、CommandRepository経由でCommandオブジェクトの取得を行うが、CommandRepositoryからのCommand取得は登録型のコマンドしか取れない。今回の機能は`explorepath`コマンドで実行されるものであり、これは一時的なコマンドであるため、今回の用途でNamedCommandHotKeyHandlerを使うことはできなかった)

```plantuml
hide circle

class LauncherMainWindow
class CandidateList
class CandidateListListenerIF

class CommandActionHandlerRegistry
class CommandHotKeyHandlerIF
class ExtraActionHotKeyHandler

class CommandHotKeyManager

CommandActionHandlerRegistry .up.|>  CandidateListListenerIF

CandidateList ..> CandidateListListenerIF : 選択/選択解除を通知

LauncherMainWindow "1" o.. "1" CommandActionHandlerRegistry

CommandActionHandlerRegistry .ri.> CommandHotKeyManager : ハンドラの登録と解除

CommandActionHandlerRegistry ..> ExtraActionHotKeyHandler : ハンドラ登録時に生成

ExtraActionHotKeyHandler .up.|> CommandHotKeyHandlerIF

CommandHotKeyManager ..> CommandHotKeyHandlerIF : ハンドラを実行

```

## ガイド欄に追加アクションを表示するための処理の追加

ガイド欄に追加アクションについての情報を表示できるようにしたかったので、GuideCtrlクラスに追加アクションを取得する処理を追加した。


GuideCtrlでガイド欄の描画をする際、対象のコマンドがExtraActionHotKeySettings を実装している場合は、
キー割り当てとラベル(ガイド欄に表示するテキスト)の情報を取得し、それを使ってガイド欄の描画を行う

```plantuml
hide circle

class Command
class ExtraActionHotKeySettings
class Action

class GuideCtrl

GuideCtrl .up.> Command : 通常のアクションの情報を取得
GuideCtrl .up.> ExtraActionHotKeySettings : (ある場合は)追加のアクションの情報を取得
GuideCtrl .up.> Action : 実行されるアクション名を取得

```

## 追加アクション設定の保存先について

アプリケーション設定ファイル(`settings.ini`)に保存する

キー名は下記の通り

```
[ExplorePath]
NumberOfExtraActions=2    # 設定された追加アクションの数

# 1つ目の追加アクションの設定
Command1="stirling"       # コマンド名
HotkeyModifiers1=2        # キー入力の修飾キーのビットフラグ
HotkeyVKCode1=83          # キー入力の仮想キーコード
IsForFile1=true           # 追加アクションがファイルに対して有効か?
IsForFolder1=false        # 追加アクションがフォルダに対して有効か?
Label1="Stirlingで開く"   # ガイド欄に表示するテキスト

# 2つ目以降は Command2,HotkeyModifiers2,.... Label2のような形で末尾の数値が増えていく

```

