# 羽衣 - Hagoromo -

(English follows Japanese.)

## 概要

マルチカラム対応の[Bluesky](https://blueskyweb.xyz/)クライアントです。

## 動作確認環境

- Windows 11 Profesional
- Ubuntu 22.04

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

[Micrsoft Visual Studio 2019](https://visualstudio.microsoft.com/)をインストールしてください（Community Edition可）。


```cmd
REM setup env
>"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
REM checkout repo
>git clone git@github.com:ioriayane/Hagoromo.git
REM copy and edit encryption seed
>copy Hagoromo\app\qtquick\encryption_seed_template.h Hagoromo\app\qtquick\encryption_seed.h
REM build Hagoromo
>mkdir build-hagoromo
>cd build-hagoromo
>path\to\Qt\5.15.2\msvc2019_64\bin\qmake.exe ..\Hagoromo\Hagoromo.pro
>cd qpp\release
>path\to\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe --qmldir ..\..\..\Hagoromo\app\qml Hagoromo.exe
```


## Ubuntu

OpenSSL 1.1.1q 以上が必要です。

Qtのオンラインインストーラでインストールできるものが使用できます。

```bash
# build openssl
$ cd path/to/Qt/Tools/OpenSSL/src
$ ./config
$ make
# checkout repo
$ git clone git@github.com:ioriayane/Hagoromo.git
# copy and edit encryption seed
$ cp Hagoromo/app/qtquick/encryption_seed_template.h Hagoromo/app/qtquick/encryption_seed.h
$ vi Hagoromo/app/qtquick/encryption_seed.h
# build Hagoromo
$ mkdir build-hagoromo
$ cd build-hagoromo
$ path/to/Qt/5.15.2/gcc_64/bin/qmake ../Hagoromo/Hagoromo.pro
$ make
# exec hagoromo
$ export LD_LIBRARY_PATH=/path/to/Qt/Tools/OpenSSL/src
$ cd app
$ ./Hagoromo
```

## Overview

This is a multi-column [Bluesky](https://blueskyweb.xyz/) client.

## System Requirements

- Windows 11 Profesional
- Ubuntu 22.04

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

Install [Micrsoft Visual Studio 2019](https://visualstudio.microsoft.com/) (Community Edition  is also available).

```cmd
REM setup env
>"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
REM checkout repo
>git clone git@github.com:ioriayane/Hagoromo.git
REM copy and edit encryption seed
>copy Hagoromo\app\qtquick\encryption_seed_template.h Hagoromo\app\qtquick\encryption_seed.h
REM build Hagoromo
>mkdir build-hagoromo
>cd build-hagoromo
>path\to\Qt\5.15.2\msvc2019_64\bin\qmake.exe ..\Hagoromo\Hagoromo.pro
>cd qpp\release
>path\to\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe --qmldir ..\..\..\Hagoromo\app\qml Hagoromo.exe
```

### Ubuntu

OpenSSL 1.1.1q or higher is required.
You can use the source code installed by Qt's online installer.

```bash
# build openssl
$ cd path/to/Qt/Tools/OpenSSL/src
$ ./config
$ make
# checkout repo
$ git clone git@github.com:ioriayane/Hagoromo.git
# copy and edit encryption seed
$ cp Hagoromo/app/qtquick/encryption_seed_template.h Hagoromo/app/qtquick/encryption_seed.h
$ vi Hagoromo/app/qtquick/encryption_seed.h
# build Hagoromo
$ mkdir build-hagoromo
$ cd build-hagoromo
$ path/to/Qt/5.15.2/gcc_64/bin/qmake ../Hagoromo/Hagoromo.pro
$ make
# exec hagoromo
$ export LD_LIBRARY_PATH=/path/to/Qt/Tools/OpenSSL/src
$ cd app
$ ./Hagoromo
```
