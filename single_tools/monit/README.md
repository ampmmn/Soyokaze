# montrol

`montrol`(Monitor Control) は、PCに接続されたモニターの情報を表示したり、入力ソースや輝度を変更したりするWindows用コマンドラインツールです。

## 前提・注意事項

- 入力ソースや輝度の変更には、モニターがDDC/CIに対応している必要があります。
- DDC/CIに対応していないモニターも一覧には表示されますが、情報取得や操作に失敗する場合があります。
- モニター番号は実行時の列挙順に割り当てられます。実行するたびに番号が変わる場合があります。
- モニターや接続方法によっては、入力ソースの切替や輝度変更に対応していない場合があります。

## 基本書式

```text
monit.exe <command> [options]
```

利用できるサブコマンドは次のとおりです。

| サブコマンド | 説明 |
| --- | --- |
| `query` | モニターの情報を表示する |
| `switch` | モニターの入力ソースを切り替える |
| `brightness` | モニターの輝度を変更する |

## ヘルプとバージョン

```text
monit.exe --help
monit.exe -h
monit.exe --version
monit.exe -v
```

サブコマンドごとのヘルプも表示できます。

```text
monit.exe query --help
monit.exe switch --help
monit.exe brightness --help
```

## query

接続されているモニターの情報を表示します。

### 書式

```text
monit.exe query <all|index> [options]
```

`<index>` は1から始まるモニター番号です。

| オプション | 説明 |
| --- | --- |
| `-s`, `--source` | 対応している入力ソースを表示する |
| `-b`, `--brightness` | 現在の輝度を表示する |
| `-j`, `--json` | 結果をJSON形式で表示する |
| `-h`, `--help` | ヘルプを表示する |

### 動作

`all` を指定すると、すべてのモニターを対象にします。

```text
monit.exe query all
```

オプションを指定しない場合、`query all` はモニター一覧だけを表示します。

```text
monit.exe query 1
```

モニター番号を指定し、オプションを指定しない場合は、そのモニターの輝度と入力ソースも表示します。

入力ソースと輝度は同時に取得できます。

```text
monit.exe query all --source --brightness
```

### 通常出力の例

```text
Detected Monitor Count: 2
[1] : Generic PnP Monitor
  Brightness:
    20/100
  Input Sources:
    1 : VGA1
    4 : DVI2
    15 : DisplayPort1
[2] : Generic PnP Monitor
  Brightness:
    20/100
  Input Sources:
    17 : HDMI1
    18 : HDMI2
    15 : DisplayPort1
    16 : DisplayPort2
```

### JSON出力

`--json` を指定すると、結果をJSON形式で表示します。

```text
monit.exe query all --source --brightness --json
```

出力例:

```json
{
    "count": 2,
    "devices": [
        {
            "index": 1,
            "displayName": "Display Name 1",
            "brightness": [
                50,
                100
            ],
            "inputSources": [
                {
                    "id": 15,
                    "displayName": "DisplayPort1"
                },
                {
                    "id": 17,
                    "displayName": "HDMI1"
                }
            ]
        }
    ]
}
```

JSONでは次の規則で値が出力されます。

- `brightness` は常に出力され、取得対象外または取得失敗時は `null` になります。
- `inputSources` は常に出力され、取得対象外または取得失敗時は空配列になります。
- `displayName` を取得できない場合は空文字列になります。

## switch

モニターの入力ソースを切り替えます。

### 書式

```text
monit.exe switch <index> <input-source> [<index> <input-source> ...]
```

対象にはモニター番号を指定します。`all` は指定できません。

複数のモニターを指定した場合は、左から右の順に処理します。

```text
monit.exe switch 1 HDMI1 2 DisplayPort1
```

入力ソースには、エイリアスまたは0～255のVCP値を指定できます。

```text
monit.exe switch 1 HDMI1
monit.exe switch 1 17
```

エイリアスは大文字と小文字を区別しません。

## brightness

モニターの輝度を変更します。輝度値は0～100の整数で指定します。

### 書式

```text
monit.exe brightness <all|index> <value> [<index> <value> ...]
```

すべてのモニターを同じ輝度にする場合は、`all` を指定します。

```text
monit.exe brightness all 20
```

特定のモニターを変更する場合は、モニター番号を指定します。

```text
monit.exe brightness 1 40
```

複数のモニターに異なる輝度を設定できます。

```text
monit.exe brightness 1 20 2 40
```

`all` とモニター番号の混在はできません。

## 入力ソースエイリアス

| エイリアス | VCP値 |
| --- | ---: |
| `vga1`, `vga` | 1 |
| `dvi1`, `dvi` | 3 |
| `dvi2` | 4 |
| `composite` | 8 |
| `svideo`, `s-video` | 9 |
| `dp1`, `dp`, `displayport` | 15 |
| `dp2` | 16 |
| `hdmi1`, `hdmi` | 17 |
| `hdmi2` | 18 |
| `usbc`, `usb-c`, `typec`, `type-c` | 27 |

標準エイリアスにないモニター固有の入力ソースは、`query` で表示されたVCP値を直接指定できます。値はモニターによって異なるため、まず対応する入力ソースを確認してください。

```text
monit.exe query 1 --source
monit.exe switch 1 <queryで表示されたVCP値>
```

## 終了コード

| 終了コード | 意味 |
| ---: | --- |
| `0` | 正常終了 |
| `1` | モニター情報の取得または操作に失敗 |
| `2` | コマンドや引数の指定が不正 |

複数のモニターを処理する場合、一部の処理に失敗しても後続の処理は継続します。1件以上失敗した場合は、処理完了後に終了コード `1` を返します。

## トラブルシューティング

### モニターが操作できない

モニターがDDC/CIに対応しているか確認してください。また、モニター本体の設定でDDC/CIが無効になっていないか確認してください。

### モニター番号が想定と異なる

次のコマンドで現在の列挙順を確認してください。

```text
monit.exe query all
```

モニター番号は接続状態やWindowsの列挙順によって変わる場合があります。

### 入力ソースが表示されない

モニターから入力ソースのケイパビリティ情報を取得できない場合があります。その場合は `query --source` の結果が空になることがあります。

### 輝度の値が期待と異なる

`monit` の輝度値は0～100に正規化された値です。モニター内部の輝度値の範囲とは異なる場合があります。

## 免責事項

このソフトウェアは無保証で提供されます。

作者は、このソフトウェアの動作、正確性、継続性、特定の目的への適合性を保証しません。
また、このソフトウェアの使用によって生じたモニターの設定変更、表示上の問題、機器や接続環境への影響、その他の損害について、作者は責任を負いません。

DDC/CIの対応状況や動作は、モニター、接続方法、変換アダプターなどの環境によって異なります。
使用前にモニターの設定を確認し、利用者自身の責任で使用してください。
