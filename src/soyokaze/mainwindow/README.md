# mainwindow

ランチャーのメインウインドウ、入力・候補表示、ウインドウ状態に関する処理を置くディレクトリ。

## 主なクラス

|クラス名|説明|
|---|---|
|[AppSound](./AppSound.h)|メインウインドウ操作に伴う効果音を再生する|
|[LauncherMainWindow](./LauncherMainWindow.h)|メインウインドウ。MFCイベントを受け取り、Stateと既存の入力・候補処理をつなぐ|
|[LauncherMainWindowIF](./LauncherMainWindowIF.h)|メインウインドウ上の部品を取得するインターフェース|
|[LauncherDropTarget](./LauncherDropTarget.h)|メインウインドウへのドラッグアンドドロップを扱う|
|[LauncherInput](./LauncherInputStatusIF.h)|キーワード入力状態を取得するインターフェース|
|[CandidateListListenerIF](./CandidateListListenerIF.h)|候補選択に関するイベントを受け取るリスナーインターフェース|
|[CmdReceiveEdit](./interprocess/CmdReceiveEdit.h)|後続プロセスからコマンド文字列を受け取る内部ウインドウ|
|[LauncherWindowState](./state/LauncherWindowState.h)|ウインドウ状態ごとのイベントを定義するインターフェース|
|[LauncherWindowStateBase](./state/LauncherWindowStateBase.h)|Stateの共通処理と既定イベント処理を提供する基底クラス|
|[LauncherWindowStateContextIF](./state/LauncherWindowStateContextIF.h)|Stateからウインドウ処理を利用するためのインターフェース|
|[HiddenState](./state/MainWindowHiddenState.h)|メインウインドウが非表示の状態|
|[IdleState](./state/MainWindowIdleState.h)|メインウインドウが表示され、入力がない状態|
|[SearchingState](./state/MainWindowSearchingState.h)|コマンド名の検索・候補操作を行う状態|
|[ParamSearchingState](./state/MainWindowParamSearchingState.h)|実行時引数に対する追加候補を表示する状態|
|[ExtraCandidateListCtrl](./ExtraCandidateListCtrl.h)|実行時引数用の候補ポップアップとリストを管理する|
|[MainWindowAppearance](./layout/MainWindowAppearance.h)|色、フォント、透明度などメインウインドウの外観を扱う|
|[MainWindowLayout](./layout/MainWindowLayout.h)|メインウインドウの配置とサイズを扱う|
|[WindowTransparency](./layout/WindowTransparency.h)|ウインドウの半透明表示を制御する|
|[MainWindowDeactivateBlocker](./MainWindowDeactivateBlocker.h)|一時的に非アクティブ時の非表示を抑制する|
|[CandidateList](./CandidateList.h)|通常の候補一覧を保持する|
|[CandidateListCtrl](./CandidateListCtrl.h)|通常の候補一覧を表示するリストコントロール|
|[LauncherWindowEventDispatcher](./LauncherWindowEventDispatcher.h)|メインウインドウ関連のイベントをリスナーへ通知する|
|[LauncherWindowEventListenerIF](./LauncherWindowEventListenerIF.h)|メインウインドウ関連イベントのリスナーインターフェース|
|[WindowAppearnce](./WindowAppearanceIF.h)|ウインドウ外観を制御するインターフェース|
|[WarnWorkTimeToast](./WarnWorkTimeToast.h)|長時間稼働時に警告を表示するトースト|

## ウインドウとStateの役割

`LauncherMainWindow`は、Windows/MFCメッセージの受け取り、UI部品の保持、および既存の検索・表示処理を担当する。現在のStateは`LauncherMainWindow::PImpl::mState`が`std::unique_ptr<LauncherWindowState>`で所有する。

Stateは、現在の入力・表示状態に応じた処理と状態遷移を担当する。`LauncherWindowStateContextIF`を通じてウインドウ操作、入力処理、候補検索などを利用し、`LauncherMainWindow`の内部メンバーやMFC処理へ直接依存しない。

```text
LauncherWindowState
        |
LauncherWindowStateBase
        |
  +-----+-----+------------------+
  |     |     |                  |
Hidden  Idle  Searching  ParamSearching
```

4つの具象Stateはいずれも`LauncherWindowStateBase`から直接派生する。

### Stateイベントの区別

ウインドウのアクティブ状態が実際に変わった通知と、ランチャーの表示・非表示を要求するイベントは別々に扱う。

|イベント|意味・呼び出し元|
|---|---|
|`OnActivate()`|メインウインドウがアクティブになった通知。`LauncherMainWindow::OnActivate()`から呼び出す|
|`OnDeactivate()`|メインウインドウが非アクティブになった通知。同じくMFCのアクティブ状態変更から呼び出す|
|`OnShowRequested(bool isShowForce)`|ホットキーなど外部からの表示・アクティブ化要求。強制表示とトグル表示もここで扱う|
|`OnHideRequested()`|メッセージやユーザー操作による非表示要求|

