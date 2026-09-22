# LauncherMainWindow State Pattern

## 目的

`LauncherMainWindow`は、ウインドウの表示状態、入力内容、候補一覧、キー操作、コマンド実行を管理する必要がある。
これらの処理を単一のウインドウクラスに集約すると、状態ごとの条件分岐が増え、入力処理の変更が難しくなる。

State Patternを利用し、現在の状態に応じた処理をStateクラスへ委譲する。
`LauncherMainWindow`はメッセージの受け取りと、Context APIによる既存処理の提供を担当する。

## クラス構成

```text
LauncherWindowState
        |
LauncherWindowStateBase
        |
  +-----+-----+
  |           |
HiddenState  IdleState  SearchingState
```

実際には`HiddenState`、`IdleState`、`SearchingState`の3クラスが、`LauncherWindowStateBase`から直接派生する。
表示中State専用の追加中間層は設けていない。

### LauncherWindowState

ファイル:

- `src/soyokaze/mainwindow/state/LauncherWindowState.h`

Stateが受け取るイベントの純粋なインターフェースを定義する。
Contextや状態固有のデータは保持しない。

主なイベントは以下のとおり。

- `OnEnter()` / `OnExit()`: Stateの開始・終了
- `OnActivate()`: ウインドウ表示要求
- `OnDeactivate()`: ウインドウ非表示要求
- `OnExecuteRequested()`: コマンド実行要求
- `OnCancel()`: キャンセル操作
- `OnContentCleared()`: 入力内容のクリア通知
- `OnTextChanged()`: 入力内容の変更通知
- `OnQueryCompleted()`: 非同期検索完了通知
- `OnKeyInput()`: 入力欄でのキー入力
- `OnCandidateSelectionChanged()`: 候補選択変更
- `OnCandidateClicked()` / `OnCandidateDoubleClicked()`: 候補クリック操作

### LauncherWindowStateBase

ファイル:

- `src/soyokaze/mainwindow/state/LauncherWindowStateBase.h`
- `src/soyokaze/mainwindow/state/LauncherWindowStateBase.cpp`

Stateの共通実装を提供する中間クラスである。

- `LauncherWindowStateContextIF`へのポインタを保持する
- `GetContext()`で派生クラスへContextを提供する
- Stateごとの差分がないイベントに既定動作を提供する
- 既定のキー入力は`false`を返す

コンストラクタは`protected`であり、通常は具象Stateを通じて利用する。

### HiddenState

ファイル:

- `src/soyokaze/mainwindow/state/MainWindowHiddenState.h`
- `src/soyokaze/mainwindow/state/MainWindowHiddenState.cpp`

ランチャーウインドウが非表示の状態を表す。

- `OnEnter()`でウインドウが表示中なら非表示にする
- `OnActivate()`でウインドウを表示する
- 再表示時にキーワードがあれば`SearchingState`へ遷移する
- 再表示時にキーワードがなければ`IdleState`へ遷移する
- 非表示中の検索完了はContextへ委譲する

### IdleState

ファイル:

- `src/soyokaze/mainwindow/state/MainWindowIdleState.h`
- `src/soyokaze/mainwindow/state/MainWindowIdleState.cpp`

ウインドウが表示され、キーワード入力がない状態を表す。

- 表示要求、再アクティブ化、トグル非表示を処理する
- `Enter`で現在のコマンドを実行する
- キーワードがある場合のキャンセルでは、入力内容だけをクリアする
- キーワードがない場合のキャンセルでは、ウインドウを非表示にする
- 入力が開始されたら`SearchingState`へ遷移する
- 候補操作用のキーは処理しない

### SearchingState

ファイル:

- `src/soyokaze/mainwindow/state/MainWindowSearchingState.h`
- `src/soyokaze/mainwindow/state/MainWindowSearchingState.cpp`

ウインドウが表示され、キーワード入力または候補検索を行っている状態を表す。

- 表示要求、再アクティブ化、トグル非表示を処理する
- 入力内容をクリアすると`IdleState`へ戻る
- 入力内容がなくなったら`IdleState`へ戻る
- 上下キーで候補を移動する
- Tabキーで候補を補完する
- Enterキーで候補を実行する
- PageUp/PageDownで候補をページ単位に移動する
- 候補のクリック、選択変更、ダブルクリックを処理する

## Context

### LauncherWindowStateContextIF

ファイル:

- `src/soyokaze/mainwindow/state/LauncherWindowStateContextIF.h`

Stateから共通処理を利用するためのインターフェースである。
Stateは`LauncherMainWindow`の具体的な型や内部メンバに直接依存せず、Context APIだけを利用する。

Context APIは次のカテゴリに分かれる。

- State遷移: `ChangeState()`
- ウインドウ操作: 表示、再アクティブ化、非表示、表示状態確認
- 入力操作: 内容クリア、フォーカス設定、キーワード確認
- 検索処理: 入力変更処理、検索完了処理
- 候補操作: 選択移動、現在候補の反映、補完、候補選択
- コマンド実行: 現在の候補の実行
- 設定確認: トグル表示設定の確認

### LauncherMainWindowによる実装

`LauncherMainWindow`が`LauncherWindowStateContextIF`を実装する。
Stateから呼び出されたContext APIでは、既存のMainWindow処理を再利用する。

例えば、Stateからの表示要求は`ShowWindowFromState()`で受け取り、既存の表示処理を実行する。
Stateに直接MFC操作や内部UI部品操作を書かないことが重要である。

## Stateの所有と遷移

