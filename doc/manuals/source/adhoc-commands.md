# Implicitly Executable Features

利用者があらかじめコマンドを登録しておき、
入力画面からキーワード入力して実行するのが本アプリのメイン機能であるが、
コマンドとして登録していなくても使える機能がある。

```{toctree}
---
maxdepth: 1
---
adhoc-command/activate-window.md
adhoc-command/base64-decode.md
adhoc-command/bookmarks.md
adhoc-command/calclator.md
adhoc-command/char-escape.md
adhoc-command/clipboard-history.md
adhoc-command/colorcode-converter.md
adhoc-command/controlpanel.md
adhoc-command/env.md
adhoc-command/eraname-converter.md
adhoc-command/everything.md
adhoc-command/file-pathconvert.md
adhoc-command/git-bash-pashconvert.md
adhoc-command/history.md
adhoc-command/ipaddress.md
adhoc-command/mailto.md
adhoc-command/mmcsnapin.md
adhoc-command/onenote.md
adhoc-command/pathfind.md
adhoc-command/recentfiles.md
adhoc-command/slide-jump.md
adhoc-command/startmenu.md
adhoc-command/timespan.md
adhoc-command/unit-converter.md
adhoc-command/uri-decode.md
adhoc-command/uwpapplication.md
adhoc-command/vmware-player-mru.md
adhoc-command/webhistory.md
adhoc-command/windowssettings.md
adhoc-command/winscp.md
adhoc-command/worksheet-jump.md
adhoc-command/vscode.md
```

## Other Features

### Logging

ログファイルに出力するメッセージのレベルを選択する。

ログファイルは[設定ファイルの保存先](#設定ファイルの保存先)フォルダ内に`<アプリ名>.log`というファイル名で出力する。  
アプリケーション設定の[その他](#その他)で出力対象とするログレベルを設定することができる。

### Continuous Worktime Warning

休憩を挟まずに連続して作業を続けていた場合に、その旨をトースト通知する機能。  
休憩を促す目的。  
スクリーンロックを休憩(離席)とみなし、連続稼働時間をリセットする。

アプリケーション設定の[その他](#その他)で有効化することにより利用できる。  

