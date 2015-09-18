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
VERSION=$(git describe --tag | sed 's/-[^-]*$//')
REVCOUNT=$(git rev-list HEAD --count)
BUNDLEVERSION=${VERSION//[v-]/.}; BUNDLEVERSION=${BUNDLEVERSION#"."}
SHORTVERSION=${LASTTAG//v/}

### begin: make
PLATFORM=$(uname)
MAKEOPTS='CC=clang CXX=clang++ AR=libtool ARFLAGS="-static -o"'

# osxcross auto-detect
if [[ "$PLATFORM" != "Darwin" ]]; then
	if [[ -z ${OSXCROSS_TARGET_DIR} || -z ${OSXCROSS_TARGET} ]]; then
		echo "osxcross not initialized. add osxcross-conf and osxcross-env to the current environment" && exit 1
	fi

	[[ -z ${TOOLCHAIN} ]] && export TOOLCHAIN=x86_64-apple-${OSXCROSS_TARGET}
	[[ -z ${LIB_BASE_PATH} ]] && export LIB_BASE_PATH="$(realpath ${OSXCROSS_TARGET_DIR}/macports/pkgs)"

	MAKEOPTS="$MAKEOPTS CROSS=1 TOOLCHAIN=${TOOLCHAIN} FE_MACOSX_COMPILE=1 EXTRA_CFLAGS=\"-arch i386 -arch x86_64\""
fi

NPROC=$(getconf _NPROCESSORS_ONLN)
LLIMIT=$(awk 'BEGIN{printf"%.1f",'${NPROC}'/2}')

make -C .. clean
eval make -C .. -j${NPROC} -l${LLIMIT} ${MAKEOPTS} $@
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
cp -a ../License.txt ${SCRATCH}/disk/
find .. -maxdepth 1 -name '*.md' | while read f
do
	fp=$(basename ${f} .md)
	pandoc -f markdown_github -t html5 "${f}" -o ${SCRATCH}/disk/${fp}.html
done
./output-changelog-md.sh ${LASTTAG} | pandoc -f markdown -t html5 -o "${SCRATCH}"/disk/Changelog.html

cat "${SCRIPT_PATH}"/Info.plist | sed 's/%%SHORTVERSION%%/'${SHORTVERSION}'/' | sed 's/%%BUNDLEVERSION%%/'${BUNDLEVERSION}'/' > "${APPCONTENT}"/Info.plist

# Copy extra libs to bundle and fix link path
pushd "${APPCONTENT}"/MacOS >/dev/null
${SCRIPT_PATH}/bundlelibs.py attract
popd >/dev/null

if [[ "$PLATFORM" != "Darwin" ]]; then
	genisoimage -D -V "AttractMode ${VERSION#v}" -no-pad -r -apple -o ${SCRATCH}/uncompressed.dmg ${SCRATCH}/disk/
	dmg dmg ${SCRATCH}/uncompressed.dmg ../attract-${VERSION#v}.dmg
else
	hdiutil create -volname "AttractMode ${VERSION#v}" -srcfolder ${SCRATCH}/disk/ -ov -format UDBZ ../attract-${VERSION#v}.dmg
fi
