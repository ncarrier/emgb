#!/bin/bash

set -eu

me=$(realpath $0)
my_dir=$(dirname ${me})/

checkpatch=${my_dir}checkpatch2.pl

sources=$(find src -name '*.c' -o -name '*.h')

exclude=""

for f in ${sources}; do
	echo '**** '$f' ****'
	skip="false"
	for e in ${exclude}; do
		if [ "${f}" = "${e}" ]; then
			echo " -> skipping ${f}"
			skip="true"
			break
		fi
	done
	if [ "${skip}" = "true" ]; then
		continue
	fi
	${checkpatch} --no-tree -f ${f}
	echo -e '\n'
done
