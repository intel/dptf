/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#include "esif_lf.h"
#include "esif_action.h"
#include "esif_ipc.h"
#include "esif_queue.h"
#include "esif_hash_table.h"

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified against Windows SDK/DDK included headers which we
 * have no control over.
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_ELF

#define INIT_DEBUG    0
#define LF_DEBUG      1
#define DECODE_DEBUG  2
#define EVENT_DEBUG   3

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_LF(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF, LF_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_DECODE(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF, DECODE_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_EVENT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF, EVENT_DEBUG, format, ##__VA_ARGS__)


/* List of Init/Exit Functions to be called during esif_lf_init/exit */
typedef enum esif_rc (*esif_init_callback)(void);
typedef void (*esif_exit_callback)(void);
struct esif_init_module {
	esif_init_callback	init_func;
	esif_exit_callback	exit_func;
	u32			started;
};
static struct esif_init_module esif_initializers[] = {
	{esif_lf_pm_init,		esif_lf_pm_exit}, /* Always First */
	{esif_action_acpi_init,		esif_action_acpi_exit},
	{esif_action_code_init,		esif_action_code_exit},
	{esif_action_const_init,	esif_action_const_exit},
	{esif_action_mmio_init,		esif_action_mmio_exit},
	{esif_action_msr_init,		esif_action_msr_exit},
	{esif_action_systemio_init,	esif_action_systemio_exit},
	{esif_action_var_init,		esif_action_var_exit},
	{esif_action_mbi_init,		esif_action_mbi_exit},
	{esif_data_init,		esif_data_exit},
	{esif_queue_init,		esif_queue_exit},
	{esif_event_init,		esif_event_exit},
	{esif_link_list_init,		esif_link_list_exit},
	{esif_hash_table_init,		esif_hash_table_exit},
	{esif_dsp_init,			esif_dsp_exit},
	{esif_command_init,		esif_command_exit} /* Always Last */
};
static enum esif_rc esif_start_initializers(void);
static void esif_stop_initializers(void);

/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */
#define GFX_DTS_TEMPERATURE_MAX 207
#define GFX_DTS_TEMPERATURE_MIN 127

#define GFX_DEGREE_TEMPERATURE_MAX 90
#define GFX_DEGREE_TEMPERATURE_MIN 10

/*
* DTS to Degree Celcius conversion with respect to table  below.
* Example : Temperature in Degree for DTS temperature of 157 will be calcuate
* as DTSToCelcius(157) =  ((GFX_DEGREE_TEMPERATRUE_MIN == 10)+
* ((GFX_DTS_TEMPERATRUE_MAX == 207)- (degreeTemp == 157))) = 60, which is
* correct in table below.
* Example :
* DTSToCelcius(180) =  ((GFX_DEGREE_TEMPERATRUE_MIN == 10)+
* ((GFX_DTS_TEMPERATRUE_MAX == 207)- (degreeTemp == 180))) = 37, which is
* correct in table below.
*/
#define DTS_TO_CELCIUS(temp) ((GFX_DEGREE_TEMPERATURE_MIN) + \
			      ((GFX_DTS_TEMPERATURE_MAX)-(temp)))


/* LPAT Xform Algorithme */
static u32 esif_xform_lpat(int ec, struct esif_lp_dsp *dsp_ptr)
{
	u8 *var_ptr = (u8 *) dsp_ptr->table;
	long long temp[2]={0, 0}; 
	long long raw[2]={0, 0}; 
	long long fp = 0; 
	long long temp_temp = 0; 
	long long actual_temp = 0;
	int i = 0; 
	int num_lpat = 0;

/* 
 * VS 2012 doesn't support "typeof(((type *)0)->memb *)" generic type,
 * so we can only use (long long *) instead.
 */
#define ESIF_VAR union esif_data_variant
#define var_offset(type, memb) ((size_t)&((type *)0)->memb)
#define var_sizeof(type, memb) (sizeof((type *)0)->memb)
#define val_var_offset(ptr, type, memb) *(long long *) \
		(ptr + var_offset(type, memb))
#define next_val_var_offset(ptr, type, all_memb, memb) *(long long *) \
		(ptr + var_offset(type, memb) + var_sizeof(type, all_memb)) 
	/* 
	 * data_variant.integer layout = {type, value} = {
	 *                                   ...
	 *                               {INT64,   25},
	 *                               {UINT64, 669},
	 *                                   ... }  
	 *
	 * Thermistor algorithm (EC raw reading example ec = 596)
	 *    Lookup LPAT ACPI object = {temp, rawVal} = {
	 * 			   ...
	 *                  {25, 669},
	 *                  {30, 615}, <-- temp[0], raw[0]
	 *                  {35, 561}, <-- temp[1], raw[1] 
	 *                  {40, 508},
	 *                         ... }
	 * Algorithm 
	 *    FP = (temp[1] - temp[0]) / (raw[0] - raw[1]) 
	 *       = ( 35       -  30      ) / ( 615     - 561       )
	 *       = 0.09259
	 * 
	 *    actual_temp = temp[0] + (raw[0] - ec) * FP;
	 *                =   30        + ( 615     - 596   ) * 0.09259 
	 *                =   30        + 1.759
	 *                =   31.759
	 *
	 * Due to kernel lacks of floating point, we will make it into 
	 * Milli-Celsisus in return as
	 *
	 *   FP = (temp[1] - temp[0]) * 1K * 1K / (raw[0] - raw[1])
	 *      = ( 35     -  30    ) * 1K * 1K / ( 615   - 561   )
	 *      = 92,590
	 *
	 *   actual_Temp = temp[0] * 1K + ((raw[0] - ec) * FP)     / 1K
	 *               =   30,000     + (( 19        ) * 92,590) / 1K
	 *               =   30,000     +    1,759
	 *               =   31,759
	 */ 
	if (0 == dsp_ptr->table_size)
		goto exit;

	num_lpat = dsp_ptr->table_size / sizeof(union esif_data_variant);

	for (i=0; i < ((num_lpat/2) - 1); i++) {
		temp[0] = val_var_offset(var_ptr, ESIF_VAR, integer.value);
		raw[0] =  next_val_var_offset(var_ptr, ESIF_VAR, integer, integer.value); 

		var_ptr += 2 * var_sizeof(ESIF_VAR, integer);
		temp[1] = val_var_offset(var_ptr, ESIF_VAR, integer.value);
		raw[1] = next_val_var_offset(var_ptr, ESIF_VAR, integer, integer.value);
  
		if (i == 0 && ec > raw[0])
			goto exit;
		if (ec <= raw[0] && ec >= raw[1]) 
			break;
	}
	if (ec <= raw[1])
		goto exit;

	fp = (temp[1] - temp[0]) * 1000000; 
	do_div(fp, (raw[0] - raw[1])); 

	temp_temp = (raw[0] - ec) * fp;
	do_div(temp_temp, 1000);	
	actual_temp = (temp[0] * 1000) + temp_temp;

exit:
	ESIF_TRACE_DYN_TEMP("%s: ec %d dsp.table_size %u num_plat %d LPAT[%d].raw %d actual_temp %llu\n",
		ESIF_FUNC, ec, dsp_ptr->table_size, num_lpat, 
		i, (int) raw[0], actual_temp);
	return (u32) actual_temp;
}


/*   DTS Counter   |	 Temperature
 *  Value   |   Degree Celcius
 *  127     |   90
 *  137     |   80
 *  147     |   70
 *  157     |   60
 *  167     |   50
 *  177     |   40
 *  187     |   30
 *  197     |   20
 *  207     |   10
 */

static ESIF_INLINE enum esif_rc esif_xform_temp(
	const enum esif_temperature_type type,
	esif_temp_t *temp_ptr,
	const enum esif_action_type action,
	const struct esif_lp_dsp *dsp_ptr,
	const struct esif_lp_primitive *primitive_ptr,
	struct esif_lp *lp_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	enum esif_temperature_type temp_in_type  = type;
	enum esif_temperature_type temp_out_type = type;
	struct esif_cpc_algorithm *algo_ptr      = dsp_ptr->get_algorithm(
			dsp_ptr,
			action);
	enum esif_primitive_opcode opcode = primitive_ptr->opcode;

	esif_temp_t temp_in;
	esif_temp_t temp_out;

	if (algo_ptr == NULL)
		return ESIF_E_NEED_ALGORITHM;

	temp_in  = *temp_ptr;
	temp_out = *temp_ptr;

	switch (algo_ptr->temp_xform) {
	
	case ESIF_ALGORITHM_TYPE_TEMP_DECIK:

		/* Convert Temp before/after ACPI action
		 * For Get:
		 *    From reading from APCI device driver in Kelvin
		 *    To normalized temp (C) back to user response buffer
		 * For Set:
		 *    From user request buffer in Kelvin or Celsius
		 *    To Kelvin passing to ACPI device driver to set
		 */
		ESIF_TRACE_DYN_TEMP(
			"%s: using algorithm DeciK (%s), for ACPI temp\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->temp_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_in_type  = ESIF_TEMP_DECIK;
			temp_out_type = type;
			/* Normalized from Kelvin */
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		} else {/* ESIF_PRIMITIVE_OP_SET */
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_DECIK;
			/* Normalized to Kelvin */
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_TJMAX_CORE:
	{
		/* LIFU Get TJMAX from GET_PROC_TJ_MAX */
		struct esif_lp_domain *lpd_ptr = NULL;
		u32 tjmax = 0;

		/*
		 * TjMax Only Lives In Doman 0 (D0) Level, If Found,
		 * Assign Tjmax, Or Use A Default Value (TODO:100??)
		 */
		lpd_ptr = &lp_ptr->domains[0];
		tjmax   = (lpd_ptr->temp_tjmax) ? lpd_ptr->temp_tjmax : 100;

		ESIF_TRACE_DYN_TEMP(
			"%s: using algorithm Tjmax %d %s, cached Tjmax %d,"
			" for CORE MSR temp\n",
			ESIF_FUNC,
			tjmax,
			esif_algorithm_type_str(algo_ptr->temp_xform),
			lpd_ptr->temp_tjmax);

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_out      = tjmax - temp_out;
			temp_in_type  = ESIF_TEMP_C;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		} else {
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_C;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
			temp_out = tjmax - temp_out;
		}
		break;
	}

	case ESIF_ALGORITHM_TYPE_TEMP_PCH_CORE:
	{
		ESIF_TRACE_DYN_TEMP(
			"%s: using algorithm %s, for PCH MMIO temp\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->temp_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_out      = (temp_out / 2) - 50;
			temp_in_type  = ESIF_TEMP_C;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		} else {
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_C;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
			temp_out = (temp_out / 2) - 50;
		}
		break;
	}

	case ESIF_ALGORITHM_TYPE_TEMP_TJMAX_ATOM:
	{
		struct esif_lp_domain *lpd_ptr = NULL;
		u32 tjmax = 0;

		/*
		 * TjMax Only Lives In Doman 0 (D0) Level, If Found,
		 * Assign Tjmax, Or Use A Default Value (TODO:100??)
		 */
		lpd_ptr = &lp_ptr->domains[0];
		tjmax   = (lpd_ptr->temp_tjmax) ? lpd_ptr->temp_tjmax : 100;

		ESIF_TRACE_DYN_TEMP(
			"%s: using algorithm %s tjmax %d, cached tjmax %d, for ATOM MSR temp\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->temp_xform),
			tjmax,
			lpd_ptr->temp_tjmax);

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			u32 temp = 0;
			/* Algorithm taken from 7.0 DPTF Code */

			/* This will read the current temperature from MSR.
			 * Please refer BWG for details.  Valid temp. range is
			 * -20 deg to 90 deg.
			 */
			temp = ((temp_out >> 16) & 0x007F);

			/*
			 * Bit 4 is set OR Bit 31 is not set --- indicates above
			 * 90 deg temperature
			 */
			if ((temp_out & 0x10) ||
			    ((temp_out & 0x80000000) == 0x0)) {
				temp_out = tjmax + temp;
			} else {
				temp_out = tjmax - temp;
			}

			temp_in_type  = ESIF_TEMP_C;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		} else {
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_C;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		}
		break;
	}

	case ESIF_ALGORITHM_TYPE_TEMP_DTS_ATOM:
	{
		u32 tjmax = dsp_ptr->get_temp_tc1(dsp_ptr, action);
		ESIF_TRACE_DYN_TEMP(
			"%s: using algorithm %s, tjmax %d for ATOM\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->temp_xform),
			tjmax);
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_out      = DTS_TO_CELCIUS(temp_out) + tjmax - 90;
			temp_in_type  = ESIF_TEMP_C;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		} else {/* ESIF_PRIMITIVE_OP_SET */
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_C;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		}
		break;
	}

	case ESIF_ALGORITHM_TYPE_TEMP_NONE:
		ESIF_TRACE_DYN_TEMP(
			"%s: using algorithm none (%s), for Code and "
			"Konst temp\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->temp_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_in_type  = ESIF_TEMP_C;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		} else {/* ESIF_PRIMITIVE_OP_SET */
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_C;
			esif_convert_temp(temp_in_type, temp_out_type,
					  &temp_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_LPAT: 
	        temp_out = esif_xform_lpat(temp_in, lp_ptr->dsp_ptr);
		temp_in_type = ESIF_TEMP_MILLIC;
		temp_out_type = type;
		esif_convert_temp(temp_in_type, temp_out_type, &temp_out);
		break;
	
	default:
		ESIF_TRACE_DYN_POWER(
			"%s: Unknown algorithm (%s) to xform temp\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->temp_xform));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_TEMP("%s: IN  temp %u %s(%d)\n", ESIF_FUNC, temp_in,
			    esif_temperature_type_desc(
				    temp_in_type), temp_in_type);

	ESIF_TRACE_DYN_TEMP("%s: OUT temp %u %s(%d)\n", ESIF_FUNC, temp_out,
			    esif_temperature_type_desc(
				    temp_out_type), temp_out_type);

	*temp_ptr = temp_out;
	return rc;
}


/* Power Transform */
static ESIF_INLINE enum esif_rc esif_xform_power(
	const enum esif_power_unit_type type,
	esif_power_t *power_ptr,
	const enum esif_action_type action,
	const struct esif_lp_dsp *dsp_ptr,
	const enum esif_primitive_opcode opcode
	)
{
	enum esif_power_unit_type power_in_type  = type;
	enum esif_power_unit_type power_out_type = type;
	enum esif_rc rc = ESIF_OK;
	struct esif_cpc_algorithm *algo_ptr = NULL;
	esif_power_t power_in;
	esif_power_t power_out;

	algo_ptr = dsp_ptr->get_algorithm(dsp_ptr, action);
	if (algo_ptr == NULL)
		return ESIF_E_NEED_ALGORITHM;

	power_in  = *power_ptr;
	power_out = *power_ptr;

	switch (algo_ptr->power_xform) {
	case ESIF_ALGORITHM_TYPE_POWER_DECIW:
		ESIF_TRACE_DYN_POWER(
			"%s: using algorithm DeciW (%s), for ACPI power\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->power_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			/* Tenths Of A Watt To Milli Watts */
			power_in_type  = ESIF_POWER_DECIW;
			power_out_type = type;
			/* Normalized from DeciW */
			esif_convert_power(power_in_type,
					   power_out_type,
					   &power_out);
		} else {
			/* Milli Watts To Tenths Of A Watt */
			power_in_type  = type;
			power_out_type = ESIF_POWER_DECIW;
			/* Normalized to DeciW */
			esif_convert_power(power_in_type,
					   power_out_type,
					   &power_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_MILLIW:
		ESIF_TRACE_DYN_POWER(
			"%s: using algorithm MillW (%s), for Code and Konst power\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->power_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			power_in_type  = ESIF_POWER_MILLIW;
			power_out_type = type;
			esif_convert_power(power_in_type,
					   power_out_type,
					   &power_out);
		} else {
			/* Milli Watts To Tenths Of A Watt */
			power_in_type  = type;
			power_out_type = ESIF_POWER_MILLIW;
			esif_convert_power(power_in_type,
					   power_out_type,
					   &power_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_UNIT_ATOM:

		/*
		 * Power 1mw * (2 ^ RAPL_POWER_UNIT)
		 * RAPL_POWER_UNIT = 5 example 1mw * (2 ^5) = 32mw
		 */
		ESIF_TRACE_DYN_POWER(
			"%s: using algorithm %s, for hardware power\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->power_xform));
		/* Hardware */
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			power_in_type  = ESIF_POWER_UNIT_ATOM;
			power_out_type = type;
			/* Normalized from hardware */
			esif_convert_power(power_in_type,
					   power_out_type,
					   &power_out);
		} else {
			power_in_type  = type;
			power_out_type = ESIF_POWER_UNIT_ATOM;
			/* Normalized to hardware */
			esif_convert_power(power_in_type,
					   power_out_type,
					   &power_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_UNIT_CORE:

		/*
		 * Power 1000mw / (2 ^ RAPL_POWER_UNIT)
		 * RAPL_POWER_LIMIT = 3 example 1000mw / (2 ^3) = 125mw
		*/
		ESIF_TRACE_DYN_POWER(
			"%s: using algorithm %s, for hardware power\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->power_xform));
		/* Hardware */
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			power_in_type  = ESIF_POWER_UNIT_CORE;
			power_out_type = type;
			/* Normalized from hardware */
			esif_convert_power(power_in_type,
					   power_out_type,
					   &power_out);
		} else {
			/* Milliwatts To hardware */
			power_in_type  = type;
			power_out_type = ESIF_POWER_UNIT_CORE;
			/* Normalized to OctaW */
			esif_convert_power(power_in_type,
					   power_out_type,
					   &power_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_NONE:
		/* No algorithm specified, do not perform any xform */
		ESIF_TRACE_DYN_POWER(
			"%s: using algorithm NONE (%s), no xform performed\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->power_xform));
		break;

	default:
		ESIF_TRACE_DYN_POWER(
			"%s: Unknown algorithm (%s) to xform power\n",
			ESIF_FUNC,
			esif_algorithm_type_str(algo_ptr->power_xform));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_POWER("%s: IN  power %u %s(%d)\n", ESIF_FUNC, power_in,
			     esif_power_unit_desc(power_in_type),
			     power_in_type);

	ESIF_TRACE_DYN_POWER("%s: OUT power %u %s(%d)\n", ESIF_FUNC, power_out,
			     esif_power_unit_desc(
				     power_out_type), power_out_type);

	*power_ptr = power_out;
	return rc;
}


enum esif_rc esif_lf_event(
	struct esif_participant_iface *pi_ptr,
	enum esif_event_type type,
	u16 domain,
	struct esif_data *data_ptr
	)
{
	enum esif_rc rc        = ESIF_OK;
	struct esif_lp *lp_ptr = NULL;
	struct esif_event *event_ptr         = NULL;
	struct esif_ipc *ipc_ptr             = NULL;
	struct esif_cpc_event *cpc_event_ptr = NULL;

	u16 ipc_data_len                     = 0;
	struct esif_ipc_event_header evt_hdr = {0};
	struct esif_ipc_event_data_create_participant evt_data = {0};

	lp_ptr = esif_lf_pm_lp_get_by_pi(pi_ptr);
	if (NULL == lp_ptr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}

	/* Create An Event */
	switch (type) {
	/* Special Event Create Send Entire PI Information */
	case ESIF_EVENT_PARTICIPANT_CREATE:
	{
		event_ptr = esif_event_allocate(ESIF_EVENT_PARTICIPANT_CREATE,
					sizeof(struct esif_participant_iface),
					ESIF_EVENT_PRIORITY_NORMAL,
					lp_ptr->instance,
					ESIF_INSTANCE_UF,
					domain,
					pi_ptr);

		if (NULL == event_ptr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		ipc_data_len = sizeof(struct esif_ipc_event_header) +
			sizeof(struct esif_ipc_event_data_create_participant);
		ipc_ptr = esif_ipc_alloc(ESIF_IPC_TYPE_EVENT, ipc_data_len);

		if (NULL == ipc_ptr) {
			rc = ESIF_E_NO_MEMORY;
			esif_event_free(event_ptr);
			goto exit;
		}

		/* Setup Event Header And Copy To IPC Buffer */
		evt_hdr.version       = event_ptr->version;
		evt_hdr.type          = event_ptr->type;
		evt_hdr.id            = event_ptr->id;
		evt_hdr.timestamp     = event_ptr->timestamp;
		evt_hdr.priority      = event_ptr->priority;
		evt_hdr.src_id        = event_ptr->src;
		evt_hdr.dst_id        = event_ptr->dst;
		evt_hdr.dst_domain_id = 'NA';	/* event_ptr->dst_domain_id; */
		evt_hdr.data_len =
			sizeof(struct esif_ipc_event_data_create_participant);

		esif_ccb_memcpy(ipc_ptr + 1,
				&evt_hdr,
				sizeof(struct esif_ipc_event_header));

		/* Setup Event Data Structure */
		evt_data.id = lp_ptr->instance;
		evt_data.version        = lp_ptr->pi_ptr->version;
		evt_data.enumerator     = lp_ptr->pi_ptr->enumerator;
		evt_data.flags          = lp_ptr->pi_ptr->flags;
		evt_data.pci_vendor     = lp_ptr->pi_ptr->pci_vendor;
		evt_data.pci_device     = lp_ptr->pi_ptr->pci_device;
		evt_data.pci_bus        = lp_ptr->pi_ptr->pci_bus;
		evt_data.pci_bus_device = lp_ptr->pi_ptr->pci_bus_device;
		evt_data.pci_function   = lp_ptr->pi_ptr->pci_function;
		evt_data.pci_revision   = lp_ptr->pi_ptr->pci_revision;
		evt_data.pci_class      = lp_ptr->pi_ptr->pci_class;
		evt_data.pci_sub_class  = lp_ptr->pi_ptr->pci_sub_class;
		evt_data.pci_prog_if    = lp_ptr->pi_ptr->pci_prog_if;

		/*
		** Handle these slow but safe.
		*/
		esif_ccb_memcpy(&evt_data.class_guid,
				lp_ptr->pi_ptr->class_guid,
				ESIF_GUID_LEN);
		esif_ccb_memcpy(&evt_data.name,
				lp_ptr->pi_ptr->name,
				ESIF_NAME_LEN);
		esif_ccb_memcpy(&evt_data.desc,
				lp_ptr->pi_ptr->desc,
				ESIF_DESC_LEN);
		esif_ccb_memcpy(&evt_data.driver_name,
				lp_ptr->pi_ptr->driver_name,
				ESIF_NAME_LEN);
		esif_ccb_memcpy(&evt_data.device_name,
				lp_ptr->pi_ptr->device_name,
				ESIF_NAME_LEN);
		esif_ccb_memcpy(&evt_data.device_path,
				lp_ptr->pi_ptr->device_path,
				ESIF_PATH_LEN);
		esif_ccb_memcpy(&evt_data.acpi_device,
				lp_ptr->pi_ptr->acpi_device,
				ESIF_SCOPE_LEN);
		esif_ccb_memcpy(&evt_data.acpi_scope,
				lp_ptr->pi_ptr->acpi_scope,
				ESIF_SCOPE_LEN);
		evt_data.acpi_uid  = lp_ptr->pi_ptr->acpi_uid;
		evt_data.acpi_type = lp_ptr->pi_ptr->acpi_type;

		esif_ccb_memcpy(((u8 *)(ipc_ptr + 1) +
				   sizeof(struct esif_ipc_event_header)),
			&evt_data,
			sizeof(struct esif_ipc_event_data_create_participant));

		/* IPC will be freed on othre side of queue operation */
		esif_event_queue_push(ipc_ptr);
		break;
	}

	/* ACPI Events are special the incoming data will contain the OS notify
	 * type.
	 * We will map
	 * that to an ESIF DSP event with the aide of the DSP.
	 */
	case ESIF_EVENT_ACPI:
	{
		u32 acpi_notify = 0;

		/* Map ACPI into ESIF event number (if DSP is loaded) in
		 *interrupt context */
		if ((NULL != lp_ptr) &&
		    (NULL != lp_ptr->dsp_ptr) &&
		    (NULL != data_ptr)) {
			/* Be paranoid make sue this is truly our EVENT data */
			if (data_ptr->buf_len == sizeof(u32) &&
			    data_ptr->data_len == sizeof(u32) &&
			    ESIF_DATA_UINT32 == data_ptr->type) {
				acpi_notify = *(u32 *)data_ptr->buf_ptr;
			}

			cpc_event_ptr = lp_ptr->dsp_ptr->get_event(
					lp_ptr->dsp_ptr,
					acpi_notify);
			if (NULL != cpc_event_ptr) {
				/* Translate ESIF_EVENT_ACPI->real ESIF event */
				type = cpc_event_ptr->esif_event;
				ESIF_TRACE_DYN_EVENT(
					"%s: Mapped ACPI event 0x%08x to "
					"ESIF Event %s(%d)\n",
					ESIF_FUNC,
					acpi_notify,
					esif_event_type_str(type),
					type);
			} else {
				ESIF_TRACE_DYN_EVENT(
					"%s: Undefined ACPI event 0x%08x in "
					"DSP! Cannot map to ESIF event!\n",
					ESIF_FUNC,
					acpi_notify);
				rc = ESIF_E_UNSPECIFIED;
				goto exit;
			}
		} else {
			ESIF_TRACE_DYN_EVENT(
				"%s: lp_ptr %p dsp_ptr %p data_ptr %p, none "
				"cannot be null\n",
				ESIF_FUNC,
				lp_ptr,
				lp_ptr->dsp_ptr,
				data_ptr);
			rc = ESIF_E_UNSPECIFIED;
			goto exit;
		}
	}
	/* BREAK INTENTIONALLY LEFT OUT */

	/* Everything Else Just Send The Handle */
	default:
	{
		event_ptr = esif_event_allocate(type,
						sizeof(lp_ptr->instance),
						ESIF_EVENT_PRIORITY_NORMAL,
						lp_ptr->instance,
						lp_ptr->instance,
						'NA',
						&lp_ptr->instance);
		if (NULL == event_ptr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		ipc_data_len = sizeof(struct esif_ipc_event_header);
		ipc_ptr = esif_ipc_alloc(ESIF_IPC_TYPE_EVENT, ipc_data_len);

		if (NULL == ipc_ptr) {
			esif_event_free(event_ptr);
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		/* Setup Event Header And Copy To IPC Buffer */
		evt_hdr.version       = event_ptr->version;
		evt_hdr.type          = event_ptr->type;
		evt_hdr.id            = event_ptr->id;
		evt_hdr.timestamp     = event_ptr->timestamp;
		evt_hdr.priority      = event_ptr->priority;
		evt_hdr.src_id        = event_ptr->src;
		evt_hdr.dst_id        = event_ptr->dst;
		evt_hdr.dst_domain_id = 'NA';	/* event_ptr->dst_domain_id; */
		evt_hdr.data_len      = 0;

		esif_ccb_memcpy(ipc_ptr + 1,
				&evt_hdr,
				sizeof(struct esif_ipc_event_header));

		/* IPC will be freed on other side of queue operation */
		esif_event_queue_push(ipc_ptr);
	}
	}	/* End of case */

	/* Send Our Event */
	ESIF_TRACE_DYN_LF("%s: type %s(%d)\n", ESIF_FUNC,
			  esif_event_type_str(type), type);

	esif_lf_send_all_events_in_queue_to_uf_by_ipc();
	esif_lf_send_event(lp_ptr->pi_ptr, event_ptr);
	esif_event_free(event_ptr);

exit:
	return rc;
}


/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */

/* ESIF Stats */
struct esif_memory_stats g_memstat = {0};
esif_ccb_lock_t g_memstat_lock;

/* ESIF Memory */
struct esif_ccb_mempool *g_mempool[ESIF_MEMPOOL_TYPE_MAX] = {0};
esif_ccb_lock_t g_mempool_lock;

struct esif_ccb_memtype *g_memtype[ESIF_MEMTYPE_TYPE_MAX] = {0};
esif_ccb_lock_t g_memtype_lock;

/* Register Participant */
enum esif_rc esif_lf_register_participant(struct esif_participant_iface *pi_ptr)
{
	enum esif_rc rc        = ESIF_OK;
	struct esif_lp *lp_ptr = NULL;
	ESIF_TRACE_DYN_LF("%s: Register Send_Event Handler\n", ESIF_FUNC);

	ESIF_TRACE_DYN_DECODE(
		"Version:        %d\n"
		"GUID:           %08x.%08x.%08x.%08x\n"
		"Enumerator:     %s(%d)\n"
		"Flags:          %08x\n"
		"Name:           %s\n"
		"Driver Name:    %s\n"
		"Device Name:    %s\n"
		"Device Path:    %s\n"
		"ACPI Device:    %s\n"
		"ACPI Scope:     %s\n"
		"ACPI UID:       %x\n"
		"ACPI Type:      %x\n"
		"PCI Vendor:     %x\n"
		"PCI Device:     %x\n"
		"PCI Bus:        %x\n"
		"PCI Bus Device: %x\n"
		"PCI Function:   %x\n"
		"PCI Revision:   %x\n"
		"PCI Class:      %x\n"
		"PCI SubClass:   %x\n"
		"PCI ProgIF:     %x\n",
		pi_ptr->version,
		*((int *)&pi_ptr->class_guid[0]),
		*((int *)&pi_ptr->class_guid[4]),
		*((int *)&pi_ptr->class_guid[8]),
		*((int *)&pi_ptr->class_guid[12]),
		esif_participant_enum_str(pi_ptr->enumerator),
		pi_ptr->enumerator,
		pi_ptr->flags,
		pi_ptr->name,
		pi_ptr->driver_name,
		pi_ptr->device_name,
		pi_ptr->device_path,
		pi_ptr->acpi_device,
		pi_ptr->acpi_scope,
		pi_ptr->acpi_uid,
		pi_ptr->acpi_type,
		pi_ptr->pci_vendor,
		pi_ptr->pci_device,
		pi_ptr->pci_bus,
		pi_ptr->pci_bus_device,
		pi_ptr->pci_function,
		pi_ptr->pci_revision,
		pi_ptr->pci_class,
		pi_ptr->pci_sub_class,
		pi_ptr->pci_prog_if);

	lp_ptr = esif_lf_pm_lp_create(pi_ptr);
	if (NULL == lp_ptr) {
		rc = ESIF_E_NO_CREATE;
		goto exit;
	}

	pi_ptr->send_event  = esif_lf_event;
	lp_ptr->xform_temp  = esif_xform_temp;
	lp_ptr->xform_power = esif_xform_power;

	/* Transition State To Registering Next State Need DSP */
	rc = esif_lf_pm_lp_set_state(lp_ptr,
				     ESIF_PM_PARTICIPANT_STATE_REGISTERING);
	if (ESIF_OK != rc)
		goto exit;

	/* Notify Upper Framework */
	rc = esif_lf_event(pi_ptr, ESIF_EVENT_PARTICIPANT_CREATE, 'NA', NULL);
	if (ESIF_OK != rc)
		goto exit;

	/* Notify Driver  */
	pi_ptr->recv_event(ESIF_EVENT_PARTICIPANT_CREATE, 'NA', NULL);
	if (ESIF_OK != rc)
		goto exit;

	/* Transition State To REQUEST DSP Next State Registered */
	rc = esif_lf_pm_lp_set_state(lp_ptr,
				     ESIF_PM_PARTICIPANT_STATE_REQUESTDSP);
exit:
	return rc;
}


/* Unregister All Participants */
void esif_lf_unregister_all_participants()
{
	u8 i = 0;
	struct esif_participant_iface *pi_ptr = NULL;

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		pi_ptr = esif_lf_pm_pi_get_by_instance_id(i);
		if (pi_ptr != NULL)
			esif_lf_unregister_participant(pi_ptr);
	}
}


/* Unregister Participant */
enum esif_rc esif_lf_unregister_participant(
	struct esif_participant_iface *pi_ptr)
{
	enum esif_rc rc        = ESIF_OK;
	struct esif_lp *lp_ptr = esif_lf_pm_lp_get_by_pi(pi_ptr);

	if (NULL == lp_ptr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}

	if (NULL != lp_ptr->dsp_ptr)
		esif_dsp_unload(lp_ptr);

	ESIF_TRACE_DYN_LF("%s: instance %d.\n", ESIF_FUNC, lp_ptr->instance);

	/* Notify Upper Framework */
	rc = esif_lf_event(lp_ptr->pi_ptr,
			   ESIF_EVENT_PARTICIPANT_UNREGISTER,
			   'NA',
			   NULL);
	if (ESIF_OK != rc)
		goto exit;

	esif_lf_pm_lp_remove(lp_ptr);
exit:
	return rc;
}

/* Optional Low-Level ESIF LF OS Init. 
 * Call before any esif_ccb_malloc, esif_lf_init, or threads created
 */
static unsigned int os_init_count = 0;
void esif_lf_os_init()
{
	if (!os_init_count) {
		esif_ccb_lock_init(&g_memstat_lock);
		os_init_count = 1;
	} else {
		os_init_count++;
	}
}

/* Optional Low-Level ESIF LF OS Exit. 
 * Call after all esif_ccb_free, esif_lf_init, and threads exit
 */
void esif_lf_os_exit()
{
	if (os_init_count) {
		os_init_count--;
	} else {
		esif_ccb_lock_uninit(&g_memstat_lock);
		os_init_count = 0;
	}
}

/* Start all Initializers, Stopping and aborting all on first Error */
static enum esif_rc esif_start_initializers()
{
	enum esif_rc rc = ESIF_OK;
	int initializer_count = sizeof(esif_initializers) / sizeof(struct esif_init_module);
	int j;

	for (j = 0; j < initializer_count; j++) {
		rc = (*esif_initializers[j].init_func)();
		if (rc != ESIF_OK) {
			esif_stop_initializers();
			break;
		}
		esif_initializers[j].started = 1;
	}
	return rc;
}

/* Stop all Initializers that have been Started */
static void esif_stop_initializers()
{
	int initializer_count = sizeof(esif_initializers) / sizeof(struct esif_init_module);
	int j;

	/* Stop in reverse order */
	for (j = initializer_count-1; j >= 0; j--) {
		if (esif_initializers[j].started) {
			(*esif_initializers[j].exit_func)();
			esif_initializers[j].started = 0;
		}
	}
}

/* ESIF LF Init */
enum esif_rc esif_lf_init(u32 debug_mask)
{
	enum esif_rc rc = ESIF_OK;
	esif_lf_os_init();
	esif_ccb_mempool_init_tracking();
	esif_ccb_memtype_init_tracking();

	/* Static  Debug Table */
	esif_debug_init_module_categories();
	if (0 != debug_mask) {
		/* Modules */
		esif_debug_set_modules(debug_mask);
		/* CPC */
		esif_debug_set_module_category(ESIF_DEBUG_MOD_CPC,
					       ESIF_TRACE_CATEGORY_DEFAULT |
					       0xff);
		/* DSP */
		esif_debug_set_module_category(ESIF_DEBUG_MOD_DSP,
					       ESIF_TRACE_CATEGORY_DEFAULT |
					       0xff);
	}

	ESIF_TRACE_DYN_INIT("%s: Initialize Eco-System Independent Framework\n",
			    ESIF_FUNC);

	/* Call all Init functions listed in esif_initializers. Abort on 1st Error */
	rc = esif_start_initializers();
	return rc;
}


/* ESIF LF Exit */
void esif_lf_exit(void)
{
	esif_lf_unregister_all_participants();

	/* Call all Exit functions listed in esif_initializers */
	esif_stop_initializers();

	esif_ccb_mempool_uninit_tracking();
	esif_ccb_memtype_uninit_tracking();
	esif_lf_os_exit();

	ESIF_TRACE_DYN_INIT("%s: Exit Eco-System Independent Framework\n",
			    ESIF_FUNC);

	ESIF_TRACE_DEBUG("\nDUMP Memory Stats:\n"
			 "-----------------------\n");
	ESIF_TRACE_DEBUG("MemAllocs:        %d\n", g_memstat.allocs);
	ESIF_TRACE_DEBUG("MemFrees:         %d\n", g_memstat.frees);
	ESIF_TRACE_DEBUG("MemInuse:         %d\n",
			 g_memstat.allocs - g_memstat.frees);
	ESIF_TRACE_DEBUG("MemPoolAllocs:    %d\n", g_memstat.memPoolAllocs);
	ESIF_TRACE_DEBUG("MemPoolFrees:     %d\n", g_memstat.memPoolFrees);
	ESIF_TRACE_DEBUG("MemPoolInuse:     %d\n",
			 g_memstat.memPoolAllocs - g_memstat.memPoolFrees);
	ESIF_TRACE_DEBUG("MemPoolObjAllocs: %d\n", g_memstat.memPoolObjAllocs);
	ESIF_TRACE_DEBUG("MemPoolObjFrees:  %d\n", g_memstat.memPoolObjFrees);
	ESIF_TRACE_DEBUG("MemPoolObjInuse:  %d\n",
			 g_memstat.memPoolObjAllocs -
			 g_memstat.memPoolObjFrees);
	ESIF_TRACE_DEBUG("MemTypeAllocs:    %d\n", g_memstat.memTypeAllocs);
	ESIF_TRACE_DEBUG("MemTypeFrees:     %d\n", g_memstat.memTypeFrees);
	ESIF_TRACE_DEBUG("MemTypeInuse:     %d\n",
			 g_memstat.memTypeAllocs - g_memstat.memTypeFrees);
	ESIF_TRACE_DEBUG("MemTypeObjAllocs: %d\n", g_memstat.memTypeObjAllocs);
	ESIF_TRACE_DEBUG("MemTypeObjFrees:  %d\n", g_memstat.memTypeObjFrees);
	ESIF_TRACE_DEBUG("MemTypeObjInuse:  %d\n\n",
			 g_memstat.memTypeObjAllocs -
			 g_memstat.memTypeObjFrees);

	if (0 != (g_memstat.allocs - g_memstat.frees)) {
		ESIF_TRACE_DEBUG(
			"!!!!!!!! POTENTIAL MEMORY LEAK DETECTED !!!!!!!!\n\n");
	}
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
