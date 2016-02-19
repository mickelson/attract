#!/bin/bash
#
# Generate markdown format changelog
# from tag (default HEAD) to previous tag
# grouped by author
#
# Copyright: © 2015 Jeffrey Clark
# License: GPLv3 (http://www.gnu.org/licenses/gpl-3.0.html)

# set cwd
SCRIPT_PATH="${BASH_SOURCE[0]}";
if ([ -h "${SCRIPT_PATH}" ]) then
	while([ -h "${SCRIPT_PATH}" ]) do SCRIPT_PATH=`readlink "${SCRIPT_PATH}"`; done
fi
pushd . > /dev/null
cd `dirname ${SCRIPT_PATH}` > /dev/null
SCRIPT_PATH=`pwd`;
cd ${SCRIPT_PATH}/..

if [ $# -eq 0 ] ; then
	TAG=$(git describe --tag --abbrev=0)
else
	## Just for attractmode
	if [[ $1 =~ ^v ]]; then
		TAG=$1
	else
		TAG=v$1
	fi
fi
PRETAG=$(git describe --tag --abbrev=0 ${TAG}^)

IFS=$'\r\n'

echo -e "# Changelog #"

RANGE="${TAG}..HEAD"
i=0
for author in $(git --no-pager log --no-merges --pretty=format:"%an" ${RANGE} | sort | uniq -c) ;
do
	[ $i -eq 0 ] && echo -e "\n## Commits made after ${TAG} ##"
	name=$(echo $author | sed 's/^\ *\([0-9]*\)\ \(.*\)$/\2/')
	count=$(echo $author | sed 's/^\ *\([0-9]*\)\ \(.*\)$/\1/')
	echo -e "\n### $name ($count commits)\n"
	git --no-pager log --author="$name" --date=short --no-merges --pretty=format:"* %s" ${RANGE} | grep -v "(nw)\| NW\| nw"
	(( i++ ))
done

RANGE="${PRETAG}..${TAG}"

echo -e "\n## Commits from ${PRETAG} to ${TAG}"

for author in $(git --no-pager log --no-merges --pretty=format:"%an" ${RANGE} | sort | uniq -c) ;
do
	name=$(echo $author | sed 's/^\ *\([0-9]*\)\ \(.*\)$/\2/')
	count=$(echo $author | sed 's/^\ *\([0-9]*\)\ \(.*\)$/\1/')
	echo -e "\n### $name ($count commits)\n"
	git --no-pager log --author="$name" --date=short --no-merges --pretty=format:"* %s" ${RANGE} | grep -v "(nw)\| NW\| nw"
done
