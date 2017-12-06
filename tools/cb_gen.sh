regs="s_gb->gb_register"

function generate_cb_bit_code() {
	local operands
	local value

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[1]}" = "(hl)" ]; then
		value="read8bit(${regs}.hl, s_gb)"
	else
		value="${regs}.${operands[1]}"
	fi

	cat <<here_doc_delim
	${regs}.zf = !BIT(${operands[0]}, ${value});
here_doc_delim
}

function generate_cb_res_code() {
	local operands
	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local mask="~(1 << ${operands[0]})"

	if [ "${operands[1]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, read8bit(${regs}.hl, s_gb) & ${mask}, s_gb);"
	else
		echo -e "\t${regs}.${operands[1]} &= ${mask};"
	fi
}

function generate_cb_rl_code() {
	local operands

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		value="read8bit(${regs}.hl, s_gb)"
	else
		value="${regs}.${operands[0]}"
	fi

	cat <<here_doc_delim
	bool carry;
	uint8_t value;

	carry = ${regs}.cf;
	value = ${value};
	${regs}.cf = !BIT(7, value);
	value <<= 1;
	value += carry;
	${regs}.zf = value == 0;
here_doc_delim

	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	else
		echo -e "\t${regs}.${operands[0]} = value;"
	fi
}

function generate_cb_rlc_code() {
	local operands

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		value="read8bit(${regs}.hl, s_gb)"
	else
		value="${regs}.${operands[0]}"
	fi

	cat <<here_doc_delim
	bool lone_bit;
	uint8_t value;

	value = ${value};
	lone_bit = BIT(7, value);
	${regs}.cf = lone_bit;
	value <<= 1;
	value += lone_bit;
	${regs}.zf = value == 0;
here_doc_delim

	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	else
		echo -e "\t${regs}.${operands[0]} = value;"
	fi
}

function generate_cb_rr_code() {
	local operands

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		value="read8bit(${regs}.hl, s_gb)"
	else
		value="${regs}.${operands[0]}"
	fi

	cat <<here_doc_delim
	bool carry;
	uint8_t value;

	carry = ${regs}.cf;
	value = ${value};
	${regs}.f = BIT(0, value);
	value >>= 1;
	value += (carry << 7);
	${regs}.zf = value == 0;
here_doc_delim

	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	else
		echo -e "\t${regs}.${operands[0]} = value;"
	fi
}

function generate_cb_rrc_code() {
	local operands

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		value="read8bit(${regs}.hl, s_gb)"
	else
		value="${regs}.${operands[0]}"
	fi

	cat <<here_doc_delim
	bool lone_bit;
	uint8_t value;

	value = ${value};
	lone_bit = BIT(0, value);
	${regs}.cf = lone_bit;
	value >>= 1;
	value += (lone_bit << 7);
	${regs}.zf = value == 0;
here_doc_delim

	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	else
		echo -e "\t${regs}.${operands[0]} = value;"
	fi
}

function generate_cb_set_code() {
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local mask="(1 << ${operands[0]})"

	if [ "${operands[1]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, read8bit(${regs}.hl, s_gb) | ${mask}, s_gb);"
	else
		echo -e "\t${regs}.${operands[1]} |= ${mask};"
	fi
}

function generate_cb_sla_code() {
	local operands

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		value="read8bit(${regs}.hl, s_gb)"
	else
		value="${regs}.${operands[0]}"
	fi

	cat <<here_doc_delim
	uint8_t value;

	value = ${value};
	${regs}.cf = BIT(7, value);
	value <<= 1;
	${regs}.zf = value == 0;
here_doc_delim

	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	else
		echo -e "\t${regs}.${operands[0]} = value;"
	fi
}

function generate_cb_swap_code() {
	local operands

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		value="read8bit(${regs}.hl, s_gb)"
	else
		value="${regs}.${operands[0]}"
	fi

	cat <<here_doc_delim
	uint8_t value;

	value = ${value};
	value = ((value & 0xf) << 4) | ((value & 0xf0) >> 4);
	${regs}.zf = value == 0;
here_doc_delim

	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	else
		echo -e "\t${regs}.${operands[0]} = value;"
	fi
}

function generate_cb_sra_code() {
	local operands

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		value="read8bit(${regs}.hl, s_gb)"
	else
		value="${regs}.${operands[0]}"
	fi

	cat <<here_doc_delim
	uint8_t value;
	bool bit7;

	value = ${value};
	bit7 = BIT(7, value);
	${regs}.cf = BIT(0, value);
	value >>= 1;
	${regs}.zf = value == 0;
	if (bit7)
		FLAGS_SET(value, (1 << 7));
	else
		FLAGS_CLEAR(value, (1 << 7));
here_doc_delim

	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	else
		echo -e "\t${regs}.${operands[0]} = value;"
	fi
}

function generate_cb_srl_code() {
	local operands

	OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		value="read8bit(${regs}.hl, s_gb)"
	else
		value="${regs}.${operands[0]}"
	fi

	cat <<here_doc_delim
	uint8_t value;

	value = ${value};
	${regs}.f = BIT(0, value);
	value >>= 1;
	FLAGS_CLEAR(value, 1 << 7);
	${regs}.zf = value == 0;
here_doc_delim

	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	else
		echo -e "\t${regs}.${operands[0]} = value;"
	fi
}
