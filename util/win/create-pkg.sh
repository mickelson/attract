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

function finish {
	if [[ ! -z $SCRATCH && -d $SCRATCH ]] ; then
		rm -rf "${SCRATCH}"
	fi
}
trap finish EXIT

SCRATCH=$(mktemp -d -t package.XXXXXXXX)
APP="${SCRATCH}"/install
mkdir -p "${APP}"

# configuration
cp -r ../config/* "${APP}"

# documentation
cp -a ../License.txt ../*.md "${APP}"/
./output-changelog-md.sh ${LASTTAG} > "${APP}"/Changelog.md

# extras
[[ -d ../../extras ]] && cp -r ../../extras/* "${APP}"

find "${APP}"/ -name '*.md' | while read f
do
	fp="${f%.md}"
	if ! [ -x "$(command -v pandoc)" ]; then
		mv "${f}" "${fp}.txt"
	else
		pandoc -f markdown_github -t html5 "${f}" -o "${fp}.html" && rm "${f}"
	fi
done

# Unix to Windows line endings
find "${APP}" -type f -regextype posix-egrep -regex '.*\.(md|txt|cfg|nut|nutr|msg|frag)$' -print0 | xargs -0 sed -i 's/$/\r/'

# make and zip
MAKEOPTS="CROSS=1 WINDOWS_STATIC=1"

NPROC=$(getconf _NPROCESSORS_ONLN)
LLIMIT=$(awk 'BEGIN{printf"%.1f",'${NPROC}'/2}')

# 32-bit
make -C .. clean
eval make -C .. -j${NPROC} -l${LLIMIT} ${MAKEOPTS} TOOLCHAIN=i686-w64-mingw32.static $@
mv ../attract.exe "${APP}"/attract.exe

make -C .. clean
eval make -C .. -j${NPROC} -l${LLIMIT} ${MAKEOPTS} WINDOWS_CONSOLE=1 TOOLCHAIN=i686-w64-mingw32.static $@
mv ../attract.exe "${APP}"/attract-console.exe

pushd "${APP}" >/dev/null
zip -r ../attract-${VERSION}-win32.zip *
popd >/dev/null

# 64-bit
make -C .. clean
eval make -C .. -j${NPROC} -l${LLIMIT} ${MAKEOPTS} TOOLCHAIN=x86_64-w64-mingw32.static $@
mv ../attract.exe "${APP}"/attract.exe

make -C .. clean
eval make -C .. -j${NPROC} -l${LLIMIT} ${MAKEOPTS} WINDOWS_CONSOLE=1 TOOLCHAIN=x86_64-w64-mingw32.static $@
mv ../attract.exe "${APP}"/attract-console.exe

pushd "${APP}" >/dev/null
zip -r ../attract-${VERSION}-win64.zip *
popd >/dev/null

mv "${SCRATCH}"/*.zip ..
