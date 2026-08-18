# Installation

## Requirements

{{ project}}はWindows環境で動作する。
以下の環境で動作確認済。

- Windows11 64bit
- Windows10 64bit


## Download

```{only} soyokaze
GitHubの[最新版リリースページ](https://github.com/ampmmn/Soyokaze/releases/latest)からダウンロードします。

Assets欄にある`.zip`形式のファイルが、実行ファイル一式をまとめたアーカイブです。ダウンロードしたファイルは、7-Zipなどの解凍ソフトで展開してください。
```

```{only} not soyokaze
下記のリポジトリで実行ファイル一式を管理しているので、`git clone`で実行ファイルを取得する

{{distribution_url}}
```

----

## Installation

```{only} soyokaze
1. ダウンロードした`.zip`ファイルを、任意のフォルダに展開する
1. 展開したフォルダを開き、`{{ project_lower }}.exe`を実行する
1. 初回起動時に、設定ファイルの保存先が自動的に作成される

本アプリは、展開したフォルダから実行できます。スタートメニューなどから起動したい場合は、必要に応じてショートカットを作成してください。

初期状態では、設定はユーザーフォルダ（通常は`C:\Users\<ユーザー名>`）直下の`.{{ project_lower }}`フォルダに保存されます。
```

```{only} not soyokaze
任意のパスで`git clone`すればインストール完了
```

----

## Uninstallation

アンインストーラはないため、以下すべてを手動で行う。

1. 作成したショートカットがある場合は、タスクトレイのメニューから`アプリケーションの設定`>`基本`>`ショートカット設定`を開き、ショートカットを削除する
1. アプリを終了する
1. インストール時に展開したフォルダを削除する
1. 設定フォルダ（通常は`C:\Users\<ユーザー名>\.{{ project_lower }}`）を削除する

----

## Update Procedure

```{only} soyokaze
1. 新しいバージョンのアーカイブをダウンロードして展開する
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

