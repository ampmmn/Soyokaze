# shell_destinations

Windowsのジャンプリストにピン留めされた項目を読み込み、Soyokazeの検索候補として表示する機能です。利用者向けの操作説明は[ジャンプリストのマニュアル](../../../../doc/manuals/source/adhoc-command/shell_destinations.md)を参照してください。

## クラス構成

| クラス／構造体 | 役割 |
|---|---|
| `ShellDestinationCommandProvider` | Automatic Destinationsファイルの監視と再読み込みを行い、検索結果に応じてコマンドを生成する |
| `AutomaticDestinationsFile` | Automatic Destinationsファイルの`DestList`とShell Linkを解析し、ピン留め項目を取り出す |
| `AutomaticDestinationEntry` | 表示名、Shell Linkのバイナリ、リンク先がディレクトリかどうかを保持する |
| `OleCfFile` | OLE Compound Fileのディレクトリとストリームを解析し、ストリームのデータを読み出す |
| `OleCfHeader` / `OleCfDirectoryEntry` | OLE Compound Fileのヘッダー情報とディレクトリエントリを保持する |
| `ShellDestinationCommand` | ピン留め項目1件に対応するコマンド。Shell Linkの実行と、ローカルパス向けの操作を提供する |

## 処理の流れ

1. Providerは設定が有効な場合、`%APPDATA%\Microsoft\Windows\Recent\AutomaticDestinations`を監視する。
2. 起動時およびディレクトリ更新時に、更新日時が現在から90日以内のファイルを読み込む。更新通知を受けてから250ミリ秒待って再読み込みする。
3. `AutomaticDestinationsFile`はOLE Compound Fileから`DestList`ストリームを読み、ピン留め状態と項目番号を取得する。
4. 項目番号を大文字の16進数表記にした名前でShell Linkストリームを探し、リンクデータと表示名を取り出す。
5. Providerは読み込んだ項目を表示名で検索できる一時コマンドとして登録する。

## ファイル形式の解析

### OLE Compound File

`OleCfFile`はOLE Compound Fileのヘッダー、MSAT、SAT、ディレクトリ、SSATを解析し、通常ストリームまたはミニストリームのデータを返します。ストリームのセクター連鎖をたどる際には、循環や範囲外参照を検査します。

読み込み経路はディレクトリエントリのサイズとヘッダーの`MinimumStandardStreamSize`で選択します。ファイル終端を越えたストリームは、取得できた範囲までを返します。解析に必要なヘッダーがない場合や形式が不正な場合は`OleCfParseException`を送出します。

### DestListとShell Link

- `DestList`のヘッダーからバージョン、項目数、ピン留め項目数を読み、各項目から項目番号、ピン留め状態、パスを読み取る。
- バージョン1では固定部分の後にパスが続く形式を扱う。それ以外のバージョンでは後続バージョン用の固定部分とプロパティストア長を扱う。
- `PinStatus`が`-1`の項目はピン留めされていないものとして除外する。
- 表示名はShell Linkの`System.Title`、Shell Linkの説明、`DestList`に格納されたパスの順に採用する。
- 表示名が`knownfolder:`形式の場合、Shellから表示名を取得できれば置き換える。解決できない場合は元の文字列を使う。
- Shell Linkに含まれるファイル属性から、リンク先がディレクトリかどうかを判定する。

## 更新・設定

- 設定キー: `Soyokaze:UseShellDestinations`
- 既定値: `true`
- 設定画面: 「拡張機能」内の「ピン留めした項目を表示する」
- 機能を無効にすると、ディレクトリ監視を解除して読み込み済みの項目を破棄する。
- 読み込み対象のAutomatic Destinationsファイルは、更新日時が90日以内のものに限る。

## コマンドの動作

| 操作 | 動作 |
|---|---|
| `Enter` | Shell Linkを一時`.lnk`ファイルとして書き出し、通常の関連付けで開く |
| `Ctrl+Enter` | ローカルパスの項目があるフォルダーを開く |
| `Alt+Enter` | ローカルパスの項目のプロパティを表示する |

`Ctrl+Enter`と`Alt+Enter`はドライブパスまたはUNC形式のローカルパスでのみ利用できます。URLなどの項目には追加操作を表示しません。

## 制約とエラー処理

- OLE Compound Fileのビッグエンディアン形式は未対応。
- ファイル単位の解析に失敗した場合、そのファイルは候補の読み込み対象にならない。
- DestList内の個別項目や対応するShell Linkが不正、または見つからない場合、その項目をスキップする。
- COMを利用できない場合はShell Linkからの表示名解決を行わず、DestListに格納されたパスを表示名として使用する。
- 更新日時が90日より古いファイルは読み込まれない。

## テスト

関連テストは[`AutomaticDestinationsFileTest.cpp`](../../../../tests/testcode/soyokaze/commands/shell_destinations/AutomaticDestinationsFileTest.cpp)です。短いファイルの読み込み失敗、Shell Linkのタイトル／説明を使った表示名決定、Known Folder形式の解決と未解決時のフォールバックを確認します。
