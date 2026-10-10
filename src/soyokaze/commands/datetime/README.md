# datetime

時間差の計算と、日時の候補表示を行うコマンド群。旧 `timespan` を改称し、日時の機能を追加した。

## 機能

- 時間差: `HH:MM-HH:MM` の入力で、時間/分/秒の3候補を表示する(プレフィックス不要)
- 日数オフセット: `N day(s) later|ago`、`N日後|前` の入力で、現在時刻から `N` 日ずらした基準日時を作り、プレフィックス入力時と同じ4候補を表示する(プレフィックス不要、上限36500日)
- 候補の生成は `MakeDateTimeCandidates(基準日時)` に共通化している。プレフィックス入力は日数0、オフセット入力は解析した日数を渡す (`AddDateTimeCandidates`)
- プレフィックス: 設定されたプレフィックス(既定値 `date`)のみを入力すると、日時/日付/時刻/曜日を別候補として表示する
- 有効/無効は「拡張機能」の `時刻と日付` 設定(`Soyokaze:IsEnableDateTime`)に従う。無効な場合は何も表示しない

## 設定

| キー | 型 | 既定値 | 内容 |
|---|---|---|---|
| `Soyokaze:IsEnableDateTime` | bool | true | 日時関連の機能の有効/無効 |
| `Soyokaze:PrefixDateTime` | string | `date` | 現在日時を表示するプレフィックス |

設定の読み込みは `AppPreference` から行い、`AppPreferenceListenerIF` の `OnAppPreferenceUpdated` で再取得する。

## クラス

- `DateTimeCommandProvider`: 入力を解析し、候補を生成する。`ParseTimeSpan`、`ParseDayOffset`、`FormatDateTime` は静的関数としてテストしやすくしている
- `DateTimeCommand`: 時間差の結果(時間/分/秒)を表示するコマンド
- `DateTimeValueCommand`: 日時の文字列を表示し、実行時にクリップボードへコピーするコマンド

## 注意点

- 候補の衝突チェックは行っていない。同じプレフィックスを他のコマンドに設定した場合は、両方の候補が表示される
- 電卓(`Calculator`)は `later`/`ago` を含む入力を評価しない。日数オフセットとの重複を避けるため
- テストは `tests/testcode/soyokaze/commands/datetime/DateTimeCommandProviderTest.cpp` を参照
