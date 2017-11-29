# functions for generating base set instructions
function generate_base_nop_code() {
	echo -e "\t/* nothing to do */"
}

adress_re='\((.*)\)'
function generate_base_ld_dst_adress_code() {
	echo -e -n "\tuint16_t dst_adr = "
	if [ ${dst_adress} = "**" ]; then
		echo -e "read16bit(s_gb->gb_register.pc + 1, s_gb);"
	elif [ ${dst_adress} = "*" ]; then
		echo -e "0xFF00u + read8bit(s_gb->gb_register.pc + 1, s_gb);"
	elif [ ${dst_adress} = "c" ]; then
		echo -e "0xFF00u + s_gb->gb_register.${dst_adress};"
	else
		echo -e "s_gb->gb_register.${dst_adress};"
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
			echo "read16bit(s_gb->gb_register.pc + 1, s_gb);"
		elif [ "${src_adress}" = '*' ]; then
			echo "0xFF00u + read8bit(s_gb->gb_register.pc + 1, s_gb);"
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
			echo -e "\tuint8_t src_val = read8bit(s_gb->gb_register.pc + 1, s_gb);"
		elif [ ${src} = "**" ]; then
			echo -e "\tuint16_t src_val = read16bit(s_gb->gb_register.pc + 1, s_gb);"
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

function generate_base_add_code() {
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local dst=${operands[0]}
	local src=${operands[1]}

	echo -e -n "\tuint32_t result = s_gb->gb_register.${dst} + "
	if [ "${src}" = "*" ]; then
		echo "read8bit(s_gb->gb_register.pc + 1, s_gb);"
	elif [ "${src}" = "**" ]; then
		echo "read16bit(s_gb->gb_register.pc + 1, s_gb);"
	elif [ "${src}" = "(hl)" ]; then
		echo "read16bit(s_gb->gb_register.hl, s_gb);"
	else
		echo "s_gb->gb_register.${src};"
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

function generate_base_xor_code() {
	local operand=$1
	local target

	if [ "${operand}" = "(hl)" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(s_gb->gb_register.hl, s_gb);
here_doc_delim
	elif [ "${operand}" = "*" ]; then
		target="value";
	cat <<here_doc_delim
	uint8_t value = read8bit(s_gb->gb_register.pc + 1, s_gb);
here_doc_delim
	else
		target="s_gb->gb_register.${operand}"
	fi

	cat <<here_doc_delim

	s_gb->gb_register.a ^= ${target};

	if (s_gb->gb_register.a != 0)
		CLEAR_ZERO();
	else
		SET_ZERO();
here_doc_delim
}

function generate_base_sub_code() {
	local operand=$1
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
	uint8_t value = read8bit(s_gb->gb_register.pc + 1, s_gb);
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

	s_gb->gb_register.a -= ${target};

	if (s_gb->gb_register.a != 0)
		CLEAR_ZERO();
	else
		SET_ZERO();
here_doc_delim
}

function generate_base_sbc_code() {
	local operand=$1

	echo -e -n "\tuint8_t value = "
	if [ "${operand}" = "(hl)" ]; then
		echo  "read8bit(s_gb->gb_register.hl, s_gb);"
	elif [ "${operand}" = "*" ]; then
		echo "read8bit(s_gb->gb_register.pc + 1, s_gb);"
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
