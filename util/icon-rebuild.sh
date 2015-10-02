#!/bin/bash
#
# Rebuild platform specific icons from source png
#
# Copyright: © 2015 Jeffrey Clark
# License: GPLv3 (http://www.gnu.org/licenses/gpl-3.0.html)

set -o nounset
set -o pipefail
set -o errtrace

error() {
	echo "ERROR in $0 : line $1 exit code $2"
	exit $2
}
trap 'error ${LINENO} ${?}' ERR

PLATFORM=$(uname)

# check for commands
! [ -x "$(command -v convert)" ] && echo "FATAL: 'convert' command not found, ImageMagick required." && exit 1

SCRATCH=$(mktemp -d -t icon.XXXXXXXX)

SCRIPT_PATH="${BASH_SOURCE[0]}";
if ([ -h "${SCRIPT_PATH}" ]) then
	while([ -h "${SCRIPT_PATH}" ]) do SCRIPT_PATH=`readlink "${SCRIPT_PATH}"`; done
fi
pushd . > /dev/null
cd `dirname ${SCRIPT_PATH}` > /dev/null
SCRIPT_PATH=`pwd`;

function finish {
	[ -d $SCRATCH ] && rm -rf "$SCRATCH"
}
trap finish EXIT

IN=icon.png

## windows
convert ${IN} -type TrueColorMatte \
	\( -clone 0 -resize 16x16 \) \
	\( -clone 0 -resize 32x32 \) \
	\( -clone 0 -resize 48x48 \) \
	\( -clone 0 -resize 256x256 \) \
	-delete 0 ../src/attract.ico

## osx
if [[ "$PLATFORM" != "Darwin" ]]; then
	if [ -x "$(command -v png2icns)" ]; then
		convert -resize 16x16   ${IN} -type TrueColorMatte "${SCRATCH}"/icon_16x16.png
		convert -resize 32x32   ${IN} -type TrueColorMatte "${SCRATCH}"/icon_32x32.png
		convert -resize 128x128 ${IN} -type TrueColorMatte "${SCRATCH}"/icon_128x128.png
		convert -resize 256x256 ${IN} -type TrueColorMatte "${SCRATCH}"/icon_256x256.png

		png2icns osx/attract.icns \
			"${SCRATCH}"/icon_16x16.png \
			"${SCRATCH}"/icon_32x32.png \
			"${SCRATCH}"/icon_128x128.png \
			"${SCRATCH}"/icon_256x256.png \
			icon.png
	else
		echo "WARNING: 'png2icns' not found. OSX icon not created."
	fi

else
	sips -z 16 16   ${IN} --out "${SCRATCH}"/icon_16x16.png
	sips -z 32 32   ${IN} --out "${SCRATCH}"/icon_16x16@2x.png
	sips -z 32 32   ${IN} --out "${SCRATCH}"/icon_32x32.png
	sips -z 64 64   ${IN} --out "${SCRATCH}"/icon_32x32@2x.png
	sips -z 128 128 ${IN} --out "${SCRATCH}"/icon_128x128.png
	sips -z 256 256 ${IN} --out "${SCRATCH}"/icon_128x128@2x.png
	sips -z 256 256 ${IN} --out "${SCRATCH}"/icon_256x256.png
	iconutil -c icns -o osx/attract.icns "${SCRATCH}"
fi
