# hotkey

ホットキー回りの処理をまとめたディレクトリ


## クラス図

```plantuml

class CommandHotKeyManager <<singleton>>

class "CommandHotKeyManager::PImpl::ITEM" as ITEM
CommandHotKeyManager o.. ITEM

class CommandHotKeyMappings


CommandHotKeyManager ..> CommandHotKeyMappings


```

## クラス説明

### AppHotKey

メイン画面（入力画面)を呼び出すためのホットキーの制御回りの処理を行う
設定ファイルからホットキー情報を取得して、グローバルホットキーとして登録する

```plantuml

class AppPreferenceListenerIF

class AppHotKey

class GlobalHotKey

class AppPreference

AppHotKey .up.|> AppPreferenceListenerIF : 設定変更通知を受けてホットキーの再登録を行うため

AppHotKey ..> AppPreference : 設定読み込み
AppHotKey ..> GlobalHotKey : ホットキー登録

```

### CommandHotKeyManager

キーとコマンド名の関連付けを管理するクラス。シングルトン。

`Register`メソッドでハンドラとキーと種別(グローバルorアプリ内)を登録する

`GetItem`メソッドで登録された要素を取得する

`GetMappings`メソッドで管理するマッピング情報の一覧を取得できる  
(CommandHotKeyManagerに結果を格納する)


### CommandHotKeyManager::PImpl::ITEM

内部クラス

ホットキーが押されたときに呼び出すハンドラを保持する


### GlobalHotKey

グローバルホットキーを扱うためのクラス  
Win32APIの`RegisterHotKey`を使ってホットキー登録、`UnregisterHotKey`を使ってホットキー解除などを行う

```plantuml

class GlobalHotKey
class HOTKEY_ATTR


GlobalHotKey ..> HOTKEY_ATTR : ToStringを利用するため

```

### NamedCommandHotKeyHandler


ホットキーが押されたときにコマンドを実行する、を実現するためのハンドラクラス。

実行するコマンドの名前をコンストラクタの引数で保持し、
`Invoke`（ホットキーが押されたときに実行されるメソッド)で、登録されたコマンド名を実行する。  
コマンドの実行は`CommandRepository`から完全一致比較で`Command`オブジェクトを取得し、`Execute()`を呼ぶ。
a

```plantuml

class CommandHotKeyHandler

class NamedCommandHotKeyHandler

class CommandRepository
class Command

NamedCommandHotKeyHandler .up.|> CommandHotKeyHandler

NamedCommandHotKeyHandler ..> CommandRepository : コマンド取得
NamedCommandHotKeyHandler ..> Command : 実行


```


## ローカルホットキー周りの処理

```plantuml

class LauncherMainWindow

class CommandHotKeyManager <<singleton>>

LauncherMainWindow -> CommandHotKeyManager

```

### シーケンス(初期化～コールバック呼び出し)

``` plantuml

autonumber

participant LauncherMainWindow
participant CommandHotKeyManager
participant CommandHotKeyHandler

note across : 初期化
-> LauncherMainWindow ++ : OnInitDialog

    ' インスタンス取得
    LauncherMainWindow -> CommandHotKeyManager ++: GetInstance()
    return

    ' ウインドウ登録 
    LauncherMainWindow -> CommandHotKeyManager ++: SetRegisterWindow(hwnd)
    return

<-- LauncherMainWindow --

note across : 通知

-> LauncherMainWindow ++ : PreTranslateMessage

    ' インスタンス取得
    LauncherMainWindow -> CommandHotKeyManager ++: GetInstance()
    return

    ' アクセラレータ取得
    LauncherMainWindow -> CommandHotKeyManager ++: GetAccelelator()
    return accel

    ' アクセラレータにメッセージを渡す
    LauncherMainWindow -> LauncherMainWindow ++: TranslateAccelerator(hwnd, accel)
        note over LauncherMainWindow : 該当するキー入力メッセージだったら、LauncherMainWindowのハンドラが呼ばれる
    return

<-- LauncherMainWindow --

note across : コールバック通知

-> LauncherMainWindow ++ : OnCommandHotKey(id)

    ' インスタンス取得
    LauncherMainWindow -> CommandHotKeyManager ++: GetInstance()
    return
   
    ' ローカルハンドラを呼び出す 
    LauncherMainWindow -> CommandHotKeyManager ++: InvokeLocalHandler(id)
        CommandHotKeyManager -> CommandHotKeyManager ++ : find(id)
        return handler

        CommandHotKeyManager -> CommandHotKeyHandler ++ : Invoke()
    return

<-- LauncherMainWindow --

```

### シーケンス(ハンドラ登録)

```plantuml

autonumber

participant CommandRepository
participant AppPreference
participant CommandHotKeyManager
participant NamedCommandHotKeyHandler

note over CommandRepository : コマンド登録データ更新時に呼ばれる
-> CommandRepository ++ : ReloadPatternObject

    CommandRepository -> AppPreference ++ : Get()
    return

    CommandRepository -> AppPreference ++ : GetCommandKeyMappings()
    return hotKeyMap

    loop foreach map in hotKeyMap

        CommandRepository -> CommandHotKeyManager ++ : GetInstance()
        return

        CommandRepository -> NamedCommandHotKeyHandler ** : 作成
        CommandRepository <-- NamedCommandHotKeyHandler : handler
        
        CommandRepository -> CommandHotKeyManager ++ : Register(handler)
        return
    end
    

<-- CommandRepository --

```




### 説明

- LauncherMainWindowはメッセージハンドラのテーブルで以下を登録しているので、
CommandHotKeyManagerが生成したアクセラレータのコールバックをうけとることができる
  - ID範囲はCommandHotKeyManagerが提供する

```
	ON_COMMAND_RANGE(core::CommandHotKeyManager::ID_LOCAL_START, 
	                 core::CommandHotKeyManager::ID_LOCAL_END, OnCommandHotKey)
```




