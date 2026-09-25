# 285 バージョン更新確認機能

## 概要

バージョン情報ダイアログから、公開されている最新バージョンを手動で確認する。
更新確認ではバージョン情報を取得するだけで、自動ダウンロードや自動更新は行わない。

## 関連ファイル

- `src/soyokaze/commands/builtin/AboutDlg.cpp`
- `src/soyokaze/commands/builtin/AboutDlg.h`
- `src/soyokaze/utility/UpdateInfo.cpp`
- `src/soyokaze/utility/UpdateInfo.h`
- `src/soyokaze/utility/WinHttp.cpp`
- `src/soyokaze/utility/WinHttp.h`
- `src/soyokaze/Soyokaze.rc`
- `src/soyokaze/resource.h`
- `tests/testcode/soyokaze/utility/UpdateInfoTest.cpp`

## 更新情報の公開形式

更新情報は次のURLからJSONとして取得する。

```text
https://ampmmn.github.io/version/soyokaze/update.json
```

現在参照するのは`latest`だけである。`0.*`の系列別情報は将来の拡張用だが、現時点では使用しない。

```json
{
  "latest": {
    "version": "0.60.0",
    "date": "2026-10-01",
    "url": "https://github.com/ampmmn/Soyokaze/releases/latest"
  },
  "0.*": {
    "version": "0.60.0",
    "date": "2026-10-01",
    "url": "https://github.com/ampmmn/Soyokaze/releases/tag/0.60.0"
  }
}
```

`latest.version`と`latest.url`は必須である。`version`は数値3要素の`major.minor.build`形式、`url`はHTTPS URLとする。`date`は任意で、指定する場合は実在する日付を`YYYY-MM-DD`形式で記述する。未知のキーは無視される。

## 処理の流れ

1. `CAboutDlg::OnInitDialog`で更新確認ボタン、結果表示欄、更新リンクの表示状態をコードから設定する。リソース上のコントロールは初期状態で非表示である。
2. 更新確認ボタンを押すと、`CAboutDlg::OnButtonCheckUpdate`がボタンを一時的に無効化し、確認中であることを表示する。
3. `launcherapp::WinHttp`の`LoadBinaryContent`でJSONを取得する。HTML以外のコンテンツを扱うため、`LoadContent`ではなく`LoadBinaryContent`を使用する。
4. `ParseUpdateInfo`がJSONの必須項目、バージョン、URL、日付を検証する。
5. `VersionInfo::GetVersionInfo`から取得した現在のバージョンと比較し、更新の有無を判定する。
6. 更新がある場合は更新日とリリースページへのSysLinkを表示する。リンクは既存のブラウザ設定を通じて開く。

## バージョン番号の扱い

Windowsの実行ファイルバージョンからは`major.minor.build`の3要素を使用する。4つ目の要素は比較に含めない。たとえばリソースの`FILEVERSION 0,58,1,1`から比較に使う値は`0.58.1`となる。

各要素は符号なし整数として解析し、次の順に比較する。

1. major
2. majorが同じ場合はminor
3. majorとminorが同じ場合はbuild

加算で単一の数値に変換しないため、各要素の桁数に依存した衝突を避けられる。要素が3つでない場合、数字以外を含む場合、または整数の範囲を超える場合は不正なバージョンとして扱う。

## 表示とフォールバック

- 最新バージョンが現在のバージョンより新しい場合：更新版がリリースされたことを表示する。`date`がある場合は更新日も表示し、URLリンクを設定する。
- 最新バージョンが現在と同じか古い場合：「利用中のバージョンは最新です。」と表示する。
- 通信失敗、JSON不正、必須情報の欠落、現在のバージョン取得失敗の場合：「更新を確認できませんでした。」と表示する。

エラーを「最新」と表示すると誤解につながるため、更新なしと確認失敗は区別する。

## WinHTTPタイムアウト

`WinHttp::SetTimeout`はタイムアウトをミリ秒で受け取り、WinHTTPセッションごとに名前解決・接続・送信・受信の各タイムアウトへ同じ値を設定する。値の既定値は`-1`で、WinHTTPの既定値を使用する。

更新確認では`SetTimeout(5000)`を設定する。この5秒は各通信段階に対するタイムアウトであり、リクエスト全体の経過時間を5秒に制限するものではない。ほかの`WinHttp`利用箇所は、個別に設定しない限り既定値で動作する。

## テスト

`UpdateInfoTest`では、バージョンの解析と要素ごとの比較、JSON解析、任意の日付、必須項目の欠落、不正なURL・日付・バージョンを確認する。通信処理およびダイアログ表示は単体テストの対象ではないため、変更時には実際のダイアログで更新あり・更新なし・確認失敗時の表示も確認する。

## 運用・変更時の注意点

- 新しいリリースを公開するときは、配布する実行ファイルの先頭3つのバージョン要素と`latest.version`を一致させる。
- リリースリンクにはHTTPS URLを設定する。
- 更新確認の対象系列を増やす場合は、JSONの系列選択規則とクライアント側の参照方法を合わせて拡張する。現在は`latest`以外を参照しない。
- コントロールの表示条件を環境ごとに変える場合は、リソースの非表示状態を維持し、`OnInitDialog`側の表示制御を変更する。
