# mainwindow

ランチャーのメインウインドウ周りの処理を置くためのディレクトリ。  

## クラス一覧

|クラス名|説明|
|-|----|
|[AppSound](./AppSound.h)|メインウインドウ操作に伴う効果音を再生する|
|[CandidateList](./CandidateList.h)|候補一覧を保持するクラス|
|[CandidateListCtrl](./CandidateListCtrl.h)|候補一覧を表示するリストコントロールクラス|
|[CandidateListListenerIF](./CandidateListListenerIF.h)|候補選択回りのイベントを受け取るためのリスナーインタフェース|
|[CmdReceiveEdit](./CmdReceiveEdit.h)|後続プロセスからのコマンドライン指定で渡されたコマンド文字列を受け取るための内部ウインドウ(不可視)|
|[LauncherDropTarget](./LauncherDropTarget.h)|メインウインドウへのドラッグアンドドロップ処理関係のクラス|
|[LauncherInput](./LauncherInputStatusIF.h)|キーワード入力状態を得るためのインタフェースクラス|
|[LauncherMainWindow](./LauncherMainWindow.h)|メインウインドウクラス|
|[LauncherMainWindowIF](./LauncherMainWindowIF.h)|ランチャーウインドウ上の部品を取得するためのインタフェースクラス|
|[LauncherWindowEventDispatcher](./LauncherWindowEventDispatcher.h)|メインウインドウ関係のイベントをリスナーに通知するクラス|
|[LauncherWindowEventListenerIF](./LauncherWindowEventListenerIF.h)|メインウインドウ関連のイベントリスナーのインタフェース|
|[MainWindowAppearance](.MainWindowAppearance.h/)|メインウインドウの外観(色、フォント)を制御するクラス|
|[MainWindowDeactivateBlocker](./MainWindowDeactivateBlocker.h)|メインウインドウが(アプリ設定により)フォーカスを失ったときに非表示になることを一時的に阻害するための部品クラス|
|[MainWindowHotKey](./MainWindowHotKey.h)|メインウインドウ上での操作に対するホットキー処理を扱うクラス|
|[OperationWatcher](./OperationWatcher.h)|メインウインドウのイベントに応じた内部動作を実装している。<br>LauncherWindowEventListenerIFのリスナーとして動作する|
|[WarnWorkTimeToast](./WarnWorkTimeToast.h)|長時間稼働時にそれを通知するトーストを実装したクラス|
|[WindowAppearnce](./WindowAppearanceIF.h)|ウインドウの外観を制御するためのインタフェースクラス|
|[WindowTransparency](./WindowTransparency.h)|ウインドウの半透明処理回りを実装しているクラス|


## クラス図

```plantuml

class WindowAppearnce
class MainWindowAppearance

MainWindowAppearance -up-|> WindowAppearnce

class LauncherMainWindowIF
class LauncherMainWindow

LauncherMainWindow -up-|> LauncherMainWindowIF

LauncherMainWindow o- WindowAppearnce

MainWindowAppearance ..> LauncherMainWindowIF

```

