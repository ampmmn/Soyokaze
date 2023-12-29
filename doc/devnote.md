# 開発時の各種メモ

## アプリケーション設定画面(ページ)を追加する手順

- ダイアログリソースを用意する
  - 既存のほかぺージのリソースをコピーするとよい
- SettingPage派生クラスを作成する

- Pageのインスタンスを作ってページツリーに登録するのはgui/SettingDialog.cpp
  - ページ追加のたびにSettingDialogを修正するのはいまいちなので、Page側から登録するような仕組みを整えたいが・・

- 設定を読み込むのはPage::OnEnterSettings
- 設定を保存するのはPage::OnOK

- 保存された設定はAppPreferenceから読み取る
  - 設定を追加したらAppPreferenceにGetterメソッドを追加する

- 設定変更を検知したい場合はAppPreferenceListenerIF派生クラスをつくってリスナー登録する

- コマンド側で変更検知したいときは、ProviderのPImplでやることがおおい

