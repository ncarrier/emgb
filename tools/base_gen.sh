# functions for generating base set instructions
# helper functions for instruction code generating functions
adress_re='\((.*)\)'
pc="s_gb->gb_register.pc"
function generate_base_ld_dst_adress_code() {
	echo -e -n "\tuint16_t dst_adr = "
	if [ ${dst_adress} = "**" ]; then
		echo -e "read16bit(${pc} + 1, s_gb);"
	elif [ ${dst_adress} = "*" ]; then
		echo -e "0xFF00u + read8bit(${pc} + 1, s_gb);"
	elif [ ${dst_adress} = "c" ]; then
		echo -e "0xFF00u + s_gb->gb_register.${dst_adress};"
	else
		echo -e "s_gb->gb_register.${dst_adress};"
	fi
}

function generate_base_binary_op_code() {
	local operand=$1
	local operator=$2
	local target

	if [ "${operand}" = "(hl)" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(s_gb->gb_register.hl, s_gb);
here_doc_delim
	elif [ "${operand}" = "*" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(${pc} + 1, s_gb);
here_doc_delim
	else
		target="s_gb->gb_register.${operand}"
	fi

	cat <<here_doc_delim

	s_gb->gb_register.a ${operator}= ${target};

	if (s_gb->gb_register.a != 0)
		CLEAR_ZERO();
	else
		SET_ZERO();
here_doc_delim
}

function generate_base_sub_gen_code() {
	local operand=$1
	local apply=$2
	local target

	if [ "${operand}" = "a" ]; then
		cat <<here_doc_delim
	s_gb->gb_register.a = 0;
here_doc_delim
		return 0
	fi

	if [ "${operand}" = "(hl)" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(s_gb->gb_register.hl, s_gb);
here_doc_delim
	elif [ "${operand}" = "*" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(${pc} + 1, s_gb);
here_doc_delim
	else
		target="s_gb->gb_register.${operand}"
	fi

	cat <<here_doc_delim

	if (${target} > s_gb->gb_register.a)
		SET_CARRY();
	else
		CLEAR_CARRY();

	if ((${target} & 0x0f) > (s_gb->gb_register.a & 0x0f))
		SET_HALFC();
	else
		CLEAR_HALFC();

	if (s_gb->gb_register.a == ${target})
		SET_ZERO();
	else
		CLEAR_ZERO();

here_doc_delim
	if [ "${apply}" = "true" ]; then
		echo -e "\ts_gb->gb_register.a -= ${target};"
	fi
}

function generate_base_add_carry_code() {
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local dst=${operands[0]}
	local src=${operands[1]}
	local add_carry=$2

	echo -e -n "\tuint32_t result = s_gb->gb_register.${dst} + "
	if [ "${src}" = "*" ]; then
		echo "read8bit(${pc} + 1, s_gb);"
	elif [ "${src}" = "**" ]; then
		echo "read16bit(${pc} + 1, s_gb);"
	elif [ "${src}" = "(hl)" ]; then
		echo "read16bit(s_gb->gb_register.hl, s_gb);"
	else
		echo "s_gb->gb_register.${src};"
	fi
	if [ "${add_carry}" = "true" ]; then
		echo -e "\tresult += FLAGS_ISCARRY(s_gb->gb_register.f);"
	fi
	cat <<here_doc_delim
	if (result & 0xffff0000)
		SET_CARRY();
	else
		CLEAR_CARRY();

	if (((s_gb->gb_register.${dst} & 0x0f) + (result & 0x0f)) > 0x0f)
		SET_HALFC();
	else
		CLEAR_HALFC();

	s_gb->gb_register.${dst} = 0xffffu & result;
here_doc_delim
}

function generate_base_jp_or_ret_or_jr_or_call_cond_code() {
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local cond=${operands[0]}
	local op=$2

	echo -e "\t${pc}++;"
	if [ "${op}" = "jp" ]; then
		size=2
		dest="read16bit(${pc}, s_gb)"
	elif [ "${op}" = "call" ]; then
		echo -e "\tpush16(read16bit(${pc} + 2, s_gb), s_gb);"
		size=2
		dest="read16bit(${pc}, s_gb)"
	elif [ "${op}" = "jr" ]; then
		size=1
		dest="${pc} + read8bit(${pc}, s_gb) + 1"
	else # ret
		size=0
		dest="pop16(s_gb)"
	fi
	echo -n -e "\tif ("
	if [[ ${cond} == "n"* ]]; then
		echo -n "!"
	fi
	if [[ ${cond} == *"c" ]]; then
		echo "FLAGS_ISCARRY(s_gb->gb_register.f))"
	else
		echo "FLAGS_ISZERO(s_gb->gb_register.f))"
	fi
	cat <<here_doc_delim
		${pc} = ${dest};
	else
		${pc} += ${size}; /* size of address */
here_doc_delim
}

function generate_base_jp_uncond_code() {
	local dest=$1
	local register

	if [ "${dest}" = "(hl)" ]; then
		register=hl
	else
		echo -e "\t${pc}++;"
		register=pc
	fi

	cat <<here_doc_delim
	${pc} = read16bit(s_gb->gb_register.${register}, s_gb);
here_doc_delim
}

function generate_base_ldi_or_ldd_code() {
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local dst=${operands[0]}
	local src=${operands[1]}
	local operator="${2}"

	if [ "${dst}" = "a" ]; then
		cat <<here_doc_delim
	s_gb->gb_register.a = read8bit(s_gb->gb_register.hl, s_gb);
	s_gb->gb_register.hl${operator};
here_doc_delim
	else
		cat <<here_doc_delim
	write8bit(s_gb->gb_register.hl, s_gb->gb_register.a, s_gb);
	s_gb->gb_register.hl${operator};
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
	local operands=$1

	if [[ "${operands}" == *","* ]]; then
		generate_base_jp_or_ret_or_jr_or_call_cond_code ${operands} "call"
	else
		echo -e "\tpush16(read16bit(${pc} + 1, s_gb), s_gb);"
		generate_base_jp_uncond_code ${operands}
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
	if (FLAGS_ISCARRY(s_gb->gb_register.f))
		FLAGS_CLEAR(s_gb->gb_register.f, FLAGS_CARRY);
	else
		FLAGS_SET(s_gb->gb_register.f, FLAGS_CARRY);
here_doc_delim
}

function generate_base_cp_code() {
	generate_base_sub_gen_code "$1" false
}

function generate_base_cpl_code() {
	echo -e "\ts_gb->gb_register.a = ~s_gb->gb_register.a;"
}

function generate_base_daa_code() {
	cat <<here_doc_delim
	int8_t low_inc = 0x06;
	int8_t high_inc = 0x60;
	uint16_t s;

	if (FLAGS_ISNEGATIVE(s_gb->gb_register.f)) {
		low_inc = -0x06;
		high_inc = -0x60;
	}
	s = s_gb->gb_register.a;
	if (FLAGS_ISHALFCARRY(s_gb->gb_register.f) || (s & 0xF) > 9)
		s += low_inc;
	if (FLAGS_ISCARRY(s_gb->gb_register.f) || s > 0x9F)
		s += high_inc;

	s_gb->gb_register.a = s & 0xff;
	if (s_gb->gb_register.a != 0)
		FLAGS_CLEAR(s_gb->gb_register.f, FLAGS_ZERO);
	else
		FLAGS_SET(s_gb->gb_register.f, FLAGS_ZERO);

	if (s >= 0x100)
		FLAGS_SET(s_gb->gb_register.f, FLAGS_CARRY);
	else
		FLAGS_CLEAR(s_gb->gb_register.f, FLAGS_CARRY);
here_doc_delim
}

function generate_base_dec_code() {
	local operand=$1
	local target

	if [ "${operand}" = "(hl)" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(s_gb->gb_register.hl, s_gb);
here_doc_delim
	else
		target="s_gb->gb_register.${operand}"
	fi
	cat <<here_doc_delim
	if (${target} & 0x0f)
		CLEAR_HALFC();
	else
		SET_HALFC();

	${target}--;

	if (${target})
		CLEAR_ZERO();
	else
		SET_ZERO();

	SET_NEG();
here_doc_delim
	if [ "${operand}" = "(hl)" ]; then
	cat <<here_doc_delim
	write8bit(s_gb->gb_register.hl, value, s_gb);
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

	if [ "${operand}" = "(hl)" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(s_gb->gb_register.hl, s_gb);
here_doc_delim
	else
		target="s_gb->gb_register.${operand}"
	fi
	cat <<here_doc_delim
	if ((${target} & 0x0f) == 0x0f)
		SET_HALFC();
	else
		CLEAR_HALFC();

	${target}++;

	if (${target})
		CLEAR_ZERO();
	else
		SET_ZERO();

	CLEAR_NEG();
here_doc_delim
	if [ "${operand}" = "(hl)" ]; then
	cat <<here_doc_delim
	write8bit(s_gb->gb_register.hl, value, s_gb);
here_doc_delim
	fi
}

function generate_base_jp_code() {
	local operands=$1

	if [[ "${operands}" == *","* ]]; then
		generate_base_jp_or_ret_or_jr_or_call_cond_code ${operands} "jp"
	else
		generate_base_jp_uncond_code ${operands}
	fi
}

function generate_base_jr_code() {
	local operands=$1

	if [ -n "${operands}" ]; then
		generate_base_jp_or_ret_or_jr_or_call_cond_code ${operands} "jr"
	else
		echo -e "\t${pc} = read8bit(s_gb->gb_register.hl, s_gb);"
	fi
}

function generate_base_ld_code() {
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
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
			echo "0xFF00u + s_gb->gb_register.c;"
		else
			echo "s_gb->gb_register.${src_adress};"
		fi
		echo -e "\tuint8_t src_val = read8bit(src_adr, s_gb);"
		if [[ ${dst} =~ ${adress_re} ]]; then
			dst_adress=${BASH_REMATCH[1]}
			generate_base_ld_dst_adress_code ${dst_adress}
			echo -e "\twrite8bit(dst_adr, src_val, s_gb);"
		else
			echo -e "\ts_gb->gb_register.${dst} = src_val;"
		fi
	else
		if [ ${src} = "*" ]; then
			echo -e "\tuint8_t src_val = read8bit(${pc} + 1, s_gb);"
		elif [ ${src} = "**" ]; then
			echo -e "\tuint16_t src_val = read16bit(${pc} + 1, s_gb);"
		else
			src_reg="s_gb->gb_register.${src}"
			echo -e "\t__typeof__(${src_reg}) src_val = ${src_reg};"
		fi
		if [[ ${dst} =~ ${adress_re} ]]; then
			dst_adress=${BASH_REMATCH[1]}
			generate_base_ld_dst_adress_code ${dst_adress}
			echo -e "\twrite8bit(dst_adr, src_val, s_gb);"
		else
			echo -e "\ts_gb->gb_register.${dst} = src_val;"
		fi
	fi
}

function generate_base_ldd_code() {
	generate_base_ldi_or_ldd_code "$1" "--"
}

function generate_base_ldhl_code() {
	cat <<here_doc_delim
	int8_t value = read8bit(s_gb->gb_register.pc, s_gb);
	int res;

	res = value + s_gb->gb_register.sp;

	if (res & 0xffff0000)
		SET_CARRY();
	else
		CLEAR_CARRY();

	if (((s_gb->gb_register.sp & 0x0f) + (value & 0x0f)) > 0x0f)
		SET_HALFC();
	else
		CLEAR_HALFC();

	s_gb->gb_register.hl = res & 0x0000ffff;
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

	echo -e "\ts_gb->gb_register.${reg} = pop16(s_gb);"
}

function generate_base_push_code() {
	local reg=$1

	echo -e "\tpush16(s_gb->gb_register.${reg}, s_gb);"
}

function generate_base_ret_code() {
	local operands=$1

	if [ -n "${operands}" ]; then
		generate_base_jp_or_ret_or_jr_or_call_cond_code ${operands} "ret"
	else
		echo -e "\t${pc} = pop16(s_gb);"
	fi
}

function generate_base_reti_code() {
	cat <<here_doc_delim
	s_gb->gb_register.pc = read16bit(s_gb->gb_register.sp, s_gb);
	s_gb->gb_register.sp += 2;
	s_gb->gb_interrupts.interMaster = 1;
here_doc_delim
}

function generate_base_rla_code() {
	cat <<here_doc_delim
	bool carry;

	carry = s_gb->gb_register.a & 0x80;
	if (carry)
		SET_CARRY();
	else
		CLEAR_CARRY();

	s_gb->gb_register.a <<= 1;
	if (s_gb->gb_register.a == 0)
		SET_ZERO();
	else
		CLEAR_ZERO();
here_doc_delim
}

function generate_base_rlca_code() {
	cat <<here_doc_delim
	bool carry;

	carry = s_gb->gb_register.a & 0x80;
	if (carry)
		SET_CARRY();
	else
		CLEAR_CARRY();

	s_gb->gb_register.a <<= 1;
	s_gb->gb_register.a += carry;
	if (s_gb->gb_register.a == 0)
		SET_ZERO();
	else
		CLEAR_ZERO();
here_doc_delim
}

function generate_base_rra_code() {
	cat <<here_doc_delim
	bool carry;

	carry = FLAGS_ISCARRY(s_gb->gb_register.f);
	if (s_gb->gb_register.a & 0x01)
		SET_CARRY();
	else
		CLEAR_CARRY();
	s_gb->gb_register.a >>= 1;
	s_gb->gb_register.a += carry << 7;

	if (s_gb->gb_register.a == 0)
		SET_ZERO();
	else
		CLEAR_ZERO();
here_doc_delim
}

function generate_base_rrca_code() {
	cat <<here_doc_delim
	bool carry = s_gb->gb_register.a & 0x01;

	if (carry)
		SET_CARRY();
	else
		CLEAR_CARRY();
	s_gb->gb_register.a >>= 1;

	s_gb->gb_register.a |= carry << 7;
	if (s_gb->gb_register.a == 0)
		SET_ZERO();
	else
		CLEAR_ZERO();
here_doc_delim
}

function generate_base_rst_code() {
	local offset=0x${1//h/}

	cat <<here_doc_delim
	s_gb->gb_register.sp -= 2;
	write16bitToAddr(s_gb->gb_register.sp, ${pc}, s_gb);
	${pc} = ${offset};
here_doc_delim
}

function generate_base_sbc_code() {
	local operand=$1

	echo -e -n "\tuint8_t value = "
	if [ "${operand}" = "(hl)" ]; then
		echo  "read8bit(s_gb->gb_register.hl, s_gb);"
	elif [ "${operand}" = "*" ]; then
		echo "read8bit(${pc} + 1, s_gb);"
	else
		echo "s_gb->gb_register.${operand};"
	fi

	cat <<here_doc_delim
	value += FLAGS_ISCARRY(s_gb->gb_register.f);
	if (value > s_gb->gb_register.a)
		SET_CARRY();
	else
		CLEAR_CARRY();

	if ((value & 0x0f) > (s_gb->gb_register.a & 0x0f))
		SET_HALFC();
	else
		CLEAR_HALFC();

	s_gb->gb_register.a -= value;

	if (s_gb->gb_register.a != 0)
		CLEAR_ZERO();
	else
		SET_ZERO();
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