`LauncherMainWindow::OnActivate()`は`nState`に応じてStateの`OnActivate()`または`OnDeactivate()`を呼ぶ。非アクティブ時の非表示は、`IsHideOnInactive()`が有効で、かつ非アクティブ化がブロックされていない場合に限り、別途`OnHideRequested()`としてStateへ依頼する。

このため、非アクティブ化通知自体は必ずしもメインウインドウを隠さない。例えば`ParamSearchingState::OnDeactivate()`は追加候補ポップアップを閉じるが、Stateを維持する。非表示設定によってウインドウを隠す場合は`OnHideRequested()`により`HiddenState`へ遷移する。

`MainWindowAppearance`は色・フォント・透明度などの外観制御を担当する。非アクティブ時にウインドウを隠すかどうかの判断やState通知は行わない。非アクティブ化がブロックされている間は、ウインドウ側が非表示要求と透明度更新の双方を抑制する。

### 主なイベントの流れ

|契機|Stateイベント|
|---|---|
|外部からの表示メッセージ|`OnUserMessageActiveWindow()` → `OnShowRequested()`|
|非表示メッセージ|`OnUserMessageHide()` → `OnHideRequested()`|
|コンテキストメニュー操作後の非表示|`OnHideRequested()`|
|MFCのアクティブ状態変更|`LauncherMainWindow::OnActivate()` → `OnActivate()`または`OnDeactivate()`|
|非アクティブ時に非表示設定が有効|上記に加えて`OnHideRequested()`|
|入力変更・検索完了・候補操作|現在のStateに対応するイベントを呼び出す|

### LauncherWindowStateBase

`LauncherWindowStateBase`は`LauncherWindowStateContextIF`へのポインタを保持し、`GetContext()`を通じて派生Stateへ提供する。Stateごとの差分がないイベントには既定処理を用意し、既定のキー入力イベントは`false`を返す。コンストラクタは`protected`で、具象Stateを通じて使用する。

## Stateの遷移と主な処理

```text
HiddenState --表示要求・キーワードなし--> IdleState
HiddenState --表示要求・キーワードあり--> SearchingState
IdleState --キーワード入力-------------> SearchingState
SearchingState --引数候補検索開始------> ParamSearchingState
ParamSearchingState --キャンセル等-----> SearchingState
IdleState / SearchingState / ParamSearchingState
              --非表示要求-------------> HiddenState
```

|現在のState|イベント・条件|遷移先・処理|
|---|---|---|
|`HiddenState`|`OnShowRequested()`、キーワードあり|表示し、`SearchingState`へ遷移|
|`HiddenState`|`OnShowRequested()`、キーワードなし|表示し、`IdleState`へ遷移|
|`IdleState`|`OnTextChanged()`、キーワードあり|`SearchingState`へ遷移|
|`IdleState`|`OnCancel()`、キーワードなし|ウインドウを隠し、`HiddenState`へ遷移|
|`SearchingState`|`OnTextChanged()`、キーワードなし|`IdleState`へ遷移|
|`SearchingState`|検索完了後、引数候補検索が可能|要求をメッセージキューへ登録し、`ParamSearchingState`へ遷移|
|`ParamSearchingState`|キャンセル、対象外入力、位置・サイズ変更|`SearchingState`へ遷移|
|`ParamSearchingState`|追加候補を確定|確定方法に応じて通常検索または実行へ戻る|
|`IdleState` / `SearchingState` / `ParamSearchingState`|`OnHideRequested()`|`HiddenState`へ遷移|

`OnActivate()`／`OnDeactivate()`というアクティブ状態通知だけでは、通常Stateの遷移や表示要求処理は行わない。追加候補を表示中の`ParamSearchingState`は、非アクティブ通知を受けたときにポップアップを閉じ、State自体は維持する。非表示要求によるState遷移時にも`OnExit()`がポップアップを閉じる。

Stateの変更は`LauncherMainWindow::ChangeState()`を通じて行う。遷移時には、現在のStateの`OnExit()`、所有Stateの入れ替え、新しいStateの`OnEnter()`の順に処理する。検索完了時には検索結果をContextへ反映してから、表示状態や入力内容に応じてStateを継続または変更する。

## 実行時引数の追加候補ポップアップ

通常の候補欄はコマンド名を検索し、追加候補ポップアップは選択中コマンドの実行時引数を検索する。追加候補は`ParamSearchingState`が表示中の状態と操作を管理し、`ExtraCandidateListCtrl`がポップアップとリスト表示を担当する。

### ポップアップの構成

`ExtraCandidateListCtrl`は候補コマンド、現在の選択、表示・非表示、上下移動を管理する。内部の`ExtraCandidatePopupWnd`は`WS_POPUP`および`WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE`で作成される。ポップアップ表示中もメインウインドウの入力フォーカスを維持する。

子リストからの`WM_NOTIFY`はポップアップウインドウを経由して`LauncherMainWindow`へ転送され、現在のStateへ委譲される。メインウインドウは`ExtraCandidateListCtrl`と候補コマンドを保持し、候補が不要になると`HideExtraCandidates()`でポップアップを隠して保持候補を破棄する。

### 追加候補検索を開始する条件

`LauncherMainWindow::CanStartParamSearching()`は次の条件を確認する。

