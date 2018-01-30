#!/bin/bash

set -eu

me=$(realpath $0)
my_dir=$(dirname ${me})/

checkpatch=${my_dir}checkpatch2.pl

sources="$(find -name '*.c' -o -name '*.h' | grep -v '/misc/')"

exclude="./include/imconfig.h
	./tools/joypad_mapping/joypad_mapping.c
	./imgui_impl_sdl.h
	./misc/instructions/instructions.c
	./imgui.h
	./legacy/cb.c
	./legacy/cpu.c
	./legacy/ld.h
	./src/imgui_demo.cpp
	./src/imgui_draw.cpp
	./src/imgui.cpp
	./include/imgui_impl_sdl.h
	./include/imgui.h
	./include/imgui_internal.h
	./include/stb_textedit.h
	./include/stb_rect_pack.h
	./include/stb_truetype.h
	"

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
