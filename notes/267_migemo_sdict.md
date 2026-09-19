# C/Migemoのsdict対応

## 概要

Soyokazeでは、ローマ字入力を日本語の正規表現に変換するためにC/Migemoを使用している。
C/Migemoは通常辞書を読み込んで検索に使用できるほか、通常辞書をsdict形式へ変換して保存する機能を持つ。

sdict形式は通常辞書よりも読み込みが高速になるため、2回目以降の起動では保存済みのsdictを優先して使用する。

## 関連するファイルと初期化条件

主な実装は以下のファイルにある。

- `src/soyokaze/matcher/Migemo.h`
- `src/soyokaze/matcher/Migemo.cpp`
- `src/soyokaze/matcher/PartialMatchPattern.cpp`

`PartialMatchPattern`の生成時にMigemo設定を確認し、以下の条件を満たす場合に辞書を開く。

- Migemoを使用する設定が有効である
- 通常辞書が存在する

通常辞書のパスは、アプリケーションのモジュールファイルディレクトリを基準にした以下のパスである。

```text
dict\utf-8\migemo-dict
```

辞書を開けた場合は`Migemo::IsInitialized()`が`true`になり、検索時にMigemoを使用する。

## DLLとC/Migemo API

`Migemo`の生成時に、アプリケーションのモジュールファイルディレクトリにある`migemo.dll`をロードする。
DLLから以下のAPIを取得する。

- `migemo_open`
- `migemo_open_sdict`
- `migemo_load`
- `migemo_switch_sdict`
- `migemo_save_sdict`
- `migemo_close`
- `migemo_query`
- `migemo_release`
- `migemo_setproc_int2char`

通常辞書用の基本APIである`migemo_open`と`migemo_load`を取得できない場合、`Migemo::Open`は失敗する。

以下の3つのsdict対応APIをすべて取得できる場合は、sdict対応経路を使用する。

- `migemo_open_sdict`
- `migemo_switch_sdict`
- `migemo_save_sdict`

sdict対応APIが不足している場合は、従来どおり通常辞書だけを使用する。

## 通常辞書

通常辞書を使用する経路は`PImpl::OpenMDict`が担当する。

1. 指定された通常辞書の存在を確認する
2. パスをC/Migemoが扱う文字コードへ変換する
3. `migemo_open`で辞書を開く
4. `migemo_setproc_int2char`で正規表現のメタ文字をエスケープする処理を設定する
5. 辞書を開けた場合は成功を返す

辞書が存在しない場合、または`migemo_open`が失敗した場合は失敗を返す。

## sdictキャッシュ

sdictは、アプリケーションの`Path::APPDIRPERMACHINE`を基準にした以下のディレクトリへ保存する。

```text
migemo-sdict\
```

保存されるファイルは以下の5つである。

```text
migemo-sdict\migemo-sdict
migemo-sdict\han2zen.dat
migemo-sdict\hira2kata.dat
migemo-sdict\roma2hira.dat
migemo-sdict\zen2han.dat
```

sdict本体だけではなく、補助辞書4ファイルも揃っている場合にキャッシュを有効とみなす。
どれか1つでも不足している場合は、保存済みsdictを使用しない。

## sdictの読み込み

sdictを使用する経路は`PImpl::OpenSDict`が担当する。

### 保存済みsdictがある場合

sdict本体と補助辞書4ファイルがすべて存在する場合、`migemo_open_sdict`でsdictを開く。

sdictを開けた場合は、通常辞書を開いたり、sdictを再生成したりせず、そのまま使用する。

sdictを開けなかった場合は、通常辞書の読み込みを試みる。

### 保存済みsdictがない場合

sdict本体または補助辞書が不足している場合は、通常辞書を`migemo_open`で開く。

通常辞書も開けなかった場合は、辞書の初期化に失敗する。

通常辞書を開けた場合は、現在のMigemoオブジェクトに対してsdictへの変換を試みる。

## sdictの生成と保存

通常辞書からsdictへ変換する場合は、以下の順序で処理する。

1. `migemo_switch_sdict`を呼び出す
2. sdict保存先ディレクトリを作成する
3. 通常辞書と同じディレクトリにある補助辞書4ファイルを保存先へコピーする
4. 補助辞書4ファイルのコピーがすべて成功した場合、`migemo_save_sdict`でsdict本体を保存する

`migemo_switch_sdict`の戻り値が`0`の場合は変換失敗とみなし、sdictの保存を行わない。
変換に失敗しても、メモリ上の通常辞書はそのまま使用する。

保存先ディレクトリの作成、補助辞書ファイルの存在確認、ファイルコピーのいずれかに失敗した場合も、sdictの保存を中止する。
この場合も、メモリ上で開いている通常辞書による検索は継続する。

補助辞書のコピーには上書き指定を使用する。
コピー対象のファイルが不足している場合は、sdict本体だけを保存することはない。

## `Migemo::Open`の呼び出し

`Migemo::Open`は、すでにMigemoオブジェクトが生成されている場合、辞書を再読み込みせず`true`を返す。
これにより、同じ`Migemo`インスタンスに対する2回目以降の呼び出しで辞書を再度開くことを防ぐ。

初回呼び出しではsdict対応APIの有無を確認し、以下のように経路を選択する。

```text
sdict対応APIがすべて存在する
    -> OpenSDict

sdict対応APIが不足している
    -> OpenMDict
```

選択した経路が成功した場合に`mIsDictLoaded`を`true`にする。

## 辞書の解放

`Migemo::Close`は、辞書オブジェクトが存在する場合に`migemo_close`を呼び出して解放する。
解放後は以下の状態に戻る。

- `mMigemoObj`は`nullptr`
- `mIsDictLoaded`は`false`

その後に`Migemo::Open`を呼び出した場合は、再び辞書の読み込み処理を行う。

## 検索時の利用

`PartialMatchPattern`は入力されたキーワードをトークンに分割し、Migemoを使用する条件を満たすトークンに対して`Migemo::Query`を呼び出す。

以下の場合はMigemoを使用しない。

- Migemoが初期化されていない
- 入力文字列が空である
- 1文字だけの入力が母音ではない

Migemoを使用する場合は、`migemo_query`でローマ字入力を正規表現へ変換する。
生成した正規表現は、通常の検索用と前方一致用のRE2パターンに使用する。

## 実装上の注意点

- sdict本体と補助辞書4ファイルは一体で扱う
- 補助辞書が不足したキャッシュは使用しない
- sdictの読み込みに失敗した場合は通常辞書へフォールバックする
- sdict生成や保存に失敗しても、メモリ上の通常辞書による検索は継続する
- sdict対応APIがない古い`migemo.dll`でも、通常辞書経路で動作できるようにする
- 同一`Migemo`インスタンスで辞書を再読み込みする場合は、先に`Close`を呼び出す
