#!/bin/sh
set -e

name="$(basename $0)"

if [ -z "$ZAR_BUILDDIR" -o -z "$ZAR_TOOLCHAIN" ]; then
    echo "$name: must set ZAR_BUILDDIR and ZAR_TOOLCHAIN to use $0"
    exit 1
fi

builddir="${ZAR_BUILDDIR}"
toolchain="${ZAR_TOOLCHAIN}"
export CC

if [ -f "$builddir/zlib/zlib.h" -a -f "$builddir/zlib/libz.a" ]; then
    exit 0
fi

echo "$name: Making zlib using ZAR_TOOLCHAIN=$toolchain under ZAR_BUILDDIR=$builddir"
mkdir -p "$builddir"

# lazy ;)
builddir="$builddir/zlib"

rm -rf "$builddir"
cp -r zlib "$builddir"

cd $builddir

echo "$name: Running configure script."
./configure --static

echo "$name: Building."
make

echo "$name: Use -I${builddir} for finding headers."
echo "$name: Use $builddir/libz.a or -L${builddir} -lz for linking."

