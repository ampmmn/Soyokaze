# office_favorites

Word/Excel/PowerPointのお気に入り(ピン止め)したファイルを候補として扱う機能の実装です。
Issue #314 に対応します。

## クラス構成

| クラス | 役割 |
|---|---|
| `OfficeFavoritesProvider` | 設定の読み込み、レジストリ/JSONの変更監視、コマンドの生成を担当する。PImpl で `AppPreferenceListenerIF` と `LauncherEventListenerIF` を実装する |
| `OfficeFavoritesCommand` | お気に入り1件に対応するコマンド。`ContextMenuSource` を実装する |
| `OfficeFavorites` | お気に入りの保持と問い合わせへの応答を担当する。レジストリ取得と JSON 取得は独立したメソッドで行う |

## データの取得元

### ローカルファイル(レジストリ)

- `HKCU\Software\Microsoft\Office\16.0\{Excel,Word,PowerPoint}\User MRU\*\File MRU`
- 値の名前は `Item N`、値の種類は `REG_SZ`
- 値の形式: `[F0000000x][T...][O...]*(パス)`
  - `[F00000001]` がピン止め、`[F00000000]` が未ピン止め
  - `*` 以降がファイルの絶対パス
- ファイルが存在しない項目は保持しない

### SharePoint/OneDrive 上のファイル(JSON)

- `%LOCALAPPDATA%\Microsoft\Office\16.0\aggmru\*\` 配下の `x-mru4-*-sr.json`(Excel)、`w-mru4-*-sr.json`(Word)、`p-mru4-*-sr.json`(PowerPoint)
- `documents.items[]` のうち `is_pinned` が `true` かつ `app` が `Excel` / `Word` / `PowerPoint` の項目を対象とする
- `url` をファイルの URL として扱う。URL は存在確認を行わない

## 更新の検知

### レジストリ

- `RegNotifyChangeKeyValue` で `User MRU` キー(アプリごと、サブツリーを含む)を監視する
- 監視キーごとに独立したイベントを用意する
- 通知は一度きりで解除されるため、通知を受けるたびに再登録してから読み直す
- 変更の確認は `LauncherEventListenerIF::OnTimer` で行う(`LauncherSystemEventWindow` の 1 秒周期タイマーから呼ばれる)

### JSON

- `LocalDirectoryWatcher::Register` で JSON ファイルを登録し、コールバックで再読み込みする
- コールバックは監視スレッドで呼ばれるため、再読み込み結果の反映(swap)は `std::mutex` で保護する

## 操作

| キー | 動作 |
|---|---|
| `Enter` | ローカルファイルは関連付けで開く。URL は対応するアプリ(`App Paths` の実行ファイル)に URL を渡して開く |
| `Ctrl+Enter` | ローカルファイルはフォルダを開く。URL は URL を開く |
| `Alt+Enter` | 対象外 |

## 制限事項

- ピン止めの変更が JSON に反映されるまでに遅延がある(Office 側の同期タイミングによる)
- JSON の監視は、起動時(機能の有効化時)に存在したファイルのみが対象となる。後からアカウント用のフォルダが作成された場合は、機能の再有効化まで検知しない
- 設定 `Soyokaze:IsEnableOfficeFavorites` を無効にすると、監視を停止して一覧を破棄する

## 設定

- キー: `Soyokaze:IsEnableOfficeFavorites`
- 既定値: `true`
- 設定画面: 「拡張機能」→「システム関連の機能」→「Word/Excel/PowerPointのお気に入りを表示する」

## テスト

- `tests/testcode/soyokaze/commands/office_favorites/OfficeFavoritesTest.cpp`
- レジストリ値の解析、JSON の解析、監視キーの一覧を対象とする。レジストリ監視と JSON 監視そのものは対象外

