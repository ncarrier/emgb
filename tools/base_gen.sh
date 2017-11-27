# functions for generating base set instructions
function generate_base_nop_code() {
	echo -e "\t/* nothing to do */"
}

adress_re='\((.*)\)'
function generate_base_ld_code() {
	local OLDIFS=$IFS; IFS=, operands=( $1 ); IFS=$OLDIFS
	local dst=${operands[0]}
	local src=${operands[1]}

	if [[ ${src} =~ ${adress_re} ]]; then
		src_adress=${BASH_REMATCH[1]}
		if [ "${src_adress}" = '**' ]; then
			echo -e "\tuint16_t src_adr = read16bit(s_gb->gb_register.pc + 1, s_gb);"
		else
			echo -e "\tuint16_t src_adr = s_gb->gb_register.${src_adress};"
		fi
		echo -e "\tuint8_t src_val = read8bit(src_adr, s_gb);"
		if [[ ${dst} =~ ${adress_re} ]]; then
			dst_adress=${BASH_REMATCH[1]}
			if [ "${dst_adress}" = '**' ]; then
				echo -e "\tuint16_t dst_adr = read16bit(s_gb->gb_register.pc + 1, s_gb);"
			else
				echo -e "\tuint16_t dst_adr = s_gb->gb_register.${src_adress};"
			fi
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
			echo -e -n "\tuint16_t dst_adr = "
			if [ ${dst_adress} = "**" ]; then
				echo -e "read16bit(s_gb->gb_register.pc + 1, s_gb);"
			else
				echo -e "s_gb->gb_register.${dst_adress};"
			fi
			echo -e "\twrite8bit(dst_adr, src_val, s_gb);"
		else
			echo -e "\ts_gb->gb_register.${dst} = src_val;"
		fi
	fi
}
