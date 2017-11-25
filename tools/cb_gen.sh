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

	echo ${operands[@]} > /dev/stderr
	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_rlc_code() {
	local op=rlc
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo ${operands[@]} > /dev/stderr
	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_rr_code() {
	local op=rr
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo ${operands[@]} > /dev/stderr
	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_rrc_code() {
	local op=rrc
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo ${operands[@]} > /dev/stderr
	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_set_code() {
	local op=set
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	if [ "${operands[1]}" = "(hl)" ]; then
	echo ${operands[@]} > /dev/stderr
		:
	else
		:
	fi
}

function generate_cb_sla_code() {
	local op=sla
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo ${operands[@]} > /dev/stderr
	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_sll_code() {
	local op=sll
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo ${operands[@]} > /dev/stderr
	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_sra_code() {
	local op=sra
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo ${operands[@]} > /dev/stderr
	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

function generate_cb_srl_code() {
	local op=srl
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS

	echo ${operands[@]} > /dev/stderr
	if [ "${operands[0]}" = "(hl)" ]; then
		:
	else
		:
	fi
}

