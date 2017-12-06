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
	if (BIT(${operands[0]}, ${value}))
		FLAGS_CLEAR(${regs}.f, FLAGS_ZERO);
	else
		FLAGS_SET(${regs}.f, FLAGS_ZERO);
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

	carry = FLAGS_ISSET(${regs}.f, FLAGS_CARRY);
	value = ${value};
	if (BIT(7, value) != 0)
		FLAGS_SET(${regs}.f, FLAGS_CARRY);
	else
		FLAGS_CLEAR(${regs}.f, FLAGS_CARRY);
	value <<= 1;
	value += carry;
	if (value != 0)
		FLAGS_CLEAR(${regs}.f, FLAGS_ZERO);
	else
		FLAGS_SET(${regs}.f, FLAGS_ZERO);
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
	if (lone_bit)
		FLAGS_SET(${regs}.f, FLAGS_CARRY);
	else
		FLAGS_CLEAR(${regs}.f, FLAGS_CARRY);
	value <<= 1;
	value += lone_bit;
	if (value != 0)
		FLAGS_CLEAR(${regs}.f, FLAGS_ZERO);
	else
		FLAGS_SET(${regs}.f, FLAGS_ZERO);
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

	carry = FLAGS_ISSET(${regs}.f, FLAGS_CARRY);
	value = ${value};
	if (BIT(0, value) != 0)
		FLAGS_SET(${regs}.f, FLAGS_CARRY);
	else
		FLAGS_CLEAR(${regs}.f, FLAGS_CARRY);
	value >>= 1;
	value += (carry << 7);
	if (value != 0)
		FLAGS_CLEAR(${regs}.f, FLAGS_ZERO);
	else
		FLAGS_SET(${regs}.f, FLAGS_ZERO);
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
	if (lone_bit)
		FLAGS_SET(${regs}.f, FLAGS_CARRY);
	else
		FLAGS_CLEAR(${regs}.f, FLAGS_CARRY);
	value >>= 1;
	value += (lone_bit << 7);
	if (value != 0)
		FLAGS_CLEAR(${regs}.f, FLAGS_ZERO);
	else
		FLAGS_SET(${regs}.f, FLAGS_ZERO);
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
	if (BIT(7, value) != 0)
		FLAGS_SET(${regs}.f, FLAGS_CARRY);
	else
		FLAGS_CLEAR(${regs}.f, FLAGS_CARRY);
	value <<= 1;
	if (value != 0)
		FLAGS_CLEAR(${regs}.f, FLAGS_ZERO);
	else
		FLAGS_SET(${regs}.f, FLAGS_ZERO);
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
	if (value != 0)
		FLAGS_CLEAR(${regs}.f, FLAGS_ZERO);
	else
		FLAGS_SET(${regs}.f, FLAGS_ZERO);
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
	if (BIT(0, value) != 0)
		FLAGS_SET(${regs}.f, FLAGS_CARRY);
	else
		FLAGS_CLEAR(${regs}.f, FLAGS_CARRY);
	value >>= 1;
	if (value != 0)
		FLAGS_CLEAR(${regs}.f, FLAGS_ZERO);
	else
		FLAGS_SET(${regs}.f, FLAGS_ZERO);
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
	if (BIT(0, value) != 0)
		FLAGS_SET(${regs}.f, FLAGS_CARRY);
	else
		FLAGS_CLEAR(${regs}.f, FLAGS_CARRY);
	value >>= 1;
	FLAGS_CLEAR(value, 1 << 7);
	if (value != 0)
		FLAGS_CLEAR(${regs}.f, FLAGS_ZERO);
	else
		FLAGS_SET(${regs}.f, FLAGS_ZERO);
here_doc_delim

	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(${regs}.hl, value, s_gb);"
	else
		echo -e "\t${regs}.${operands[0]} = value;"
	fi
}
