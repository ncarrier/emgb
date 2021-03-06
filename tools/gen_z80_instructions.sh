#!/bin/bash

# usage gen_z80_instructions.sh table_gb.html instructions.c

set -euf

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
	dir="$( cd -P "$( dirname "${SOURCE}" )" && pwd )"
	SOURCE="$(readlink "$SOURCE")"
	[[ ${SOURCE} != /* ]] && SOURCE="$DIR/$SOURCE"
done
dir="$( cd -P "$( dirname "${SOURCE}" )" && pwd )"/

source ${dir}cb_gen.sh
source ${dir}base_gen.sh

file=$1
output=$2
exec > $2

temp_file=$(mktemp)
grep -E '</?tr|td|th|table' ${file} > ${temp_file}

re_td='<td [^>]*axis="(.*)">(.*)<.*'
re_td_ln='<td class="ln">'
re_help='(.*)\|(.*)\|(.*)\|(.*)\|(.*)'
re_sll='^sll (.*)'

cf=0
nf=1
hf=2
zf=3

line_number=0
function text_to_func() {
	local title=${1,,}

	title=${title%%\*\*}
	shift
	local text=$*

	ret=${text//(/p}
	ret=${ret//[) ,]/_}
	ret=${ret//[^_a-zA-Z0-9]/x}
	ret=${ret//__/_}
	ret=${ret/%_/}

	if [ -n "${title}" ]; then
		ret=${title}_${ret}
	fi
	echo ${ret}
}

function parse_instruction_line() {
	help=${BASH_REMATCH[1]}
	opcode=0x${high_nibble}${low_nibble}
	text=${BASH_REMATCH[2]}
	[[ "${help}" =~ ${re_help} ]];
	flags="${BASH_REMATCH[1]}"
	size=${BASH_REMATCH[2]}
	cycles=${BASH_REMATCH[3]}
	cycles_cond=${BASH_REMATCH[4]}
	opcode=0x${high_nibble}${low_nibble}
	doc=${BASH_REMATCH[5]}
	func=$(text_to_func "${title}" ${text})
}

function parse_exec_line() {
	opcode=0x${high_nibble}${low_nibble}
	text="cb *"
	flags="----"
	size=2
	cycles=0
	cycles_cond=0
	doc="execute instruction in subtable ${opcode}"
	func="${title}exec_${opcode}"
}

parse_line() {
	local line=$1
	local regex=$2
	local title=$3
	local action=$4

	if [[ "${line}" =~ ${regex} ]]; then
		printf -v low_nibble "%X" ${line_number}

		${action}
		${body_gen} ${opcode} "${text}" "${doc}" ${cycles} \
			${cycles_cond} ${size} ${func} "${title}" "${flags}"

		line_number=$((${line_number} + 1))
	fi
}

function generate() {
	local file=$1
	local title=${2//\"/}
	local header_gen=$3
	local body_gen=$4
	local footer_gen=$5
	local help
	local text
	local flags
	local cycles
	local cycles_cond
	local doc
	local opcode
	local func
	local high_nible
	local low_nible
	local line

	local title_found=false
	while read line; do
		if [[ "${line}" =~ '<table title="'${title}'">' ]]; then
			${header_gen} "${title}"
			title_found=true
		fi
		if [ "${title_found}" = "true" ]; then
			if [[ "${line}" =~ \<th\>(.*)\</th\> ]]; then
				high_nibble=${BASH_REMATCH[1]}
				line_number=0
			fi
			parse_line "${line}" "${re_td}" "${title}" parse_instruction_line
			parse_line "${line}" "${re_td_ln}" "${title}" parse_exec_line
			if [[ "${line}" =~ '</table>' ]]; then
				${footer_gen} "${title}"
				break
			fi
		fi
	done < ${file}
}

function definition_header_gen() {
	echo "/* start of op code function definitions */"
}

function generate_cb_opcode() {
	local text=( $1 )

	op=${text[0]}
	operands=${text[1]}

	generate_cb_${op}_code "${text[1]}" "${operands}"
}

function generate_base_opcode() {
	local text=( $1 )
	local opcode=$2
	local cycles_cond=$3

	if [ ${#text[*]} -eq 1 ]; then
		operands=""
	else
		operands=${text[1]}
	fi
	op=${text[0]}

	# call if function is defined
	if type -t generate_base_${op}_code > /dev/null; then
		generate_base_${op}_code "${operands}" ${opcode} ${cycles_cond}
	else
		echo "not implemented yet: [${opcode}] ${op} \"${operands}\"" > /dev/stderr
	fi
}

und_defined="false"

function definition_body_gen() {
	local opcode=$1
	local text=$2
	local doc=$3
	local cycles=$4
	local cycles_cond=$5
	local size=$6
	local func=$7
	local title=$8
	local flags

	if [ "${func}" = "und" ]; then
		if [ "${und_defined}" = "false" ]; then
			und_defined="true"
		else
			return 0
		fi
	fi

	flags=( "${9:0:1}" "${9:1:1}" "${9:2:1}" "${9:3:1}" )
	echo "/* ${text} [${opcode}] : ${doc} */"
	echo "static unsigned ${func}(struct gb *s_gb)"
	echo "{"
	if [ "${title}" = "CB" ]; then
		generate_cb_opcode "${text}" ${opcode}
	else
		generate_base_opcode "${text}" ${opcode} ${cycles_cond}
	fi
	for f in zf nf hf cf; do
		case ${flags[${f}]} in
		"-")
			echo "	/* ${f} unaffected */"
			;;
		"+")
			echo "	/* ${f} affected as defined */"
			;;
		"1")
			echo "	/* ${f} set */"
			echo "	s_gb->registers.${f} = true;"
			;;
		"0")
			echo "	/* ${f} reset */"
			echo "	s_gb->registers.${f} = false;"
			;;
		"*")
			echo "	/* ${f} exceptional */"
			;;
		*)
			echo "	/* unknown action on ${f} */"
		esac
	done
	echo -e "\n\treturn ${cycles};\n}"
	echo
}

function definition_footer_gen() {
	:
}

function struct_header_gen() {
	local title=$1

	if [ -z "${title}" ]; then
		title=base
	fi
	echo "const struct cpu_op instructions_${title,,}[] = {"
}

function struct_body_gen() {
	local opcode=$1
	local text=$2
	local doc=$3
	local cycles=$4
	local cycles_cond=$5
	local real_size=$6
	local func=$7

	size=${real_size}
	for op in jp jr ret call rst reti; do
		local re="^${op}"
		if [[ "${func}" =~ ${re} ]]; then
			size=0
		fi
	done
	cat<<here_doc_delim
	[${opcode}] = {
		.opcode = ${opcode},
		.value = "${text}",
		.doc = "${doc}",
		.cycles = ${cycles},
		.cycles_cond = ${cycles_cond},
		.size = ${size},
		.real_size = ${real_size},
		.func = ${func},
	},
here_doc_delim
}

function struct_footer_gen() {
	echo "};"
}

cat <<here_doc_delim
/*
 * AUTOGENERATED CODE - $(date)
 * DON'T EDIT BY HAND !
 */

#include "gb.h"
#include "utils.h"
#include "instructions.h"

/* base instruction set */
here_doc_delim
# only CB and base instruction sets are present on the GB, plus some
# modifications
generate "${temp_file}" "" definition_{header,body,footer}_gen
generate "${temp_file}" "" struct_{header,body,footer}_gen
echo
title=CB
generate "${temp_file}" ${title} definition_{header,body,footer}_gen
generate "${temp_file}" ${title} struct_{header,body,footer}_gen
echo

