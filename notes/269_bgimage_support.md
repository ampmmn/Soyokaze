# 269 候補欄の背景画像対応

## 概要

候補欄の背景に画像を表示する機能を追加した。
画像の表示位置、拡大縮小方法、透過率を設定できる。

画像を表示できない場合は、従来の`StandardCandidateListRenderer`による描画へフォールバックする。

## 関連ファイル

- `src/soyokaze/mainwindow/CandidateListRenderer.h`
- `src/soyokaze/mainwindow/StandardCandidateListRenderer.h`
- `src/soyokaze/mainwindow/StandardCandidateListRenderer.cpp`
- `src/soyokaze/mainwindow/BGImageCandidateListRenderer.h`
- `src/soyokaze/mainwindow/BGImageCandidateListRenderer.cpp`
- `src/soyokaze/mainwindow/CandidateListCtrl.cpp`
- `src/soyokaze/settingwindow/AppSettingPageBGImage.cpp`

## クラス構成

`CandidateListRenderer`は候補項目の描画方式を抽象化する基底クラスである。

`StandardCandidateListRenderer`は、従来の候補項目描画を担当する。
アイコン、コマンド名、コマンド種別、選択状態、通常の背景色および交互背景色を描画する。

`BGImageCandidateListRenderer`は`StandardCandidateListRenderer`を継承し、背景画像の合成だけを追加する。
候補項目の文字やアイコンの描画は、背景画像を合成した後に基底クラスへ委譲する。

内部状態はPImplに保持している。主な状態は以下のとおりである。

- 読み込んだ背景画像
- 背景画像の描画バッファ
- 通常背景色のバッファ
- 交互背景色のバッファ
- 画像の描画矩形
- 透過率と表示位置
- 現在の候補欄サイズ
- 交互背景色の有効状態

## レンダラーの切り替え

`CandidateListCtrl::InitColumns()`で`BGImage:Enable`を確認し、使用するレンダラーを作成する。

```text
BGImage:Enable == true
    -> BGImageCandidateListRenderer

BGImage:Enable == false
    -> StandardCandidateListRenderer
```

作成したレンダラーには、候補一覧、空欄状態、交互背景色、コマンド種別表示、アイコン表示、文字サイズを設定する。

`SetIsAlternateColor()`は仮想関数として定義している。これにより、背景画像レンダラーでも交互背景色の設定を保持できる。

## 設定値

背景画像関連の設定キーは以下のとおりである。

- `BGImage:Enable`
  - 背景画像表示の有効・無効
- `BGImage:BGImageFilePath`
  - 背景画像のファイルパス
- `BGImage:Alpha`
  - 背景画像の透過率。0～100の範囲で扱う
- `BGImage:Position`
  - 背景画像の表示位置および拡大縮小方式

表示位置の値は以下のとおりである。

- `0`: 左上
- `1`: 右上
- `2`: 中央
- `3`: 左下
- `4`: 右下
- `5`: 長辺を候補欄の長辺に合わせる
- `6`: 短辺を候補欄の短辺に合わせる

## 描画処理

### バッファ作成

`BGImageCandidateListRenderer::UpdateSize()`で候補欄のサイズを記録し、次回の描画時にバッファを作り直す。

`PImpl::CreateBuffers()`では以下のバッファを作成する。

1. 背景画像を描画する32bitバッファ
2. 通常背景色を塗ったバッファ
3. 交互背景色を塗ったバッファ

背景色バッファを2つ用意することで、画像の外側だけでなく、画像と重なる領域の背景色も候補行ごとに切り替えられる。

### 画像の拡大縮小

画像の描画矩形を表示位置と設定値から計算する。

長辺合わせまたは短辺合わせの場合は、元画像のアスペクト比を維持して描画サイズを計算する。

画像をバッファへ描画するときは、描画先DCに以下を設定する。

- `HALFTONE`補間
- `SetBrushOrgEx()`によるブラシ原点の調整

これにより、単純な最近傍補間による色の不連続や画素の乱れを抑える。

### 背景画像と背景色の合成

`PImpl::DrawComposite()`が指定された矩形へ背景画像と背景色を合成する。

1. 指定矩形と画像矩形の共通部分を求める
2. 画像部分へ背景画像を`AlphaBlend()`する
3. 画像の外側へ背景色を合成する
4. 画像部分には透過率に応じた背景色を合成する

交互背景色が有効で、対象行の番号が奇数の場合は交互背景色バッファを使用する。無効の場合は通常背景色バッファを使用する。

### 標準描画との順序

`BGImageCandidateListRenderer::DrawItem()`では、まず背景画像と背景色を合成する。
その後、`StandardCandidateListRenderer::DrawItem()`を呼び出して候補項目を描画する。

この順序により、以下の要素は背景画像より前面に表示される。

- 選択行の背景色
- 選択行の文字色
- アイコン
- コマンド名
- コマンド種別

背景画像レンダラーでは基底クラスの背景描画を無効にしている。ただし、選択状態の背景色は標準描画の処理で引き続き描画される。

## 空欄と末尾の余白

候補が空の場合でも、候補欄全体を背景画像と背景色で描画する。
空欄描画はダミー項目を0番目として扱い、通常の候補行と同じ規則で交互背景色を切り替える。

候補が表示されている場合は、最後の候補項目より下の余白も描画する。
余白の行番号は最後の候補の次の番号から開始し、交互背景色の並びを継続する。

## フォールバック

以下の場合、背景画像レンダラーは描画バッファを使用できない。

- 背景画像のパスが空
- 画像の読み込みに失敗
- 描画領域のサイズが不正
- 画像または背景色バッファの作成に失敗
- DCの取得に失敗
- 背景画像の描画に失敗

この場合は`SetIsDrawBackground(true)`に切り替え、一時的に`StandardCandidateListRenderer`の通常描画を実行する。

画像ロード、バッファ作成、画像描画、`AlphaBlend()`失敗時には、原因の特定に必要な情報を英文ログへ出力する。

## 実装上の注意点

- 背景画像を表示する場合も、選択行の背景色を画像より前面に描画する
- 交互背景色を使用する場合は、通常色と交互色の両方のバッファを更新する
- 候補欄のサイズ変更後は、画像の位置と拡大縮小率が変わる可能性があるためバッファを再生成する
- `HALFTONE`を設定したDCの状態は`ScopedDCState`で復元する
- 画像を読み込めない場合でも候補項目自体は標準レンダラーで表示する
- ログメッセージは既存の方針に合わせて英文とする
