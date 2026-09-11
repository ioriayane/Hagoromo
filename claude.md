# Hagoromo 開発ガイド

## プロジェクト概要

Hagoromo は、Windows / Ubuntu / macOS 向けのマルチカラム対応 Bluesky クライアントです。
Qt 6 のデスクトップアプリケーションで、C++ と QML で構成されています。
また、Hugo ベースのユーザーマニュアルサイトもこのリポジトリに含まれています。

主な実行時・ビルド依存関係:

- Qt 6.11.0
- CMake 3.19 以上
- Ninja
- OpenSSL 3.6.3
- zlib 1.3.2
- C++11 互換コンパイラ

## リポジトリ構成

- `app/`: アプリ本体、Qt Quick の C++ 実装、QML の画面・コントロール・ダイアログ
- `lib/`: 再利用可能な静的ライブラリ、Bluesky / AT Protocol クライアントコード
- `tests/`: CMake / CTest によるユニットテストと記録済み API レスポンス
- `tools/`: 補助的な C++ ツール
- `1stparty/`: AT Protocol の依存ソース
- `3rdparty/`: OpenSSL / cpp-httplib / zlib のサブモジュール
- `scripts/`: OS 別のビルド・テスト・デプロイ・メンテナンス用スクリプト
- `web/`: Hugo ベースのドキュメントサイト
- `.github/workflows/`: CI とリリースチェック

## 初期セットアップ

ビルド前にサブモジュールを初期化してください:

```bash
git submodule update --init --recursive
```

テンプレートから暗号化用のシードファイルを作成し、ローカル開発環境に合わせて編集します:

```bash
# Unix
cp lib/tools/encryption_seed_template.h lib/tools/encryption_seed.h

# Windows cmd
copy lib\tools\encryption_seed_template.h lib\tools\encryption_seed.h
```

資格情報、実アカウント情報、秘密鍵や暗号化シードをコミットしないでください。

## ビルドとテスト

このリポジトリのスクリプトは `build-hagoromo/` と `deploy-hagoromo/` を生成します。
これらはビルド生成物であり、コミット対象ではありません。

### Windows

Visual Studio 2022 の開発環境、Perl、Qt 6.11.0 を用意し、`cmd.exe` から次を実行します:

```cmd
scripts\build.bat path\to\Qt\6.11.0\msvc2022_64\
```

ユニットテストの実行は、以下のスクリプトが主な検証コマンドです:

```cmd
scripts\unittest.bat path\to\Qt\6.11.0\msvc2022_64\
```

### Linux

必要な依存パッケージを事前に導入します:

```bash
sudo apt-get install zlib1g-dev ninja-build
```

ビルドとテストは次のように実行します:

```bash
./scripts/build.sh linux path/to/Qt/6.11.0/gcc_64/
./scripts/unittest.sh linux path/to/Qt/6.11.0/gcc_64/
```

### macOS

Qt 6.11.0 を用意したうえで、次を実行します:

```bash
./scripts/build.sh mac path/to/Qt/6.11.0/gcc_64/
./scripts/unittest.sh mac path/to/Qt/6.11.0/gcc_64/
```

これらのスクリプトは開発時に Debug テストをビルドします。
リリースビルドは配布向けで、環境固有の Qt / OpenSSL / zlib のランタイムファイルを扱います。

CMake を直接使う場合、`-DHAGOROMO_LAYOUT_TEST=ON` を付けると、アプリ本体ではなく
`app/qml/layout_test_main.qml` を利用してレイアウト確認ができます。

### VS Code + CMake Tools を使う場合

`.vscode/settings.json`(`.gitignore` 対象)に Qt のパスを設定します:

```json
{
    "cmake.configureArgs": [
        "-DCMAKE_PREFIX_PATH=/path/to/Qt/6.11.0/gcc_64",
        "-DCMAKE_MAKE_PROGRAM=/path/to/Qt/Tools/Ninja/ninja"
    ]
}
```

