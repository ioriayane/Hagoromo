---
title: カスタムする
weight: 100
---

# カスタムする

## 見た目を変更する

- 色を変更する
- 表示サイズを変更する
- フォントを変更する


## フィードの動きを変更する

## レイアウトを変更する

{{ $image := resources.Get "images/screenshot.jpg" }}
<img src="{{ $image.RelPermalink }}" width="{{ $image.Width }}" height="{{ $image.Height }}">

{{< figure src="$image.RelPermalink" title="Steve Francia" >}}
