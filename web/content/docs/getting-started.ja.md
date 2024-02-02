---
title: はじめかた
weight: 10
description: マルチカラム対応Blueskyクライアント
---

# はじめかた

## 入手

[トップページ](/)よりzipファイルをダウンロードします。

使用する環境に合わせて選択してください。ファイル名は下記のとおりです。

- Windows用 : `hagoromo_x.y.z_windows.zip`
- Linux用 : `hagoromo_x.y.z_linux.zip`
- Mac用 : `hagoromo_x.y.z_mac.zip`


## インストールと起動

ダウンロードしたzipファイルを任意のフォルダに解凍して下記のファイルを実行してください。

- Windows : `Hagoromo.exe`
- Linux : `Hagoromo.sh`
- Mac : `Hagoromo.app`

Macの場合は[開発元が未確認のMacアプリを開く](https://support.apple.com/ja-jp/guide/mac-help/mh40616/mac)を参考にしてください。

## アカウントの追加

{{< figure src="../images/add_account.ja.png" >}}

1. サイドバーからアカウント管理のダイアログを開きます。
2. 追加ボタンを押します。

{{< figure src="../images/login.ja.png" >}}

1. 以下の項目を入力します。
   - サービス  
     `https://bsky.social`を入力します。  
     基本的に変更不要ですが、他のPDSも使用できます。
   - ID  
     下記のいずれかを入力します。
     - ハンドル
     - Eメール
     - DID
   - パスワード  
     [公式サイト](https://bsky.app/settings/app-passwords)で作成したアプリパスワードを入力してください。  
     本物のパスワードは使用しないでください。
2. `ログイン`ボタンを押します。

## カラムの追加

{{< figure src="../images/add_column.ja.png" >}}

アカウントを選択してからカラムタイプから追加したい項目を選んで追加ボタンを押します。

カラムタイプには下記が表示されます。

- 標準フィード
  - タイムライン（Following）と通知が表示されます。
- カスタムフィード
  - 保存したカスタムフィードが表示されます（Pin to Homeではありません）。
- リスト
  - 自身のリストが表示されます（ミュートやブロックをするリストではありません）。

## 投稿

{{< figure src="../images/post.ja.png" >}}

1. 鉛筆マークのボタンを押します（または`n`キー）
2. 素敵な出来事を入力します（300文字まで）
3. ポストボタンを押します
