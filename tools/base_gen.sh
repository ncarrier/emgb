# functions for generating base set instructions
# helper functions for instruction code generating functions
adress_re='\((.*)\)'
regs="s_gb->gb_register"
pc="${regs}.pc"
function generate_base_ld_dst_adress_code() {
	echo -e -n "\tuint16_t dst_adr = "
	if [ ${dst_adress} = "**" ]; then
		echo -e "read16bit(${pc} + 1, s_gb);"
	elif [ ${dst_adress} = "*" ]; then
		echo -e "0xFF00u + read8bit(${pc} + 1, s_gb);"
	elif [ ${dst_adress} = "c" ]; then
		echo -e "0xFF00u + ${regs}.${dst_adress};"
	else
		echo -e "${regs}.${dst_adress};"
	fi
}

function generate_base_binary_op_code() {
	local operand=$1
	local operator=$2
	local target

	if [ "${operand}" = "(hl)" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(${regs}.hl, s_gb);
here_doc_delim
	elif [ "${operand}" = "*" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(${pc} + 1, s_gb);
here_doc_delim
	else
		target="${regs}.${operand}"
	fi

	cat <<here_doc_delim

	${regs}.a ${operator}= ${target};
	${regs}.zf = ${regs}.a == 0;
here_doc_delim
}

function generate_base_sub_gen_code() {
	local operand=$1
	local apply=$2
	local target

	if [ "${operand}" = "a" ]; then
		cat <<here_doc_delim
	${regs}.a = 0;
here_doc_delim
		return 0
	fi

	if [ "${operand}" = "(hl)" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(${regs}.hl, s_gb);
here_doc_delim
	elif [ "${operand}" = "*" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(${pc} + 1, s_gb);
here_doc_delim
	else
		target="${regs}.${operand}"
	fi

	cat <<here_doc_delim

	${regs}.cf = ${target} > ${regs}.a;
	${regs}.hf = (${target} & 0x0f) > (${regs}.a & 0x0f);
	${regs}.zf = ${regs}.a == ${target};

here_doc_delim
	if [ "${apply}" = "true" ]; then
		echo -e "\t${regs}.a -= ${target};"
	fi
}

function generate_base_add_carry_code() {
	local operands
	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local dst=${operands[0]}
	local src=${operands[1]}
	local add_carry=$2

	cat <<here_doc_delim
	uint16_t value;
	uint32_t result;

here_doc_delim
	echo -n -e "\tvalue = "
	if [ "${src}" = "*" ]; then
		echo "read8bit(${pc} + 1, s_gb);"
	elif [ "${src}" = "**" ]; then
		echo "read16bit(${pc} + 1, s_gb);"
	elif [ "${src}" = "(hl)" ]; then
		echo "read16bit(${regs}.hl, s_gb);"
	else
		echo "${regs}.${src};"
	fi
	echo -e "\tresult = ${regs}.${dst} + value;"
	if [ "${add_carry}" = "true" ]; then
		echo -e "\tresult += ${regs}.cf;"
	fi
	cat <<here_doc_delim
	${regs}.cf = result & 0xffff0000;
	${regs}.hf = ((${regs}.${dst} & 0x0f) + (value & 0x0f)) > 0x0f;

	${regs}.${dst} = 0xffffu & result;
here_doc_delim
}

function generate_base_jp_uncond_code() {
	local dest=$1
	local register

	if [ "${dest}" = "(hl)" ]; then
		echo -e "\t${pc} = ${regs}.hl;"
	else
		echo -e "\t${pc} = read16bit(${regs}.pc + 1, s_gb);"
	fi
}

function generate_base_ldi_or_ldd_code() {
	local operands
	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local dst=${operands[0]}
	local src=${operands[1]}
	local operator="${2}"

	if [ "${dst}" = "a" ]; then
		cat <<here_doc_delim
	${regs}.a = read8bit(${regs}.hl, s_gb);
	${regs}.hl${operator};
here_doc_delim
	else
		cat <<here_doc_delim
	write8bit(${regs}.hl, ${regs}.a, s_gb);
	${regs}.hl${operator};
here_doc_delim
	fi
}

# start of functions generating instructions code
function generate_base_add_code() {
	generate_base_add_carry_code "$1" false
}

function generate_base_adc_code() {
	generate_base_add_carry_code "$1" true
}

function generate_base_and_code() {
	generate_base_binary_op_code "$1" "&"
}

function generate_base_call_code() {
	local operands
	local cond
	local neg

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [[ "${1}" == *","* ]]; then
		cond=${operands[0]}
		[[ ${cond} == "n"* ]] && neg="!" || neg=""
	cat <<here_doc_delim
	if (${neg}${regs}.${cond: -1}f) {
		push16(${pc} + 3, s_gb);
		${pc} = read16bit(${pc} + 1, s_gb);
	} else {
		${pc} += 3;
	}
here_doc_delim
	else
		cat <<here_doc_delim
	push16(${pc} + 3, s_gb);
	${pc} = read16bit(${pc} + 1, s_gb);
here_doc_delim
	fi
}

function generate_base_exec_code() {
	cat <<here_doc_delim
	uint8_t opcode;

	opcode = read8bit(${pc}, s_gb);
	instructions_cb[opcode].func(s_gb);
here_doc_delim
}

function generate_base_ccf_code() {
	cat <<here_doc_delim
	${regs}.cf = !${regs}.cf;
here_doc_delim
}

function generate_base_cp_code() {
	generate_base_sub_gen_code "$1" false
}

function generate_base_cpl_code() {
	echo -e "\t${regs}.a = ~${regs}.a;"
}

function generate_base_daa_code() {
	cat <<here_doc_delim
	int8_t low_inc = 0x06;
	int8_t high_inc = 0x60;
	uint16_t s;

	if (${regs}.nf) {
		low_inc = -0x06;
		high_inc = -0x60;
	}
	s = ${regs}.a;
	if (${regs}.hf || (s & 0xF) > 9)
		s += low_inc;
	if (${regs}.cf || s > 0x9F)
		s += high_inc;

	${regs}.a = s & 0xff;

	${regs}.zf = ${regs}.a == 0;
	${regs}.cf = s >= 0x100;
here_doc_delim
}

function generate_base_dec_code() {
	local operand=$1
	local target

	regsize=${#operand}

	if [ "${operand}" = "(hl)" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(${regs}.hl, s_gb);
here_doc_delim
	else
		target="${regs}.${operand}"
	fi
	if [ ${regsize} -ne 2 ]; then
		echo -e "\t${regs}.hf = !(${target} & 0x0f);"
	fi

	echo -e "\t${target}--;"

	if [ ${regsize} -ne 2 ]; then
		cat <<here_doc_delim
	${regs}.zf = ${target} == 0;
here_doc_delim
	fi
	if [ "${operand}" = "(hl)" ]; then
		cat <<here_doc_delim
	write8bit(${regs}.hl, value, s_gb);
here_doc_delim
	fi
}

function generate_base_di_code() {
	echo -e "\ts_gb->gb_interrupts.interMaster = 0;"
}

function generate_base_ei_code() {
	echo -e "\ts_gb->gb_interrupts.interMaster = 1;"
}

function generate_base_halt_code() {
	echo -e "\ts_gb->gb_cpu.stopCpu = 1;"
}

function generate_base_inc_code() {
	local operand=$1
	local target
	local reg_size

	# valid for all but (hl), but in fact we just want 2 or not 2
	# flags are affected only for 1 byte operands and (hl)
	regsize=${#operand}

	if [ "${operand}" = "(hl)" ]; then
		target="value";
		echo -e "\tuint8_t value = read8bit(${regs}.hl, s_gb);\n"
	else
		target="${regs}.${operand}"
	fi

	if [ ${regsize} -ne 2 ]; then
		echo -e "\t${regs}.hf = (${target} & 0x0f) == 0x0f;"
	fi

	echo -e "\t${target}++;"

	if [ ${regsize} -ne 2 ]; then
		echo -e "\t${regs}.zf = ${target} == 0;"
	fi

	if [ "${operand}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	fi
}

function generate_base_jp_code() {
	local operands
	local neg

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [[ "$1" == *","* ]]; then
		local cond=${operands[0]}

		[[ ${cond} == "n"* ]] && neg="!" || neg=""
	cat <<here_doc_delim
	if (${neg}${regs}.${cond: -1}f)
		${pc} = read16bit(${pc} + 1, s_gb);
	else
		${pc} += 3;
here_doc_delim
	else
		generate_base_jp_uncond_code $1
	fi
}

function generate_base_jr_code() {
	local operands
	local neg

	if [[ "$1" == *","* ]]; then
		OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
		local cond=${operands[0]}

		[[ ${cond} == "n"* ]] && neg="!" || neg=""
	cat <<here_doc_delim
	if (${neg}${regs}.${cond: -1}f)
		${pc} += (int8_t)read8bit(${pc} + 1, s_gb) + 2;
	else
		${pc} += 2;
here_doc_delim
	else
		echo -e "\t${pc} += (int8_t)read8bit(${regs}.pc + 1, s_gb) + 2;"
	fi
}

function generate_base_ld_code() {
	local operands
	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local dst=${operands[0]}
	local src=${operands[1]}

	if [[ ${src} =~ ${adress_re} ]]; then
		src_adress=${BASH_REMATCH[1]}
		echo -e -n "\tuint16_t src_adr = "
		if [ "${src_adress}" = '**' ]; then
			echo "read16bit(${pc} + 1, s_gb);"
		elif [ "${src_adress}" = '*' ]; then
			echo "0xFF00u + read8bit(${pc} + 1, s_gb);"
		elif [ "${src_adress}" = 'c' ]; then
			echo "0xFF00u + ${regs}.c;"
		else
			echo "${regs}.${src_adress};"
		fi
		echo -e "\tuint8_t src_val = read8bit(src_adr, s_gb);"
		if [[ ${dst} =~ ${adress_re} ]]; then
			dst_adress=${BASH_REMATCH[1]}
			generate_base_ld_dst_adress_code ${dst_adress}
			echo -e "\twrite8bit(dst_adr, src_val, s_gb);"
		else
			echo -e "\t${regs}.${dst} = src_val;"
		fi
	else
		if [ ${src} = "*" ]; then
			echo -e "\tuint8_t src_val = read8bit(${pc} + 1, s_gb);"
		elif [ ${src} = "**" ]; then
			echo -e "\tuint16_t src_val = read16bit(${pc} + 1, s_gb);"
		else
			src_reg="${regs}.${src}"
			echo -e "\t__typeof__(${src_reg}) src_val = ${src_reg};"
		fi
		if [[ ${dst} =~ ${adress_re} ]]; then
			dst_adress=${BASH_REMATCH[1]}
			generate_base_ld_dst_adress_code ${dst_adress}
			echo -e "\twrite8bit(dst_adr, src_val, s_gb);"
		else
			echo -e "\t${regs}.${dst} = src_val;"
		fi
	fi
}

function generate_base_ldd_code() {
	generate_base_ldi_or_ldd_code "$1" "--"
}

function generate_base_ldhl_code() {
	cat <<here_doc_delim
	int8_t value = read8bit(${regs}.pc, s_gb);
	int res;

	res = value + ${regs}.sp;

	${regs}.cf = res & 0xffff0000;
	${regs}.hf = ((${regs}.sp & 0x0f) + (value & 0x0f)) > 0x0f;

	${regs}.hl = res & 0x0000ffff;
here_doc_delim
}

function generate_base_ldi_code() {
	generate_base_ldi_or_ldd_code "$1" "++"
}

function generate_base_nop_code() {
	echo -e "\t/* nothing to do */"
}

function generate_base_or_code() {
	generate_base_binary_op_code "$1" '|'
}

function generate_base_pop_code() {
	local reg=$1

	echo -e "\t${regs}.${reg} = pop16(s_gb);"
	if [ ${reg} = "af" ]; then
		echo -e "\t${regs}.f &= 0xf0;"
	fi
}

function generate_base_push_code() {
	local reg=$1

	echo -e "\tpush16(${regs}.${reg}, s_gb);"
}

function generate_base_ret_code() {
	local operands
	local neg

	if [ -n "$1" ]; then
		OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
		local cond=${operands[0]}

		[[ ${cond} == "n"* ]] && neg="!" || neg=""
	cat <<here_doc_delim
	if (${neg}${regs}.${cond: -1}f)
		${pc} = pop16(s_gb);
	else
		${pc}++;
here_doc_delim
	else
		echo -e "\t${pc} = pop16(s_gb);"
	fi
}

function generate_base_reti_code() {
	cat <<here_doc_delim
	${regs}.pc = pop16(s_gb);
	s_gb->gb_interrupts.interMaster = 1;
here_doc_delim
}

function generate_base_rla_code() {
	cat <<here_doc_delim
	bool carry;

	carry = ${regs}.a & 0x80;
	${regs}.cf = carry;

	${regs}.a <<= 1;
	${regs}.zf = ${regs}.a == 0;
here_doc_delim
}

function generate_base_rlca_code() {
	cat <<here_doc_delim
	bool carry;

	carry = ${regs}.a & 0x80;
	${regs}.cf = carry;

	${regs}.a <<= 1;
	${regs}.a += carry;
	${regs}.zf = ${regs}.a == 0;
here_doc_delim
}

function generate_base_rra_code() {
	cat <<here_doc_delim
	bool carry;

	carry = ${regs}.cf;
	${regs}.cf = ${regs}.a & 0x01;
	${regs}.a >>= 1;
	${regs}.a += carry << 7;

	${regs}.zf = ${regs}.a == 0;
here_doc_delim
}

function generate_base_rrca_code() {
	cat <<here_doc_delim
	bool carry = ${regs}.a & 0x01;

	${regs}.cf = carry;
	${regs}.a >>= 1;

	${regs}.a |= carry << 7;
	${regs}.zf = ${regs}.a == 0;
here_doc_delim
}

function generate_base_rst_code() {
	local offset=0x${1//h/}

	cat <<here_doc_delim
	${regs}.sp -= 2;
	write16bitToAddr(${regs}.sp, ${pc}, s_gb);
	${pc} = ${offset};
here_doc_delim
}

function generate_base_sbc_code() {
	local operand=$1

	echo -e -n "\tuint8_t value = "
	if [ "${operand}" = "(hl)" ]; then
		echo  "read8bit(${regs}.hl, s_gb);"
	elif [ "${operand}" = "*" ]; then
		echo "read8bit(${pc} + 1, s_gb);"
	else
		echo "${regs}.${operand};"
	fi

	cat <<here_doc_delim
	value += ${regs}.cf;
	${regs}.cf = value > ${regs}.a;

	${regs}.hf = (value & 0x0f) > (${regs}.a & 0x0f);

	${regs}.a -= value;

	${regs}.zf = ${regs}.a != 0;
here_doc_delim
}

function generate_base_scf_code() {
	:
}

function generate_base_stop_code() {
	cat <<here_doc_delim
	s_gb->gb_cpu.stopCpu = 1;
here_doc_delim
}

function generate_base_sub_code() {
	generate_base_sub_gen_code "$1" true
}

function generate_base_und_code() {
	:
}

function generate_base_xor_code() {
	generate_base_binary_op_code "$1" '^'
}

