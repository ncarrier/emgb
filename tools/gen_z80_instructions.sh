#!/bin/bash

set -eu

file=$1
temp_file=$(mktemp)
grep -E '</?tr|td|th|table' ${file} > ${temp_file}

re_td='<td [^>]*axis="(.*)">(.*)<.*'
re_help='(.*)\|(.*)\|(.*)\|(.*)'

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
	local doc
	local opcode
	local func
	local high_nible
	local low_nible
	local line
	local line_number

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
			if [[ "${line}" =~ ${re_td} ]]; then
				printf -v low_nibble "%X" ${line_number}
				help=${BASH_REMATCH[1]}
				text=${BASH_REMATCH[2]}
				[[ "${help}" =~ ${re_help} ]]; 
				flags=${BASH_REMATCH[1]}
				size=${BASH_REMATCH[2]}
				cycles=${BASH_REMATCH[3]}
				doc=${BASH_REMATCH[4]}
				opcode=0x${high_nibble}${low_nibble}
				func=$(set -f; text_to_func "${title}" ${text})
				${body_gen} ${opcode} "${text}" "${doc}" \
					${cycles} ${size} ${func}
				line_number=$((${line_number} + 1))
			fi
			if [[ "${line}" =~ '</table>' ]]; then
				${footer_gen}
				break
			fi
		fi
	done < ${file}
}

function definition_header_gen() {
	echo "/* start of op code function definitions */"
}

function definition_body_gen() {
	local opcode=$1
	local text=$2
	local doc=$3
	local cycles=$4
	local size=$5
	local func=$6

	echo "/* ${text} [${opcode}] : ${doc} */"
	echo "static void ${func}(struct s_gb *s_gb)"
	echo "{"
	echo "	/* start of ${func} manual code */"
	echo
	echo "	/* end of ${func} manual code */"
	echo "}"
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
	echo "const struct s_cpu_z80 instructions_${title}[] = {"
}

function struct_body_gen() {
	local opcode=$1
	local text=$2
	local doc=$3
	local cycles=$4
	local size=$5
	local func=$6

	echo -e "\t[${opcode}] = {"
	echo -e "\t\t.opcode = ${opcode},"
	echo -e "\t\t.value = \"${text}\","
	echo -e "\t\t.doc = \"${doc}\","
	echo -e "\t\t.cycles = ${cycles},"
	echo -e "\t\t.size = ${size},"
	echo -e "\t\t.func = ${func},"
	echo -e "\t},"
}

function struct_footer_gen() {
	echo "};"
}

echo -e "/*"
echo -e " * AUTOGENERATED CODE - $(date)"
echo -e " * edit by hand, only between \"start of xxx manual code\" and"
echo -e " * \"end of xxx manual code\" comments."
echo -e " */"

echo -e "#include \"cpu.h\"\n"

echo
# only CB and base instruction sets are present on the GB, plus some
# modifications
echo "/* base instruction set */"
generate "${temp_file}" "" definition_{header,body,footer}_gen
generate "${temp_file}" "" struct_{header,body,footer}_gen
echo
title=CB
generate "${temp_file}" ${title} definition_{header,body,footer}_gen
generate "${temp_file}" ${title} struct_{header,body,footer}_gen
echo

