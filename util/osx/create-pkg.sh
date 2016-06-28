#!/usr/bin/env bash
#
# Copyright: © 2015 Jeffrey Clark
# License: GPLv3 (http://www.gnu.org/licenses/gpl-3.0.html)

### begin: init
set -o pipefail
set -o errtrace

error() {
	echo "ERROR in $0 : line $1 exit code $2"
	exit $2
}
trap 'error ${LINENO} ${?}' ERR

SCRIPT_PATH="${BASH_SOURCE[0]}";
if ([ -h "${SCRIPT_PATH}" ]) then
	while([ -h "${SCRIPT_PATH}" ]) do SCRIPT_PATH=`readlink "${SCRIPT_PATH}"`; done
fi
pushd . > /dev/null
cd `dirname ${SCRIPT_PATH}` > /dev/null
SCRIPT_PATH=`pwd`;
cd ..
### end: init

LASTTAG=$(git describe --tag --abbrev=0)
VERSION=$(git describe --tag | sed 's/-[^-]\{8\}$//')
BUNDLEVERSION=${VERSION//[v-]/.}; BUNDLEVERSION=${BUNDLEVERSION#"."}
SHORTVERSION=${LASTTAG//v/}

### begin: make
PLATFORM=$(uname)
MAKEOPTS=""

# osxcross auto-detect
if [[ "$PLATFORM" != "Darwin" ]]; then
	! [ -x "$(command -v hfsplus)" ]      && echo "FATAL: 'hfsplus' not found" && exit 1
	! [ -x "$(command -v mkfs.hfsplus)" ] && echo "FATAL: 'mkfs.hfs' not found" && exit 1
	! [ -x "$(command -v dmg)" ]          && echo "FATAL: 'dmg' not found" && exit 1

	if [[ -z ${OSXCROSS_TARGET_DIR} || -z ${OSXCROSS_TARGET} ]]; then
		echo "osxcross not initialized. add osxcross-conf and osxcross-env to the current environment" && exit 1
	fi

	[[ -z ${TOOLCHAIN} ]] && export TOOLCHAIN=x86_64-apple-${OSXCROSS_TARGET}
	[[ -z ${LIB_BASE_PATH} ]] && export LIB_BASE_PATH="$(realpath ${OSXCROSS_TARGET_DIR}/macports/pkgs)"

	MAKEOPTS="$MAKEOPTS CC=clang CXX=clang++ AR=libtool ARFLAGS=\"-static -o\" CROSS=1 TOOLCHAIN=${TOOLCHAIN} FE_MACOSX_COMPILE=1"
fi

NPROC=$(getconf _NPROCESSORS_ONLN)
LLIMIT=$(awk 'BEGIN{printf"%.1f",'${NPROC}'/2}')

make -C .. clean
eval make -C .. -j${NPROC} -l${LLIMIT} ${MAKEOPTS} DATA_PATH=../config/ $@
### end: make

function finish {
	if [[ ! -z $SCRATCH && -d $SCRATCH ]] ; then
		rm -rf "${SCRATCH}"
	fi
}
trap finish EXIT

SCRATCH=$(mktemp -d -t package.XXXXXXXX)
APPCONTENT="${SCRATCH}"/disk/Attract.app/Contents

# Create bundle folder structure
mkdir -p "${APPCONTENT}"/{MacOS,Resources,libs}

cp -r ../config "${APPCONTENT}"/
[[ -d ../../extras ]] && cp -r ../../extras/* "${APPCONTENT}"/config/
cp -a ../attract "${APPCONTENT}"/MacOS/
cp -a "${SCRIPT_PATH}"/attract.icns "${APPCONTENT}"/Resources/
cp -a "${SCRIPT_PATH}"/launch.sh "${APPCONTENT}"/MacOS/

# Documentation
mkdir "${SCRATCH}/disk/Documentation"
cp -a ../License.txt ${SCRATCH}/disk/Documentation/
cp ../*.md "${SCRATCH}"/disk/Documentation/
./output-changelog-md.sh ${LASTTAG} > "${SCRATCH}"/disk/Documentation/Changelog.md

# convert markdown to html if possible
find "${SCRATCH}"/disk/Documentation/ -name '*.md' | while read f
do
	fp="${f%.md}"
	if ! [ -x "$(command -v pandoc)" ]; then
		mv "${f}" "${fp}.txt"
	else
		pandoc -f markdown_github -t html5 "${f}" -o "${fp}.html" && rm "${f}"
	fi
done

cat "${SCRIPT_PATH}"/Info.plist | sed 's/%%SHORTVERSION%%/'${SHORTVERSION}'/' | sed 's/%%BUNDLEVERSION%%/'${BUNDLEVERSION}'/' > "${APPCONTENT}"/Info.plist

cp osx/DS_Store "${SCRATCH}/disk/.DS_Store"
cp osx/VolumeIcon.icns "${SCRATCH}/disk/.VolumeIcon.icns"
mkdir "${SCRATCH}/disk/.background"
cp osx/background.png "${SCRATCH}/disk/.background/background.png"

# Copy extra libs to bundle and fix link path
pushd "${APPCONTENT}"/MacOS >/dev/null
${SCRIPT_PATH}/bundlelibs.py attract
popd >/dev/null

if [[ "$PLATFORM" != "Darwin" ]]; then
	WORKDMG="${SCRATCH}/uncompressed.dmg"
	DMGSIZE=$[$(du -k -s "${SCRATCH}/disk/" | cut -f1)+1024]
	dd if=/dev/zero of="${WORKDMG}" bs=1024 count=$DMGSIZE
	mkfs.hfsplus -v "AttractMode" "${WORKDMG}"
	hfsplus "${WORKDMG}" addall "${SCRATCH}/disk/"
	hfsplus "${WORKDMG}" attr / C
	hfsplus "${WORKDMG}" symlink " " /Applications
	# TODO: genisoimage has drawbacks, but most people dont have libdmg-hfsplus built
	#       make conditional package based on best available tools.
	# genisoimage -D -V "AttractMode ${VERSION#v}" -no-pad -r -apple -o ${SCRATCH}/uncompressed.dmg ${SCRATCH}/disk/
	dmg dmg "${WORKDMG}" ../attract-${VERSION}.dmg
else
	# TODO: mount dmg and set file attributes (volume icon) and create Application symlink
	hdiutil create -volname "AttractMode" -srcfolder ${SCRATCH}/disk/ -ov -format UDBZ ../attract-${VERSION}.dmg
fi
