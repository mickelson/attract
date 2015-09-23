#!/bin/bash
if [ "$1" == "" ]; then
        echo "Package version label not specified, aborting"
	exit 1
fi

if [ ! -d .git ]; then
        echo "This script needs to be run from an Attract-Mode git repository"
	exit 1
fi

echo "Preparing Windows packages with version label: $1"

# Clean build area
#
if [ -f ./attract.exe ]; then
    rm ./attract.exe
fi

# Make sure we are up to date
#
git pull origin master

# Set up staging area
#
if [ ! -d stage ]; then
    mkdir stage
else
    rm -r stage/*
fi

cp -r *.md *.txt config/* stage/
cp -r ../extras/* stage/

# output changelog
util/output-changelog-md.sh $1 > stage/Changelog.md

# Unix to Windows line endings
#
find stage -type f -regextype posix-egrep -regex '.*\.(md|txt|cfg|nut|nutr|msg|frag)$' -print0 | xargs -0 sed -i 's/$/\r/'

# Build and zip 32-bit windows version
#
make clean
make CROSS=1 TOOLCHAIN=i686-w64-mingw32.static WINDOWS_STATIC=1 -j 3
mv attract.exe stage/attract.exe
cd stage
zip -r attract-$1-win32.zip *
mv attract-$1-win32.zip ..
cd ..

# Build and zip 64-bit windows version
#
make clean
make CROSS=1 TOOLCHAIN=x86_64-w64-mingw32.static WINDOWS_STATIC=1 -j 3
mv attract.exe stage/attract.exe
cd stage
zip -r attract-$1-win64.zip *
mv attract-$1-win64.zip ..
cd ..
