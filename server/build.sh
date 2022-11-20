#!/bin/bash

CMAKE=cmake
PROFILE=Release

[ ! -z "$1" ] && PROFILE=$1

BLDDIR=.build/lab1/$PROFILE

$CMAKE --build $BLDDIR -- -j8
