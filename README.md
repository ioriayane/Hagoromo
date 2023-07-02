# 羽衣 - Hagoromo -

(English follows Japanese.)

## 概要

マルチカラム対応の[Bluesky](https://blueskyweb.xyz/)クライアントです。

## 動作確認環境

- Windows 11 Profesional
- Ubuntu 22.04
- Mac 10.13

## 使用ソフトウェア

- [Qt 5.15.2](https://www.qt.io/)
- [OpenSSL 1.1.1q](http://www.openssl.org/)

## ダウンロード

[Releases](https://github.com/ioriayane/Hagoromo/releases)よりダウンロードしてください。

## インストール

アーカイブを解凍して任意のフォルダに保存してください。

## ビルド方法

### 共通

ビルドするプラットフォームのQt 5.15.2をインストールしてください。

Official sise : [Qt](https://www.qt.io/)

### Windows

[Microsoft Visual Studio 2019](https://visualstudio.microsoft.com/)をインストールしてください（Community Edition可）。

改行コードをCRLFでチェックアウトしてください。

```cmd
REM checkout repo
>git clone git@github.com:ioriayane/Hagoromo.git
>git submodule update -i
REM copy and edit encryption seed
>copy Hagoromo\app\qtquick\encryption_seed_template.h Hagoromo\app\qtquick\encryption_seed.h
REM build Hagoromo
>.\scripts\build.bat path\to\Qt\5.15.2\msvc2019_64\bin
REM Execute
>deploy-hagoromo\hagoromo\Hagoromo.exe
```

### Ubuntu

OpenSSL 1.1.1q 以上を使用します。build.shでビルドします。

```bash
# checkout repo
$ git clone git@github.com:ioriayane/Hagoromo.git
$ git submodule update -i
# copy and edit encryption seed
$ cp Hagoromo/app/qtquick/encryption_seed_template.h Hagoromo/app/qtquick/encryption_seed.h
$ vi Hagoromo/app/qtquick/encryption_seed.h
# build Hagoromo
$ ./scripts/build.sh linux path/to/Qt/5.15.2/gcc_64/bin
# exec hagoromo
$ ./deploy-hagoromo/hagoromo/Hagoromo.sh
```

### Mac

OpenSSL 1.1.1q 以上を使用します。build.shでビルドします。

```bash
# checkout repo
$ git clone git@github.com:ioriayane/Hagoromo.git
$ git submodule update -i
# copy and edit encryption seed
$ cp Hagoromo/app/qtquick/encryption_seed_template.h Hagoromo/app/qtquick/encryption_seed.h
$ vi Hagoromo/app/qtquick/encryption_seed.h
# build Hagoromo
$ ./scripts/build.sh mac path/to/Qt/5.15.2/gcc_64/bin
# exec hagoromo
$ open ./deploy-hagoromo/hagoromo/Hagoromo.app
```


## Overview

This is a multi-column [Bluesky](https://blueskyweb.xyz/) client.

## System Requirements

- Windows 11 Profesional
- Ubuntu 22.04
- Mac 10.13

## Software used

- [Qt 5.15.2](https://www.qt.io/)
- [OpenSSL 1.1.1q](http://www.openssl.org/)

## Download

Please download from [Releases](https://github.com/ioriayane/Hagoromo/releases).

## Installation

Unzip the archive and save it in any folder.

## How to Build

### Common

Install Qt 5.15.2 for the platform you want to build.

Official sise : [Qt](https://www.qt.io/)


### Windows

Install [Microsoft Visual Studio 2019](https://visualstudio.microsoft.com/) (Community Edition  is also available).

Check out the newline code in CRLF.

```cmd
REM checkout repo
>git clone git@github.com:ioriayane/Hagoromo.git
>git submodule update -i
REM copy and edit encryption seed
>copy Hagoromo\app\qtquick\encryption_seed_template.h Hagoromo\app\qtquick\encryption_seed.h
REM build Hagoromo
>.\scripts\build.bat path\to\Qt\5.15.2\msvc2019_64\bin
REM Execute
>deploy-hagoromo\hagoromo\Hagoromo.exe
```

### Ubuntu

OpenSSL 1.1.1q or higher is required. It is built in build.sh.

```bash
# checkout repo
$ git clone git@github.com:ioriayane/Hagoromo.git
$ git submodule update -i
# copy and edit encryption seed
$ cp Hagoromo/app/qtquick/encryption_seed_template.h Hagoromo/app/qtquick/encryption_seed.h
$ vi Hagoromo/app/qtquick/encryption_seed.h
# build Hagoromo
$ ./scripts/build.sh linux path/to/Qt/5.15.2/gcc_64/bin
# exec hagoromo
$ ./deploy-hagoromo/hagoromo/Hagoromo.sh
```

### Mac

OpenSSL 1.1.1q or higher is required. It is built in build.sh.

```bash
# checkout repo
$ git clone git@github.com:ioriayane/Hagoromo.git
$ git submodule update -i
# copy and edit encryption seed
$ cp Hagoromo/app/qtquick/encryption_seed_template.h Hagoromo/app/qtquick/encryption_seed.h
$ vi Hagoromo/app/qtquick/encryption_seed.h
# build Hagoromo
$ ./scripts/build.sh mac path/to/Qt/5.15.2/gcc_64/bin
# exec hagoromo
$ open ./deploy-hagoromo/hagoromo/Hagoromo.app
```
