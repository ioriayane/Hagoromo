#!/bin/sh
appname=$(basename $0 .sh)
basedir=$(cd $(dirname $0); pwd)

LD_LIBRARY_PATH=$basedir/lib
export LD_LIBRARY_PATH
#export QT_IM_MODULE=ibus
#export QT_IM_MODULE=fcitx
#export GTK_IM_MODULE=fcitx
#export XMODIFIERS=@im=fcitx

$basedir/bin/$appname "$@"

