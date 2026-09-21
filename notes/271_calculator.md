# 271 単位付き計算機能

## 概要

Calculator候補に、単位付きの式を評価する機能を追加した。

この機能は、既存の`adhoc-command/unit-converter`にある単位変換コマンドとは別の機能である。
Calculator候補の評価処理から、単位付きの式を入力した場合にMathyPadを呼び出して結果を表示する。

標準電卓機能と単位付き計算機能は独立して有効・無効を設定できる。

## 関連ファイル

- `src/soyokaze/commands/calculator/Calculator.h`
- `src/soyokaze/commands/calculator/Calculator.cpp`
- `src/soyokaze/commands/calculator/CalculatorAdhocCommandProvider.cpp`
- `src/soyokaze/settingwindow/ExtensionSettingDialog.cpp`
- `src/soyokaze/Soyokaze.rc`
- `src/soyokaze/resource.h`
- `tests/testcode/soyokaze/commands/calculator/CalculatorTest.cpp`

## 設定値

設定画面の値は以下のキーへ保存する。

- `Calculator:Enable`
  - Pythonを利用する標準電卓機能の有効・無効
- `Calculator:IsUseUnitConverter`
  - 単位付き計算機能の有効・無効

`Calculator:IsUseUnitConverter`という名前は既存の設定名との互換性を維持するために使用している。
この設定は`Calculator:Enable`とは独立しているため、標準電卓を無効にして単位付き計算機能だけを有効にできる。

設定変更時は`CalculatorAdhocCommandProvider`が`Calculator`へ両方の設定を反映する。
アプリケーション起動後の候補準備時にも現在の設定を読み直す。

## 評価処理

`Calculator::Evaluate`は、以下の順序で式を評価する。

```text
標準電卓機能が有効
    -> Pythonによる標準電卓で評価
    -> 成功した場合は結果を返す

標準電卓機能が無効、または標準電卓で評価できない
    -> 単位付き計算機能で評価
    -> 成功した場合は結果を返す
```

標準電卓で評価に成功した場合は、単位付き計算機能を呼び出さない。
標準電卓が無効な場合や、入力が標準電卓で扱えない場合は、単位付き計算機能へフォールバックする。

単位付き計算機能も無効、または評価に失敗した場合は、`Calculator::Evaluate`が`false`を返す。

`CalculatorAdhocCommandProvider`は評価に失敗した式を候補として追加しない。
評価結果が整数文字列の場合だけ、従来どおり10進数・16進数・8進数・2進数の候補を追加する。
単位付き計算の結果が整数以外の場合は、10進数用の結果だけを追加する。

## MathyPad DLL連携

`Calculator::PImpl::InitializeMathypad`で、アプリケーションのモジュールファイルディレクトリにある`mathypad.dll`を遅延ロードする。
DLLは最初に単位付き計算を実行するときにロードし、一度初期化を試みた後は同じ`Calculator`インスタンスで再試行しない。

DLLから以下のAPIを取得する。

- `mathypad_evaluate`
- `mathypad_free_string`

APIのどちらか一方でも取得できない場合は初期化に失敗したものとしてDLLを解放し、単位付き計算を利用不可とする。

`mathypad_evaluate`の関数定義は以下のとおりである。

```cpp
char* mathypad_evaluate(const char* expression);
```

返却された文字列は`mathypad_free_string`で解放する。
解放前に`std::string`へコピーし、その後にSoyokaze側の文字列へ変換する。

`Calculator`の破棄時には、`FreeLibrary`でDLLを解放する。

## 文字コード

MathyPad APIの引数と返却値はUTF-8として扱う。

1. `CString`の入力をUTF-8へ変換する
2. UTF-8文字列を`mathypad_evaluate`へ渡す
3. 返却されたUTF-8文字列をコピーする
4. `mathypad_free_string`で返却バッファを解放する
5. Soyokaze側の文字列へ変換する

MathyPadが`nullptr`を返した場合は評価失敗とする。

## 対応単位

MathyPadによる単位付き計算では、以下の単位を扱う。

### 時間

- ナノ秒
- ミリ秒
- 秒
- 分
- 時
- 日
- 週
- 月
- 四半期
- 年

### ビット単位

- `Bit`、`Kb`、`Mb`、`Gb`、`Tb`、`Pb`、`Eb`
- `Kib`、`Mib`、`Gib`、`Tib`、`Pib`、`Eib`

### バイト単位

- `Byte`、`KB`、`MB`、`GB`、`TB`、`PB`、`EB`
- `KiB`、`MiB`、`GiB`、`TiB`、`PiB`、`EiB`

### パーセント

- `%`

## テスト

`CalculatorTest`では、標準電卓機能と単位付き計算機能の両方を無効にした場合に、評価が失敗することを確認する。

MathyPad DLLの実体に依存する評価は、テスト環境へDLLを配置する必要があるため、通常の単体テストでは直接検証していない。
DLLを使用する評価を追加でテストする場合は、DLLの配置、APIのエクスポート、返却文字列の解放まで確認する必要がある。

## 実装上の注意点

- 標準電卓機能と単位付き計算機能の設定を同じ条件として扱わない
- `mathypad.dll`のロード失敗をアプリケーション全体の初期化失敗にしない
- MathyPadが返した文字列をSoyokaze側で解放しない
- `mathypad_free_string`による解放前に返却内容をコピーする
- DLL APIの取得に失敗した場合は、取得済みのハンドルと関数ポインタをすべて破棄する
- 単位付き計算結果に対して、整数用の基数変換候補を無条件に追加しない