現在のStateは`LauncherMainWindow::PImpl::mState`が所有する。
型は`std::unique_ptr<LauncherWindowState>`であり、具体的なStateの所有権はContextの`ChangeState()`へ渡す。

`ChangeState()`の処理順序は次のとおり。

1. 遷移先が`nullptr`の場合は何もしない
2. 現在のStateがあれば`OnExit()`を呼び出す
3. `mState`を新しいStateへ置き換える
4. 新しいStateの`OnEnter()`を呼び出す

Stateの生成と遷移はState側からContext経由で行う。

```cpp
context->ChangeState(std::make_unique<SearchingState>(context));
```

## 状態遷移

```text
HiddenState --表示要求・キーワードなし--> IdleState
HiddenState --表示要求・キーワードあり--> SearchingState
IdleState --キーワード入力-------------> SearchingState
SearchingState --内容クリア・空入力-----> IdleState
IdleState --入力なしでキャンセル--------> HiddenState
IdleState/SearchingState --非表示要求--> HiddenState
```

主な遷移条件は以下のとおり。

| 現在のState | イベント | 条件 | 遷移先 |
| --- | --- | --- | --- |
| `HiddenState` | `OnActivate()` | キーワードあり | `SearchingState` |
| `HiddenState` | `OnActivate()` | キーワードなし | `IdleState` |
| `IdleState` | `OnTextChanged()` | キーワードあり | `SearchingState` |
| `IdleState` | `OnCancel()` | キーワードなし | `HiddenState` |
| `SearchingState` | `OnTextChanged()` | キーワードなし | `IdleState` |
| `SearchingState` | `OnContentCleared()` | ウインドウ表示中 | `IdleState` |
| `SearchingState` | `OnContentCleared()` | ウインドウ非表示 | `HiddenState` |
| `IdleState` / `SearchingState` | `OnDeactivate()` | 常に | `HiddenState` |
| `IdleState` / `SearchingState` | 実行後 | ウインドウ非表示 | `HiddenState` |

検索完了後も、処理結果によってウインドウ表示状態やキーワードの有無が変わる可能性がある。
そのため、`OnQueryCompleted()`ではContextへ検索結果を反映した後、状態を確認して必要なら遷移する。

## MainWindowからStateへのイベント委譲

`LauncherMainWindow`はWindowsメッセージやMFCイベントを受け取り、現在のStateへ委譲する。

- 表示要求: `OnUserMessageActiveWindow()`から`OnActivate()`
- 非表示要求: `OnUserMessageHide()`から`OnDeactivate()`
- 検索完了: `OnUserMessageQueryComplete()`から`OnQueryCompleted()`
- 入力変更: `OnEditCommandChanged()`から`OnTextChanged()`
- Enter: `OnOK()`から`OnExecuteRequested()`
- キャンセル: `OnCancel()`から`OnCancel()`
- キー入力: `OnKeywordEditNotify()`から`OnKeyInput()`
- 候補操作: リスト通知から候補関連イベント

`OnKeyInput()`の戻り値は、キー入力をState側で処理したかどうかを表す。
`true`の場合はメッセージを処理済みとして扱い、`false`の場合は通常の処理へ委ねる。

## 非同期検索の扱い

検索処理そのものは`LauncherMainWindow`の既存処理を利用する。
Stateは検索開始を直接実装せず、Contextの`HandleTextChanged()`を呼び出す。

検索完了時は次の順序で処理する。

1. MainWindowが検索結果をContext経由で反映する
2. 結果反映後のウインドウ表示状態を確認する
3. キーワードの有無を確認する
4. 現在のStateを継続するか、別Stateへ遷移する

検索結果のポインタはContext APIを通じて渡されるため、State側で所有権を取得したり解放したりしない。

## 新しいStateを追加する場合

1. `MainWindowXxxState.h/.cpp`を作成する
2. `LauncherWindowStateBase`から派生する
3. コンストラクタでContextを基底クラスへ渡す
4. 必要なイベントだけをoverrideする
5. MainWindowの具体型には依存せず、Context APIを利用する
6. State遷移は`ChangeState()`を利用する
7. State固有の処理をユニットテストへ追加する
8. `.vcxproj`と`.filters`へファイルを登録する

Stateごとの差分がないイベントは`LauncherWindowStateBase`の既定実装が処理する。
新しいStateを追加する際に、すべてのイベントを空実装する必要はない。

## 関連ファイル

- `src/soyokaze/mainwindow/state/LauncherWindowState.h`
- `src/soyokaze/mainwindow/state/LauncherWindowStateBase.h`
- `src/soyokaze/mainwindow/state/LauncherWindowStateBase.cpp`
- `src/soyokaze/mainwindow/state/LauncherWindowStateContextIF.h`
- `src/soyokaze/mainwindow/state/MainWindowHiddenState.h`
- `src/soyokaze/mainwindow/state/MainWindowHiddenState.cpp`
- `src/soyokaze/mainwindow/state/MainWindowIdleState.h`
- `src/soyokaze/mainwindow/state/MainWindowIdleState.cpp`
- `src/soyokaze/mainwindow/state/MainWindowSearchingState.h`
- `src/soyokaze/mainwindow/state/MainWindowSearchingState.cpp`
- `src/soyokaze/mainwindow/LauncherMainWindow.h`
- `src/soyokaze/mainwindow/LauncherMainWindow.cpp`
- `tests/testcode/soyokaze/mainwindow/state/LauncherWindowStateTest.cpp`
