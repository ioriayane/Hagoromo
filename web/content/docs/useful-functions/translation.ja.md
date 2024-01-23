---
title: 翻訳
weight: 20
description: マルチカラム対応Blueskyクライアント
---

# 翻訳

Google翻訳もしくはDeepLを使用してポストの翻訳ができます。

## Google翻訳を使用（初期状態）

### 翻訳後の言語を設定

{{< figure src="../images/translation_g_settings.ja.png" width="70%" >}}


1. `翻訳後の言語`を選択します。
2. `OK`を押します。

※ `Api Key`は空欄のままにしてください。

### ポストを翻訳

{{< figure src="../images/translation_g_do.ja.png" >}}

1. ポストの表示エリア右下の`…`を押します。
2. `翻訳`を選択します。
3. 既定のブラウザでGoogle翻訳が開きます。

## DeepLを使用

### DeepL APIを設定

{{< figure src="../images/translation_d_settings.ja.png" width="70%" >}}

1. 以下の項目を設定します。
   - Api Url
     - Freeプラン : `https://api-free.deepl.com/v2/translate`
     - Proプラン : `https://api.deepl.com/v2/translate`
   - Api Key
     - DeepLのサイトで確認したKeyを入力します。
   - 翻訳後の言語
     - 言語を選択します。
2. `OK`を押します。

### ポストを翻訳

{{< figure src="../images/translation_d_do.ja.png" >}}

1. ポストの表示エリア右下の`…`を押します。
2. `翻訳`を選択します。
3. `Now translating...`と表示されます。少し待ちます。
4. 翻訳結果が表示されます。