設定後は `CMake: Configure` を実行し、OUTPUT の `CMake/Build` で `CMAKE_PREFIX_PATH` が反映されているか確認してください。

## 開発フロー

1. 通常の変更は `dev` からブランチを切る
2. 変更範囲を限定し、既存の C++ / QML の構成に沿う
3. PR を作る前に、関連するユニットテストと整形チェックを実行する
4. 通常の機能追加や不具合修正は `dev` に、リリース PR は `main` を対象にする
5. リリース PR のタイトルは `Release/vX.Y.Z` 形式にする

### 整形

ソースコードの整形は pre-commit で行います:

```bash
pip install pre-commit
pre-commit install
pre-commit run --all-files
```

設定されているフックは、末尾改行の修正、余分な空白削除、`clang-format` による整形です。
リポジトリの `.clang-format` 設定を利用しているため、無関係なフォーマット変更は避けてください。

### リリース手順

`main` を対象にしたリリース PR では CI(`.github/workflows/main.yml`)が下記の整合性を検証するため、
バージョンを上げる際はまとめて更新してください:

- `app/main.cpp` の `app.setApplicationVersion(...)`
- `web/content/docs/release-note.en.md` / `release-note.ja.md`(`scripts/validate_releasenote.py` で検証)
- `web/layouts/shortcodes/download_link.html`(`scripts/updateweb.py` の生成結果との diff で照合)
- PR タイトルは `Release/vX.Y.Z` 形式(正規表現でチェックされる)

## コード規約

- C++11 互換の機能のみを使う。言語仕様を引き上げる変更はプロジェクト全体の方針が明確な場合に限る
- Qt の所有権、シグナル/スロット、Model/View、`QObject` の設計に沿う
- ネットワーク処理や AT Protocol アクセスは `lib/atprotocol` や既存の operation 系クラスに置く
- UI 表示や対話処理は QML 側に寄せ、データや操作は既存の Qt Quick モデルや登録済みオブジェクト経由で公開する
- `lib/atprotocol` の `lexicons.h` / `lexicons_func*.cpp` は `scripts/defs2struct.py` によるレキシコン自動生成物(`Please do not edit.` と明記)なので直接編集せず、`lexicons/` 配下の定義を更新してスクリプトを再実行する
- ユーザー向けの文言を追加する場合は `.ts` ファイルを手動編集せず、CMake の `update_translations` ターゲット(`qt_add_translations` で生成)経由で `app/Hagoromo_*.ts` / `lib/HagoromoLib_*.ts` を更新する
- プロトコル、モデル、操作の挙動を変える場合は、対象のテストと記録済みレスポンスデータも更新する
- 新規ユニットテストは `tests/<name>_test/`(`tst_*.cpp` + `data/` + `response/` + `.qrc`)の既存構成に倣う
- Windows 開発では CRLF を維持する前提があるため、改行コードの取り扱いに注意する

## 重要な注意事項

- サブモジュールやベンダリングされたコードを、明示的な必要がない限り編集しない
- テストが通るように記録済み API レスポンスをただ書き換えない
- アクセストークン、パスワード、暗号化シード、個人情報をログ出力しない
- 変更前に生成物やデプロイ出力を確認し、必要ないファイルをコミットしない(`build-hagoromo/`, `deploy-hagoromo/`, `3rdparty/build-openssl/`, `openssl/{bin,include,lib,...}`, `zlib/{bin,include,lib}`, `web/public/`, `temp.html` などは `.gitignore` 対象)
- 既存のスクリプトや CMake ターゲットを優先し、独自のビルド方法を作りすぎない

## 検証チェックリスト

変更を完了する前に次を確認してください:

- 差分に無関係なファイルや生成物が混ざっていないか確認する
- 変更に最も近い CTest またはリポジトリ用スクリプトを実行する
- ソース整形に影響する場合は `pre-commit run --all-files` を実行する
- QML や UI の変更では必要に応じてレイアウト確認モードを使う
- 環境依存の要件が不足していて検証を実行できない場合は、その旨を明記する
