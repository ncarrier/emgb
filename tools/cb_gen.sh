function generate_cb_bit_code() {
	local op=bit
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[1]}" = "(hl)" ]; then
		echo -e "\tif (BIT(${operands[0]}, read8bit(s_gb->gb_register.hl, s_gb)))"
	else
		echo -e "\tif (BIT(${operands[0]}, s_gb->gb_register.${operands[1]}))"
	fi
	echo -e "\t\tFLAGS_CLEAR(s_gb->gb_register.f, FLAGS_ZERO);"
	echo -e "\telse"
	echo -e "\t\tFLAGS_SET(s_gb->gb_register.f, FLAGS_ZERO);"
}

function generate_cb_res_code() {
	local op=res
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local mask="~(1 << ${operands[0]})"

	if [ "${operands[1]}" = "(hl)" ]; then
		echo -e "\twrite8bit(s_gb->gb_register.hl, read8bit(s_gb->gb_register.hl, s_gb) & ${mask}, s_gb);"
	else
		echo -e "\ts_gb->gb_register.${operands[1]} &= ${mask};"
	fi
}

function generate_cb_rl_code() {
	local op=rl
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo -e "\tbool carry;"
	echo -e "\tuint8_t value;"
	echo
	echo -e "\tcarry = FLAGS_ISSET(s_gb->gb_register.f, FLAGS_CARRY);"
	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\tvalue = read8bit(s_gb->gb_register.hl, s_gb);"
	else
		echo -e "\tvalue = s_gb->gb_register.${operands[0]};"
	fi
	echo -e "\tif (BIT(7, value) != 0)"
	echo -e "\t\tFLAGS_SET(s_gb->gb_register.f, FLAGS_CARRY);"
	echo -e "\telse"
	echo -e "\t\tFLAGS_CLEAR(s_gb->gb_register.f, FLAGS_CARRY);"
	echo -e "\tvalue <<= 1;"
	echo -e "\tvalue += carry;"
	echo -e "\tif (value != 0)"
	echo -e "\t\tFLAGS_CLEAR(s_gb->gb_register.f, FLAGS_ZERO);"
	echo -e "\telse"
	echo -e "\t\tFLAGS_SET(s_gb->gb_register.f, FLAGS_ZERO);"
	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(s_gb->gb_register.hl, value, s_gb);"
	else
		echo -e "\ts_gb->gb_register.${operands[0]} = value;"
	fi
}

function generate_cb_rlc_code() {
	local op=rlc
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo -e "\tbool lone_bit;"
	echo -e "\tuint8_t value;"
	echo
	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\tvalue = read8bit(s_gb->gb_register.hl, s_gb);"
	else
		echo -e "\tvalue = s_gb->gb_register.${operands[0]};"
	fi
	echo -e "\tlone_bit = BIT(7, value);"
	echo -e "\tif (lone_bit)"
	echo -e "\t\tFLAGS_SET(s_gb->gb_register.f, FLAGS_CARRY);"
	echo -e "\telse"
	echo -e "\t\tFLAGS_CLEAR(s_gb->gb_register.f, FLAGS_CARRY);"
	echo -e "\tvalue <<= 1;"
	echo -e "\tvalue += lone_bit;"
	echo -e "\tif (value != 0)"
	echo -e "\t\tFLAGS_CLEAR(s_gb->gb_register.f, FLAGS_ZERO);"
	echo -e "\telse"
	echo -e "\t\tFLAGS_SET(s_gb->gb_register.f, FLAGS_ZERO);"
	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(s_gb->gb_register.hl, value, s_gb);"
	else
		echo -e "\ts_gb->gb_register.${operands[0]} = value;"
	fi
}

function generate_cb_rr_code() {
	local op=rr
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo -e "\tbool carry;"
	echo -e "\tuint8_t value;"
	echo
	echo -e "\tcarry = FLAGS_ISSET(s_gb->gb_register.f, FLAGS_CARRY);"
	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\tvalue = read8bit(s_gb->gb_register.hl, s_gb);"
	else
		echo -e "\tvalue = s_gb->gb_register.${operands[0]};"
	fi
	echo -e "\tif (BIT(0, value) != 0)"
	echo -e "\t\tFLAGS_SET(s_gb->gb_register.f, FLAGS_CARRY);"
	echo -e "\telse"
	echo -e "\t\tFLAGS_CLEAR(s_gb->gb_register.f, FLAGS_CARRY);"
	echo -e "\tvalue >>= 1;"
	echo -e "\tvalue += (carry << 7);"
	echo -e "\tif (value != 0)"
	echo -e "\t\tFLAGS_CLEAR(s_gb->gb_register.f, FLAGS_ZERO);"
	echo -e "\telse"
	echo -e "\t\tFLAGS_SET(s_gb->gb_register.f, FLAGS_ZERO);"
	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(s_gb->gb_register.hl, value, s_gb);"
	else
		echo -e "\ts_gb->gb_register.${operands[0]} = value;"
	fi
}

function generate_cb_rrc_code() {
	local op=rrc
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo -e "\tbool lone_bit;"
	echo -e "\tuint8_t value;"
	echo
	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\tvalue = read8bit(s_gb->gb_register.hl, s_gb);"
	else
		echo -e "\tvalue = s_gb->gb_register.${operands[0]};"
	fi
	echo -e "\tlone_bit = BIT(0, value);"
	echo -e "\tif (lone_bit)"
	echo -e "\t\tFLAGS_SET(s_gb->gb_register.f, FLAGS_CARRY);"
	echo -e "\telse"
	echo -e "\t\tFLAGS_CLEAR(s_gb->gb_register.f, FLAGS_CARRY);"
	echo -e "\tvalue >>= 1;"
	echo -e "\tvalue += (lone_bit << 7);"
	echo -e "\tif (value != 0)"
	echo -e "\t\tFLAGS_CLEAR(s_gb->gb_register.f, FLAGS_ZERO);"
	echo -e "\telse"
	echo -e "\t\tFLAGS_SET(s_gb->gb_register.f, FLAGS_ZERO);"
	if [ "${operands[0]}" = "(hl)" ]; then
		echo -e "\twrite8bit(s_gb->gb_register.hl, value, s_gb);"
	else
		echo -e "\ts_gb->gb_register.${operands[0]} = value;"
	fi
}

function generate_cb_set_code() {
	local op=set
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local mask="(1 << ${operands[0]})"

	if [ "${operands[1]}" = "(hl)" ]; then
		echo -e "\twrite8bit(s_gb->gb_register.hl, read8bit(s_gb->gb_register.hl, s_gb) | ${mask}, s_gb);"
	else
		echo -e "\ts_gb->gb_register.${operands[1]} |= ${mask};"
	fi
}

function generate_cb_sla_code() {
	local op=sla
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_sll_code() {
	local op=sll
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_sra_code() {
	local op=sra
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_srl_code() {
	local op=srl
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

