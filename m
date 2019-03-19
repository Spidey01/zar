#!/bin/sh
set -e
clear

ZAR_BUILDDIR="${ZAR_BUILDDIR:-obj}"
ZAR_TOOLCHAIN=${ZAR_TOOLCHAIN:-gcc}
export ZAR_BUILDDIR ZAR_TOOLCHAIN

./ninja/make-zlib.sh

rm -vf build.ninja
cat ninja/global.ninja ninja/${ZAR_TOOLCHAIN}.ninja ninja/unix.ninja ninja/rules.ninja > build.ninja

ninja $*

