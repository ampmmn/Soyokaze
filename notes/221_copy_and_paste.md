# 221実装案についてのメモ

## 概要

キーワードマネージャでCtrl-C,Ctrl-Vキー操作を使ってクリップボードにコマンドデータのコピー&ペーストをできるようにする。  
リモートデスクトップ経由で一方のPCにあるコマンドをもう一方のPC側のアプリ側にコマンドを移せるようにするための機能

## 詳細

以下の機能をもつクラスを新設(CommandClipboardTransfer)
- 起動時にアプリケーション専用のクリップボード形式を登録しておく(RegisterClipboardFormat)
- クリップボードのデータストリームからCommandEntryIFインスタンスを生成する
- CommandEntryIFの内容をもとにクリップボードに着込む
- シングルトン

- キーワードマネージャでCtrl-Cを押したらコピー処理を開始する
- キーワードマネージャでCtrl-Vを押したらペースト処理を開始する

- CommandRepositoryがProviderを保持しているが、分離してProviderを管理する別クラスを用意する
  - CommandProviderRepositoryを新設

CommandRepositoryはCommandProviderRepositoryからProvider一覧を得る

## 処理別

### 初期化

起動時にAppクラスでCommandClipboardTransferを初期化する

- CommandClipboardTransferはクリップボード形式の登録を行う

```plantuml

participant App
participant CommandClipboardTransfer
participant System

-> App ++ : InitInstance()
  App -> CommandClipboardTransfer ++ : Initialize()
    CommandClipboardTransfer -> System : RegisterClipboardFormat()
    CommandClipboardTransfer <-- System
  return

  ... (その後の処理) ...

```

### コピー処理

キーワードマネージャでコマンドを選択した状態でCtrl-Cを押したらコピー処理を実行する

- 一時的な領域に出力するための新しいCommandEntryIFを作成する
  - CommandClipboardTransferがインスタンスを生成する
    - 専用のCommandEntryIFを実装する

- コマンドにCommandEntryIFを渡してSaveをよぶ
  - Saveのなかでクリップボードにコピーするためのデータストリームを生成する
- CommandClipboardTransfer側にCommandEntryIFをもどしてクリップボードに登録してもらう

```plantuml

participant KeywordManagerDialog as dlg
participant "コピー対象のコマンド" as cmd
participant CommandClipboardTransfer
participant CommandJSONEntry as entry
participant System

-> dlg ++ : Ctrl-C押下

    dlg  -> cmd ++ : GetName()
    return name

    dlg -> CommandClipboardTransfer ++ : NewEntry(name)
        CommandClipboardTransfer -> entry  ++ : 作成
        return entry
    return entry

    dlg -> cmd ++ : Save(entry)
        cmd -> entry ++ : データを書き込み
            entry -> entry : JSONデータを生成
        return
    return

    dlg -> CommandClipboardTransfer ++ : SendEntry(entry)
        CommandClipboardTransfer -> entry ++ : DumpRawData()
        return JSONデータ

        CommandClipboardTransfer -> System : JSONデータをクリップボードに書き込み
        CommandClipboardTransfer <-- System
    return
return

```

### ペースト処理

キーワードマネージャでコマンドを選択した状態でCtrl-Vを押したらペースト処理を実行する

- CommandClipboardTransferにペースト可能か問い合わせる(現在のクリップボードのデータ形式が登録したものかどうか)

- 登録したものであるなら、データを取得する

- 取得したら、そのデータストリームを使って、CommandEntryIFインスタンスの生成を試みる

- 生成できたら、それをキーワードマネージャ側に返す

- CommandEntryIFの名前をみて、同名のコマンドが既存かどうかを確かめる
  - 同名コマンドが既存の場合、ユーザに対応を選択させる(上書き or リネームで取り込み)

- キーワードマネージャはCommandProviderRepositoryからProviderの一覧を取得し、CommandEntryIFからコマンド生成を試みる

- コマンド生成できたら、それを登録する

```plantuml

participant KeywordManagerDialog as dlg
participant "ペーストされるコマンド" as cmd
participant CommandProviderRepository as repos
participant CommandProvider as provider
participant CommandRepository as cmdRepos
participant CommandClipboardTransfer as transfer
participant CommandJSONEntry as entry
participant System

-> dlg ++ : Ctrl-V押下

    dlg -> transfer ++ : ReceiveEntry()
        transfer -> System : クリップボードの値を取得
        transfer <-- System : JSONデータ

        transfer -> entry ++ : クリップボードの値(JSONデータ)からCommandJSONEntry生成を試みる
        return 
    return entry

    dlg -> repos ++ : EnumProviders()
    return providers

    loop for each providers
        dlg -> provider ++ : LoadFrom(entry)
            provider -> cmd ++ : LoadFrom(entry)
                cmd -> entry ++ : 設定値を参照
                return
            return
        return result

        note over provider : LoadFromが成功するまで繰り返し
    end loop

    alt LoadFromに成功
    
        note over cmdRepos : コマンドを登録
        dlg -> cmdRepos ++ : RegisterCommand(cmd)
        return
    end alt

return

```


### 疑似コード

ペーストしたとき処理のPython風コードできじゅつする

```
for provider in providers:

  if provider->LoadFrom(entry, &newCmd) == False:
    continue

  # 登録したコマンドの有無を確認
  if cmdRepos->HasCommand(newCmd->GetName()) == False:
    # 重複する名前がないでそのまま登録
    cmdRepos->RegisterCommand(newCmd)
  else:

    # ToDo: 既にコマンドが登録するので名前を変えるか上書きをユーザに選択させる(確認UIをここで表示)

    if should_overwrite:
      # 既存のコマンドを削除
      cmdRepos->Unregister(orgCmd)
    else:
      # インポートするコマンドの名前を重複しない名前に変更する
      newCmd->SetName(newName)

    # インポートするコマンドを登録する
    cmdRepos->RegisterCommand(newCmd)
    break

```

## メモ

- FIXME : キーワードマネージャで編集したら、いきなり本物が更新される(キャンセルで元に戻す)ではなく、キーワードマネージャでOKしたときに本物に反映する、の形にできないのか?

