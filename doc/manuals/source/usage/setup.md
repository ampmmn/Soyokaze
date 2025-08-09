# Installation

## Requirements

{{ project}}はWindows環境で動作する。
以下の環境で動作確認済。

- Windows11 64bit
- Windows10 64bit


## Doanload

```{only} soyokaze
GitHubの[最新版リリースページ](https://github.com/ampmmn/Soyokaze/releases/latest)から入手することができる  
Assets欄にある`.7z`形式のファイルが実行ファイル一式をダウンロードする
```

```{only} not soyokaze
下記のリポジトリで実行ファイル一式を管理しているので、`git clone`で実行ファイルを取得する

{{distribution_url}}
```

----

## Installation

```{only} soyokaze
1. zip(7z)をファイルを展開して任意のフォルダに展開する
1. 展開先フォルダにある{{ project_lower }}.exeを実行する
1. 設定はユーザーフォルダ(たいていは C:/Users/(ユーザー名))直下の .{{ project_lower }} フォルダに保存される  
初回実行時に .{{project_lower}}フォルダを作成する
```

```{only} not soyokaze
任意のパスで`git clone`すればインストール完了
```

----

## Uninstallation

アンインストーラはないため、以下すべてを手動で行う。

1. タスクトレイ上のコンテキストメニュー-`アプリケーションの設定`-`基本`-`ショートカット設定`を表示し、`作成したショートカットをすべてを削除する`を押下する
1. アプリを終了する
1. インストール時に展開したフォルダごと削除する
1. [設定フォルダ](#configuration-directory) (C:/Users/(ユーザー名)/.{{project_lower}})を削除する

----

## Update Procedure

```{only} soyokaze
1. 本アプリを実行している場合は終了する
1. 実行ファイルを置いたフォルダ内のファイルをすべて上書きする
```

```{only} not soyokaze
アプリを終了して、`git pull`すれば最新版に更新できる。  
もし、コンフリクトが生じた場合は、`git fetch` → `git reset --hard origin:main` でOK
```

----

## Configuration Directory

アプリ初回起動時に ユーザーフォルダ直下(通常は `C:\Users\<ユーザー名>`)に .{{project_lower}} フォルダを作成し、このフォルダ内に設定ファイル一式を保存する。

### Portable Mode

exeと同じフォルダ階層に`profile`というフォルダが存在する場合、ポータブル版として動作する。  
この場合、設定ファイル一式を`profile`フォルダ内に作成する。

