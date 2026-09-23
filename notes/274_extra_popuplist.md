# 274 実行時引数に対する追加候補ポップアップ

## 概要

コマンドの実行時引数を入力しているときに、入力中のパラメータに対応するコマンド候補をポップアップ表示する機能を追加した。

通常の候補欄は入力されたコマンド名を検索するために使用する。一方、追加候補ポップアップは、現在選択中のコマンドの後ろに入力されたパラメータを検索するために使用する。

追加候補の検索対象は、現在選択中のコマンドが`IsAcceptArguments()`で引数を受け付ける場合に限る。検索結果は`CanResolve()`が`true`を返すコマンドだけに絞り込む。

## 関連ファイル

- `src/soyokaze/mainwindow/ExtraCandidateListCtrl.h`
- `src/soyokaze/mainwindow/ExtraCandidateListCtrl.cpp`
- `src/soyokaze/mainwindow/LauncherMainWindow.h`
- `src/soyokaze/mainwindow/LauncherMainWindow.cpp`
- `src/soyokaze/mainwindow/state/MainWindowSearchingState.h`
- `src/soyokaze/mainwindow/state/MainWindowSearchingState.cpp`
- `src/soyokaze/mainwindow/state/MainWindowParamSearchingState.h`
- `src/soyokaze/mainwindow/state/MainWindowParamSearchingState.cpp`
- `src/soyokaze/mainwindow/state/LauncherWindowStateContextIF.h`
- `src/soyokaze/mainwindow/state/LauncherWindowState.h`

## Issueの要件

Issue #274では、以下の仕様を定めている。

- 現在選択中のコマンドが`IsAcceptArguments() == true`のときに追加候補を表示する
- 現在入力中のトークンを検索語として`CommandRepository::Query()`を実行する
- `CanResolve() == false`のコマンドは候補から除外する
- 引用符で始まるトークンは文字列リテラルとして扱い、候補を表示しない
- 空のトークンでは候補を表示しない
- コマンド名部分にキャレットがある場合は候補を表示しない
- 追加候補の検索は同期処理でよい
- 候補が0件の場合はポップアップを表示しない
- `Tab`または`Enter`で選択中の候補を確定する
- メインウインドウが非アクティブになった場合はポップアップを閉じる

## クラス構成

### ExtraCandidateListCtrl

`ExtraCandidateListCtrl`は`CListCtrl`を継承した追加候補用のリストコントロールである。

主な責務は以下のとおりである。

- 候補コマンドの保持
- 候補名のリスト表示
- 現在の選択項目の管理
- 上下キーによる選択移動
- ポップアップの表示・非表示
- 候補欄と同じ配色による描画

内部のポップアップウインドウは`ExtraCandidatePopupWnd`が保持する。リストコントロール自体はポップアップウインドウの子ウインドウとして作成する。

### ExtraCandidatePopupWnd

ポップアップウインドウは`CWnd`を継承した内部クラスである。

以下のスタイルで作成する。

- 拡張スタイル: `WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE`
- ウインドウスタイル: `WS_POPUP`

`WS_EX_NOACTIVATE`により、ポップアップ表示時もメインウインドウの入力フォーカスを維持する。

子リストから通知された`WM_NOTIFY`は、ポップアップウインドウを経由して所有者である`LauncherMainWindow`へ転送する。これにより、追加候補のクリックをメインウインドウのStateで処理できる。

## LauncherMainWindowでの保持

`LauncherMainWindow::PImpl`は以下を保持する。

- `ExtraCandidateListCtrl mExtraCandidateListBox`
- `std::vector<RefPtr<Command>> mExtraCandidates`
- 追加候補検索中であることを示す状態

ポップアップの生成はメインウインドウの初期化時に行う。フォント変更時にはメイン候補欄と同じフォントを設定する。

追加候補が不要になった場合は、`HideExtraCandidates()`でポップアップを非表示にし、保持している候補を破棄する。

## 追加候補検索の開始判定

`LauncherMainWindow::CanStartParamSearching()`が、追加候補検索を開始できるかを判定する。

次の条件を順に確認する。

1. キャレット位置に選択範囲がない
2. 現在の入力がパラメータ部分を含む
3. パラメータが空ではない
4. パラメータが引用符で始まっていない
5. 現在選択中のコマンドが存在する
6. 現在選択中のコマンドが`IsAcceptArguments()`を返す

最後の条件が`false`の場合、追加候補は表示しない。ディレクトリや絶対パスであることだけを理由に追加候補検索を開始する処理は、Issue #274の要件と異なるため採用しない。

## パラメータ範囲の取得

`GetPathParameterRange()`は、コマンド後の絶対パスを含む入力から検索対象の範囲を取得する。

認識する絶対パスの形式は以下のとおりである。

- ドライブレターで始まるパス: `C:\\path`
- スラッシュ区切りのドライブパス: `C:/path`
- UNCパス: `\\\\server\\share`

絶対パスがコマンドの後ろにある場合は、空白を区切りとして開始位置を探す。連続した空白は読み飛ばす。

この処理により、次のような入力でもパス全体を検索対象にできる。

```text
command C:\\path with spaces\\
```

絶対パスの範囲取得は、パラメータ検索を許可する判定とは別の処理である。最終的な表示可否は、現在選択中のコマンドの`IsAcceptArguments()`で決まる。

