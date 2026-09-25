# ウインドウ位置情報の仕様

## 概要

ランチャーのメインウインドウの位置とサイズを、モニター構成ごとに記憶する。
保存位置がないモニター構成へ切り替わった場合は、別構成の位置を流用せず、現在位置を維持する。

位置復元はウインドウの表示状態とは独立している。非表示中に位置を復元してもウインドウを表示せず、次回の通常の表示要求時に復元済みの位置を使用する。

## 関連ファイル

- `src/soyokaze/control/WindowPosition.h`
- `src/soyokaze/control/WindowPosition.cpp`
- `src/soyokaze/mainwindow/layout/MainWindowPosition.h`
- `src/soyokaze/mainwindow/layout/MainWindowPosition.cpp`
- `src/soyokaze/mainwindow/layout/MainWindowLayout.h`
- `src/soyokaze/mainwindow/layout/MainWindowLayout.cpp`
- `src/soyokaze/app/LauncherSystemEventWindow.cpp`
- `src/soyokaze/core/LauncherEventListenerIF.h`
- `src/soyokaze/mainwindow/LauncherMainWindow.cpp`
- `src/soyokaze/utility/Base64.cpp`
- `src/soyokaze/utility/SHA1.cpp`
- `tests/testcode/soyokaze/mainwindow/layout/MainWindowPositionTest.cpp`

## 保存先とデータ形式

位置情報ファイルはアプリケーションのプロファイルディレクトリにある `windowplacement.yaml` である。

YAMLはモニター構成識別子をキー、`WINDOWPLACEMENT` のBase64表現を値とするマップである。

```yaml
<モニター構成識別子>: "<WINDOWPLACEMENTのBase64データ>"
```

値は `WINDOWPLACEMENT` 構造体のバイト列をBase64エンコードしたものである。読み込み時は、デコード後のバイト列が構造体と同じサイズであること、および `length` が `sizeof(WINDOWPLACEMENT)` と一致することを検証する。YAMLの読み込みサイズ上限は1 MiBである。

この値は構造体のメモリ表現を保存する実装形式であり、フィールドごとの移植可能なテキスト形式ではない。

## モニター構成識別子

識別子は、`EnumDisplayMonitors()` で列挙した各モニターの矩形から生成する。モニター名や列挙順には依存しない。

1. 各矩形を `left`, `top`, `right`, `bottom` の順で比較し、座標順にソートする。
2. 各矩形を `left,top,right,bottom;` の形式で連結する。
3. 連結したASCII文字列のSHA-1フルダイジェストを小文字の16進数で表す。

識別子は40文字である。モニター名、EDID、DPI、列挙順は識別には使用しない。同じ矩形一覧なら同じ構成となり、モニターの位置または矩形サイズが変わると構成識別子も変わる。

## 起動時の復元

`MainWindowLayout::RestoreWindowPosition()` が `MainWindowPosition::Restore()` を呼び出し、現在のモニター構成に対応する位置を探す。

復元順序は以下のとおりである。

1. YAMLファイルを読み込めた場合、現在の構成識別子と一致するエントリだけを適用する。
2. YAMLが存在しない、または読み込み・解析に失敗した場合は、旧形式の `<ウインドウ名>.position`、続けて `Soyokaze.position` を試す。
3. 旧形式の位置を読み込めた場合は、現在の構成識別子のエントリとして扱い、以後の保存でYAML形式へ移行する。
4. 復元できなかった場合、呼び出し側は通常サイズ600×300のウインドウを中央に配置し、その位置を現在の構成の位置として記録する。

YAMLの解析に成功したものの現在の構成エントリがない場合、旧形式ファイルへはフォールバックしない。別構成の位置を使う `default` フォールバックも行わない。
旧YAMLに残っている `default` キーは読み込み時に無視し、YAML書き出し時にも出力しない。

復元位置を適用した後、ウインドウ矩形がどのモニターとも交差しない場合は適用を失敗とし、適用前の位置へ戻す。

## 保存とサイズの扱い

`WindowPosition` の破棄時に `Save()` を呼び出し、現在の構成の位置をYAMLへ保存する。保存時には保持中の全構成エントリを書き出す。

位置情報の更新は次のように行う。

- キーワード入力中は、現在の `WINDOWPLACEMENT` を記録する。
- キーワード未入力時は、入力欄だけの表示サイズに縮めるが、記録済みの高さは維持する。
- そのため、次にキーワード入力を始めると、候補欄を含む前回の高さへ戻せる。

`MainWindowPosition::SetPositionTemporary()` による一時的な縮小は、保存済みの位置・サイズを直接書き換えない。通常のウインドウ移動やサイズ変更に伴う更新は `MainWindowLayout::RecalcControls()` から行われる。

## モニター構成変更

`LauncherSystemEventWindow` は `WM_DEVICECHANGE`、`WM_DISPLAYCHANGE`、`WM_SETTINGCHANGE` を監視する。通知が連続する場合は500msのデバウンス後に `OnMonitorConfigurationChanged()` を通知する。

位置管理の処理は次のとおりである。

1. 現在のモニター構成を再計算する。
2. 変更がなければ `MonitorChangeResult::Unchanged` を返す。
3. 変更があれば、読み込み済みの現在位置を変更前の構成に保存し、新しい構成へ切り替える。
4. 新しい構成の位置情報があれば適用し、`PlacementRestored` を返す。
5. 位置情報がなければ現在位置を維持し、現在の `WINDOWPLACEMENT` を取得して `NoSavedPlacement` を返す。
6. 構成が変わった場合は、位置情報の有無にかかわらず、実際の入力状態でレイアウトを再同期する。

`MainWindowLayout::UpdateInputStatus()` は位置・サイズの同期後に `RecalcControls()` を呼び出す。これにより、ウインドウサイズが変わらず `WM_SIZE` が発生しない場合も、キーワードの有無に応じて候補欄を表示・非表示にできる。

## 非表示状態の維持

保存済み `WINDOWPLACEMENT` には保存時の `showCmd` が含まれる。復元対象のウインドウが非表示の場合、この `showCmd` をそのまま適用すると、位置復元を契機にウインドウが表示される可能性がある。

そのため、位置適用時と `MainWindowPosition::SyncPosition()` では、実際のウインドウが非表示なら、適用する一時コピーの `showCmd` を `SW_HIDE` にする。保存済みの `mPosition` 自体は変更しないため、後の表示要求に必要な位置情報は保持される。

この処理はウインドウStateを変更しない。非表示時は `HiddenState` のままになり、次回の通常の表示要求で表示される。

## 関連テスト

`MainWindowPositionTest.SyncPosition_DoesNotShowHiddenWindow` は、非表示のウインドウに対して位置同期を行っても表示状態が変わらず、保存済みの `showCmd` も維持されることを確認する。

モニター構成変更の通知順序や実機の着脱動作は、複数ディスプレイ環境で別途確認する。