1. 入力欄に選択範囲がない。
2. コマンド名の後ろに検索対象となる引数トークンがある。
3. 引数トークンが空ではない。
4. 引数トークンが引用符で始まっていない。
5. 現在選択中のコマンドが存在し、`IsAcceptArguments()`が`true`を返す。

通常のトークン範囲に加え、ドライブレター形式（`C:\\path`、`C:/path`）またはUNC形式（`\\\\server\\share`）の絶対パスを含む引数範囲を認識する。コマンド後方に絶対パスがある場合、空白を区切りとして開始位置を探し、連続する空白を読み飛ばす。絶対パスやディレクトリであることだけでは追加候補検索を許可せず、選択中コマンドの`IsAcceptArguments()`を最終条件とする。

### 候補の検索と表示

`SearchingState`は通常候補検索完了後に検索開始可否を確認する。開始可能なら`RequestParamSearching()`でメッセージキューへ要求を登録してから`ParamSearchingState`へ遷移する。通常候補の選択変更通知を先に処理するため、検索完了通知から直接Stateを切り替えない。

`ParamSearchingState::OnEnter()`および入力変更時に`UpdateExtraCandidates()`が次の処理を行う。

1. キャレット位置と入力文字列から検索対象の引数トークンを取得する。
2. トークンを検索語として`CommandQueryRequest`を作り、`CommandRepository::Query()`を実行する。
3. 検索完了を最大2秒待つ。
4. 検索結果から`CanResolve()`が`true`のコマンドだけを候補にする。
5. `ExtraCandidateListCtrl`へ候補を設定し、候補があればポップアップを表示する。

検索に失敗した場合、タイムアウトした場合、検索語が空または引用符で始まる場合、候補がない場合はポップアップを閉じる。候補はコマンド名を1列で表示し、`Resolve()`は候補確定時まで呼び出さない。`IsAcceptArguments()`は引数検索を許可するか、`CanResolve()`は検索結果を候補にできるかを表し、異なる判定として扱う。

### 配置と外観

ポップアップは入力欄のキャレット位置を基準に配置する。X座標はキャレットの画面座標、Y座標は入力欄の下側である。幅は320ピクセル、最大16行で、候補数と行高に応じて高さを決める。メインウインドウと同じフォントを使い、通常候補欄に合わせた文字色、背景色、交互行色、選択色で描画する。アイコンやコマンド種別、ヘッダー、背景画像は表示しない。

### 操作と確定

`ParamSearchingState::OnKeyInput()`は次の操作を扱う。

|キー・操作|処理|
|---|---|
|`↑` / `↓`|追加候補の選択を移動する。選択は候補間を循環する|
|`Tab`|選択候補を解決し、入力中のトークンを置換して`SearchingState`へ戻る。通常検索後に追加候補を再表示できる|
|`Enter`|選択候補を解決して通常の候補検索へ戻る。追加候補を再表示しない|
|候補クリック|選択中の追加候補を確定する|

候補の確定では、`Resolve()`で得た値で対象トークン部分だけを置換し、後続の入力文字列は維持する。展開値の直後へキャレットを移動して通常の候補検索を再実行する。

## 保守時の確認事項

- Stateイベントの変更時は、ウインドウの実アクティブ状態通知と表示・非表示要求を混同しない。
- 非アクティブ時の非表示設定および`MainWindowDeactivateBlocker`による抑制を維持する。
- 非アクティブ通知時の`ParamSearchingState`はポップアップを閉じるが、通知だけでは`HiddenState`へ遷移しない。
- 追加候補検索の開始判定、引数範囲取得、`IsAcceptArguments()`と`CanResolve()`の役割を別々に保つ。
- `Tab`確定後の再検索・再表示と、`Enter`確定後の通常検索への復帰を区別する。
- キャレットや選択範囲の変更、ウインドウ移動・リサイズ時のState遷移を確認する。
- Stateごとの動作変更には`tests/testcode/soyokaze/mainwindow/state/LauncherWindowStateTest.cpp`のテストを追加する。

## 関連ファイル

- `src/soyokaze/mainwindow/LauncherMainWindow.h/.cpp`
- `src/soyokaze/mainwindow/ExtraCandidateListCtrl.h/.cpp`
- `src/soyokaze/mainwindow/state/LauncherWindowState.h`
- `src/soyokaze/mainwindow/state/LauncherWindowStateBase.h/.cpp`
- `src/soyokaze/mainwindow/state/LauncherWindowStateContextIF.h`
- `src/soyokaze/mainwindow/state/MainWindowHiddenState.h/.cpp`
- `src/soyokaze/mainwindow/state/MainWindowIdleState.h/.cpp`
- `src/soyokaze/mainwindow/state/MainWindowSearchingState.h/.cpp`
- `src/soyokaze/mainwindow/state/MainWindowParamSearchingState.h/.cpp`
- `src/soyokaze/mainwindow/layout/MainWindowAppearance.h/.cpp`
- `src/soyokaze/mainwindow/layout/WindowTransparency.h/.cpp`
- `tests/testcode/soyokaze/mainwindow/state/LauncherWindowStateTest.cpp`