## 候補検索

`UpdateExtraCandidates()`は、以下の手順で候補を更新する。

1. 現在の入力文字列とキャレット位置から検索対象のトークン範囲を取得する
2. 対象トークンを検索語として`CommandQueryRequest`を作成する
3. `CommandRepository::Query()`を実行する
4. 検索完了を最大2秒待つ
5. 検索結果から`CanResolve() == true`のコマンドだけを残す
6. `ExtraCandidateListCtrl`へ候補を設定する
7. 候補が空でなければポップアップを表示する

検索結果が取得できない場合、タイムアウトした場合、または候補が0件の場合は、ポップアップを非表示にする。

候補は検索結果のコマンド名を1列で表示する。`Resolve()`は候補の確定時まで呼び出さない。

## ポップアップの配置と外観

ポップアップはキャレット位置を基準に配置する。

- X座標: キャレットの画面座標
- Y座標: 入力欄の下側
- 幅: 320px
- 高さ: 候補数に応じて変更
- 最大表示行数: 16行

候補欄の外観に合わせるため、以下を使用する。

- メインウインドウと同じフォント
- 通常の文字色
- 通常の背景色
- 交互行の背景色
- 選択行の文字色と背景色

追加候補欄では、アイコン、コマンド種別、ヘッダー、背景画像は表示しない。

## State遷移

### SearchingStateからParamSearchingStateへ

通常の候補検索が完了した後、`SearchingState::OnQueryCompleted()`から`CanStartParamSearching()`を呼び出す。

追加候補検索を開始できる場合は、`RequestParamSearching()`でメッセージをキューへ登録し、その後`ParamSearchingState`へ遷移する。

検索完了通知を直接のState遷移に使用せず、メッセージキューを経由するのは、通常候補の選択変更通知を先に処理するためである。

### ParamSearchingState

`ParamSearchingState::OnEnter()`で追加候補の検索とポップアップ表示を行う。

主な処理は以下のとおりである。

- `OnTextChanged()`で入力変更後の候補を再検索する
- 候補がなくなった場合は`SearchingState`へ戻る
- `OnCancel()`でポップアップを閉じて`SearchingState`へ戻る
- `OnDeactivate()`でポップアップを閉じて`HiddenState`へ遷移する
- `OnWindowGeometryChanged()`で`SearchingState`へ戻る
- `OnExit()`でポップアップを必ず閉じる

### Tab確定後の遷移

`Tab`で候補を確定した場合は、`ResolveExtraCandidate()`で入力内容を更新した後、`SearchingState`へ戻る。

このときは通常候補検索を再実行し、入力内容に応じて追加候補を再表示できる状態にする。

### Enter確定後の遷移

`Enter`で候補を確定した場合は、候補を解決した後に通常の実行処理へ戻る。追加候補ポップアップは再表示しない。

## キー・マウス操作

`ParamSearchingState::OnKeyInput()`が追加候補表示中のキー入力を処理する。

- `↑`: 1つ前の候補へ移動する
- `↓`: 1つ後の候補へ移動する
- `Tab`: 選択中の候補を入力欄へ反映する
- `Enter`: 選択中の候補を確定する

上下キーによる選択は末尾から先頭、先頭から末尾へ循環する。

追加候補リストのクリック通知は`LauncherMainWindow::OnNotify()`で受け取り、現在のStateの`OnExtraCandidateClicked()`へ渡す。クリック時は選択中の候補を確定する。

## 候補の確定と入力文字列の置換

`ResolveExtraCandidate()`は、追加候補から現在選択中のコマンドを取得し、`Resolve()`を呼び出す。

`Resolve()`で得られた値は、現在のトークン範囲だけを置き換える。トークンより後ろに入力されている文字列は維持する。

例えば、次の入力で`param`を候補から確定した場合、`tail`は維持する。

```text
hoge param tail
```

置換後は、展開された値の直後へキャレットを移動し、通常の候補検索を再実行する。

## 実装上の注意点

- `IsAcceptArguments()`は、追加候補ポップアップを表示するコマンドかどうかの判定に使用する
- `CanResolve()`は、追加候補として表示できるコマンドかどうかの判定に使用する
- `IsAcceptArguments()`と`CanResolve()`は別の能力を表すため、同じ条件として扱わない
- ポップアップ表示中も通常候補欄の選択コマンドは変更しない
- メインウインドウが非アクティブになったらポップアップを閉じる
- ポップアップを閉じるときは表示状態だけでなく保持候補も破棄する
- 同期検索の待機時間は2秒である
- ポップアップ自身はフォーカスを取得しない
- 固定幅や最大表示行数は現時点の実装値であり、設定項目ではない

## 仕様変更時の確認事項

以下を変更する場合は、通常候補検索との状態遷移を合わせて確認する必要がある。

- `IsAcceptArguments()`の判定条件
- 絶対パスを含むトークン範囲の取得
- `CanResolve()`による候補の絞り込み
- `Tab`確定後に追加候補を再表示する条件
- `Enter`確定後の実行処理
- キャレット移動や選択範囲変更時のState遷移
- メインウインドウの移動・リサイズ・非アクティブ化時のポップアップ処理
