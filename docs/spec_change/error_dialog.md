# エラーダイアログの表示方法の変更

# 概要・目的

現在、main.qmlのmessageDialogで表示しているエラーメッセージは画面全体を占有する。
それが良いケースもあるが、サービス側の不調でエラーが頻度が上がる場合は煩わしいケースもある。
そのため、notificationLayoutで表示しているoperationProgressManagerのように画面右下を基準に積み上げてコンパクトに表示する。

# 仕様

- 以下の2つのパーツで構成される。
  - メッセージの管理機能（類似機能:app/qml/parts/OperationProgressManager.qml）
  - 個々のメッセージを表示するフレーム（類似機能:app/qml/parts/OperationProgressFrame.qml）
- 管理機能について
  - レイアウトについて
    - OperationProgressManager.qmlのようにColumnLayoutを使用し、個々のフレームはRepeaterでロードする。
    - main.qmlにおいてoperationProgressManagerの上に追加する。
    - 新しいものを下に追加していく。
  - 個々のフレームを追加するメソッドを外部に提供する。
  - フレームは最大5個保持し、6個目を追加するとき一番古いものを削除する。
  - クリックしたフレームの内容を外部に伝えるシグナルを実装する。
    - シグナルを受けたmain.qmlは従来版のmessageDialogを使用して表示する。
- フレームについて
  - レイアウトについて
    - Frameを使用し、タイトルと本文を2行で表示する。
    - 本文は改行せず1行で表示し、はみ出る場合はelideで処理する。
    - 横幅は「300 * AdjustedValues.ratio」を最大とする。
  - フレームをクリックしたときの動作について。
    - 管理機能のシグナルを使用して外部にクリックしたフレームのタイトルと本文を伝える。
    - フレームは削除する。
  - ×ボタンでFrameを直接削除できる。
    - app/qml/controls/IconButton.qmlを使用する。
    - app/qml/images/close.pngを使用する。
