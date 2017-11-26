#!/bin/bash

set -euf

source tools/cb_gen.sh
source tools/base_gen.sh

file=$1
temp_file=$(mktemp)
grep -E '</?tr|td|th|table' ${file} > ${temp_file}

re_td='<td [^>]*axis="(.*)">(.*)<.*'
re_td_ln='<td class="ln">'
re_help='(.*)\|(.*)\|(.*)\|(.*)'
re_sll='^sll (.*)'

CARRY=0
NEG=1
HALFC=3
ZERO=4

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
	opcode=0x${high_nibble}${low_nibble}
	doc=${BASH_REMATCH[4]}
	if [[ ${text} =~ ${re_sll} ]]; then
		text="swap ${BASH_REMATCH[1]}"
		doc="The high and low nibbles of ${BASH_REMATCH[1]} are swapped."
	fi
	func=$(text_to_func "${title}" ${text})
}

function parse_exec_line() {
	opcode=0x${high_nibble}${low_nibble}
	text="exec ${opcode}"
	flags="------"
	size=2
	cycles=0
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
		${body_gen} ${opcode} "${text}" "${doc}" ${cycles} ${size} \
			${func} "${title}" "${flags}"

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

	generate_cb_${op}_code "${text[1]}"
}

function generate_base_opcode() {
	local text=( $1 )

	if [ ${#text[*]} -eq 1 ]; then
		operands=""
	else
		operands=${text[1]}
	fi
	op=${text[0]}

	# call if function is defined
	if type -t generate_base_${op}_code > /dev/null; then
		generate_base_${op}_code "${operands}"
	else
		echo ${op} > /dev/stderr
	fi
}

function definition_body_gen() {
	local opcode=$1
	local text=$2
	local doc=$3
	local cycles=$4
	local size=$5
	local func=$6
	local title=$7
	local flags=$8

	flags=( "${8:0:1}" "${8:1:1}" "${8:2:1}" "${8:3:1}" "${8:4:1}" "${8:5:1}" )
	echo "/* ${text} [${opcode}] : ${doc} */"
	echo "static void ${func}(struct s_gb *s_gb)"
	echo "{"
	echo "	/* start of ${func} manual code */"
	echo
	echo "	/* end of ${func} manual code */"
	if [ "${title}" = "CB" ]; then
		generate_cb_opcode "${text}"
	else
		generate_base_opcode "${text}"
	fi
	for f in ZERO NEG HALFC CARRY; do
		case ${flags[${f}]} in
		"-")
			echo "	/* ${f} unaffected */"
			;;
		"+")
			echo "	/* ${f} affected as defined */"
			;;
		"1")
			echo "	/* ${f} set */"
			echo "	SET_${f}();"
			;;
		"0")
			echo "	/* ${f} reset */"
			echo "	CLEAR_${f}();"
			;;
		*)
			echo "	/* unknown action on ${f} */"
		esac
	done
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
	echo "const struct s_cpu_z80 instructions_${title,,}[] = {"
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
echo -e "#include \"utils.h\"\n"
echo -e "#include \"GB.h\"\n"

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

