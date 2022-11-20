#!/bin/bash

CMAKE=cmake
PROFILE=Release

[ ! -z "$1" ] && PROFILE=$1

BLDDIR=.build/lab1/$PROFILE
[ ! -d "$BLDDIR" ] && mkdir -p $BLDDIR

cd $BLDDIR
$CMAKE -G Ninja -DCMAKE_BUILD_TYPE=$PROFILE $OLDPWD
cd $OLDPWD
