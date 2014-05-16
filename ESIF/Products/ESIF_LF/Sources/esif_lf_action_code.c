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
#include "esif_lf.h"		/* Lower Framework */
#include "esif_lf_action.h"	/* EISF Action                      */
#include "esif_participant.h"	/* EISF Participant                 */
#include "esif_lf_poll.h"	/* EISF Poll                        */
#include "esif_lf_ccb_gen_action.h"

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified against Windows SDK/DDK included headers which we
 * have no control over.
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Debug Logging Defintions */
#define INIT_DEBUG       0
#define GET_DEBUG        1
#define ESIF_DEBUG_MODULE ESIF_DEBUG_MOD_ACTION_CODE

#define ESIF_MSR_PLATFORM_INFO 0xCE
#define ESIF_MSR_BIT_FIVR_RFI_TUNING_AVAIL (1LL << 25)

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_CODE, \
		       INIT_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_GET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_CODE, \
		       GET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_SET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_CODE, \
		       GET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)

/* TODO Temporary */
u32 g_affinity;

/* Static LPAT 
 * 
 * VS 2012 C++ doesn't support C99 designated initializer style, aka
 * 
 *    union foo {int a, struct {int x, long long y} s; };
 * 
 *    union foo data[] = {{.s.x=1, .s.y=2}, ...}  GCC... OK
 *    union foo data[] = {{.s.x=1, .s.y=2}, ...}  VS ... ERROR 
 * 
 * Without designated initializer, static table will always start assignment 
 * from echo first member in union, no way to each struct member, so we'll 
 * just have to create a struct to do that. 
 */
#pragma pack(push, 1)
struct esif_data_variant_integer {  /* Same as esif_data_variant.integer */
        enum esif_data_type  type;  /* 4 Bytes For Most Compilers */
        long long value;            /* 8 Byte Integer */
};
#pragma pack(pop)

/* Static LPAT */
static struct esif_data_variant_integer esif_lpat_table[] = {
	{ESIF_DATA_UINT64, 2531}, {ESIF_DATA_UINT64, 976},
        {ESIF_DATA_UINT64, 2581}, {ESIF_DATA_UINT64, 960},
        {ESIF_DATA_UINT64, 2631}, {ESIF_DATA_UINT64, 940},
        {ESIF_DATA_UINT64, 2681}, {ESIF_DATA_UINT64, 916},
        {ESIF_DATA_UINT64, 2731}, {ESIF_DATA_UINT64, 884},
        {ESIF_DATA_UINT64, 2781}, {ESIF_DATA_UINT64, 852},
        {ESIF_DATA_UINT64, 2831}, {ESIF_DATA_UINT64, 812},
        {ESIF_DATA_UINT64, 2881}, {ESIF_DATA_UINT64, 768},
        {ESIF_DATA_UINT64, 2931}, {ESIF_DATA_UINT64, 720},
        {ESIF_DATA_UINT64, 2981}, {ESIF_DATA_UINT64, 668},
        {ESIF_DATA_UINT64, 3031}, {ESIF_DATA_UINT64, 612},
        {ESIF_DATA_UINT64, 3081}, {ESIF_DATA_UINT64, 560},
        {ESIF_DATA_UINT64, 3131}, {ESIF_DATA_UINT64, 508},
        {ESIF_DATA_UINT64, 3181}, {ESIF_DATA_UINT64, 456},
        {ESIF_DATA_UINT64, 3231}, {ESIF_DATA_UINT64, 404},
        {ESIF_DATA_UINT64, 3281}, {ESIF_DATA_UINT64, 356},
        {ESIF_DATA_UINT64, 3331}, {ESIF_DATA_UINT64, 312},
        {ESIF_DATA_UINT64, 3381}, {ESIF_DATA_UINT64, 276},
        {ESIF_DATA_UINT64, 3431}, {ESIF_DATA_UINT64, 240},
        {ESIF_DATA_UINT64, 3481}, {ESIF_DATA_UINT64, 212},
        {ESIF_DATA_UINT64, 3531}, {ESIF_DATA_UINT64, 184},
        {ESIF_DATA_UINT64, 3581}, {ESIF_DATA_UINT64, 160},
        {ESIF_DATA_UINT64, 3631}, {ESIF_DATA_UINT64, 140},
        {ESIF_DATA_UINT64, 3731}, {ESIF_DATA_UINT64, 104},
};

#define GFX_PSTATE_FREQ_INCREMENT	50 /* Increment in 50Mhz units */
#define GFX_PSTATE_FREQ_RPX_CONV_FACTOR 50 /* Provided in 50MHz units */

struct gfx_freq_range {
	u32 rp0_freq;
	u32 rp1_freq;
	u32 rpn_freq;
	u32 freq_increment;
};

extern enum esif_rc read_mod_write_msr_bit_range(
	const u32 msr,
	const u8 bit_from,
	const u8 bit_to,
	u64 val0
);

static enum esif_rc esif_set_action_code_fndg(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_lp_action *action_ptr,
	const struct esif_data *req_data_ptr
);

static enum esif_rc esif_set_action_code_rpcp(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_lp_action *action_ptr,
	const struct esif_data *req_data_ptr
);
	
static enum esif_rc esif_get_action_code_rpcp(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_lp_action *action_ptr,
	struct esif_data *rsp_data_ptr
);

static u32 calc_num_freqs_in_gfx_range(
	struct gfx_freq_range* freq_range_ptr
);

static enum esif_rc create_gfx_pstate_tbl_from_range_data (
	struct gfx_freq_range *freq_range_ptr,
	u32 **freq_tbl_ptr,
	u32 *num_entries_ptr
);


/* Temperature will be C and Power will be in mw */
enum esif_rc esif_set_action_code(
	struct esif_lp *lp_ptr,
	const struct esif_lp_primitive *primitive_ptr,
	const struct esif_lp_action *action_ptr,
	struct esif_data *req_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	const struct esif_primitive_tuple *tuple_ptr =  NULL;
	u32 method;
	u8 domain_index = 0;
	struct esif_lp_domain *lpd_ptr = NULL;
	esif_flags_t poll_mask         = 0;

	if ((NULL == lp_ptr) || (NULL == primitive_ptr) ||
	    (NULL == action_ptr) || (NULL == req_data_ptr)) {
		    rc = ESIF_E_PARAMETER_IS_NULL;
		    goto exit;
	}

	method = action_ptr->get_p1_u32(action_ptr);
	tuple_ptr =  &primitive_ptr->tuple;

	/* derrive our domain index */
	rc = esif_lp_domain_index(tuple_ptr->domain, &domain_index);
	if (ESIF_OK != rc || domain_index > lp_ptr->domain_count) {
		ESIF_TRACE_DYN_SET("Domain index out of bounds %d\n",
				    domain_index);
		return ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	lpd_ptr = &lp_ptr->domains[domain_index];

	ESIF_TRACE_DYN_SET("Method %c%c%c%c di %d\n",
			   ((u8 *)&method)[0],
			   ((u8 *)&method)[1],
			   ((u8 *)&method)[2],
			   ((u8 *)&method)[3],
			   domain_index);

	switch (method) {
	/* Set Power Trip Points */
	case '0TPS':	/* SPT0 */
		lpd_ptr->power_aux0 = *(u32 *)req_data_ptr->buf_ptr;
		ESIF_TRACE_DYN_SET(
			"SET_POWER_THRESHOLD_0 for %s tuple (%d.%d.%d) "
			"di %d power %d (aux0) %s(%d)\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			lpd_ptr->power_aux0,
			esif_power_unit_desc(
				NORMALIZE_POWER_UNIT_TYPE),
			NORMALIZE_POWER_UNIT_TYPE);
		break;

	case '1TPS':	/* SPT1 */
		lpd_ptr->power_aux1 = *(u32 *)req_data_ptr->buf_ptr;
		ESIF_TRACE_DYN_SET(
			"SET_POWER_THRESHOLD_1 for %s tuple (%d.%d.%d) "
			"di %d power %d (aux1) %s(%d)\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			lpd_ptr->power_aux1,
			esif_power_unit_desc(
				NORMALIZE_POWER_UNIT_TYPE),
			NORMALIZE_POWER_UNIT_TYPE);
		break;

	/* Set Temperature Trip Points */
	case '0TTS':	/* STT0 */
	{
		lpd_ptr->temp_aux0 = *(u32 *)req_data_ptr->buf_ptr;

		ESIF_TRACE_DYN_SET(
			"SET_TEMPERATURE_THRESHOLD_0 for %s "
			"tuple (%d.%d.%d) di %d temp %d (aux0) in %s(%d)\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			lpd_ptr->temp_aux0,
			esif_temperature_type_desc(
				NORMALIZE_TEMP_TYPE),
			NORMALIZE_TEMP_TYPE);
	}
	break;

	case '1TTS':	/* STT1 */
	{
		lpd_ptr->temp_aux1 = *(u32 *)req_data_ptr->buf_ptr;

		ESIF_TRACE_DYN_SET(
			"SET_TEMPERATURE_THRESHOLD_1 for %s "
			"tuple (%d.%d.%d) di %d temp %d (aux0) in %s(%d)\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			lpd_ptr->temp_aux1,
			esif_temperature_type_desc(
				NORMALIZE_TEMP_TYPE),
			NORMALIZE_TEMP_TYPE);
	}
	break;

	case 'FFAS':	/* SAFF */
		g_affinity = *(u32 *)req_data_ptr->buf_ptr;
		ESIF_TRACE_DYN_SET(
			"SET_PROC_LOGICAL_PROCESSOR_AFFINITY for %s "
			"tuple (%d.%d.%d) di %d affinity %08x\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			g_affinity);
		break;

	case 'GDNF':	/* FNDG */
		rc = esif_set_action_code_fndg(lp_ptr, tuple_ptr, action_ptr, req_data_ptr);
		break;

	case 'PCPR':	/* RPCP */
		rc = esif_set_action_code_rpcp(lp_ptr, tuple_ptr, action_ptr, req_data_ptr);
		break;

	default:
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}

	/*
	** Only poll what we have to
	*/
	if (lpd_ptr->temp_aux0 != ESIF_DOMAIN_TEMP_INVALID ||
	    lpd_ptr->temp_aux1 != ESIF_DOMAIN_TEMP_INVALID) {
		poll_mask |= ESIF_POLL_TEMPERATURE;
	}

	if (lpd_ptr->power_aux0 > 0 || lpd_ptr->power_aux1 > 0)
		poll_mask |= ESIF_POLL_POWER;

	lpd_ptr->poll_mask = poll_mask;

	if (0 == lpd_ptr->poll_mask) {
		ESIF_TRACE_DYN_SET(
			"STOP_DOMAIN_POLLING for %s tuple (%d.%d.%d) "
			"di %d\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index);

		/* Stop Domain Polling Thread If Necessary */
		esif_poll_stop(lpd_ptr);
	} else {
		ESIF_TRACE_DYN_SET(
			"START_DOMAIN_POLLING for %s tuple (%d.%d.%d) "
			"di %d for %d msec\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			lpd_ptr->timer_period_msec);

		/* Start Domain Polling Thread If Necessary */
		esif_poll_start(lpd_ptr);
	}
exit:
	ESIF_TRACE_DYN_SET("RC: %s(%d)\n", esif_rc_str(rc), rc);
	return rc;
}

enum esif_rc esif_get_action_code(
	struct esif_lp *lp_ptr,
	const struct esif_lp_primitive *primitive_ptr,
	const struct esif_lp_action *action_ptr,
	struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	const struct esif_primitive_tuple *tuple_ptr;
	u32 method;
	u32 val = 0;
	u8 domain_index = 0;
	const struct esif_lp_domain *lpd_ptr = NULL;
	u8 cached = ESIF_FALSE;

	UNREFERENCED_PARAMETER(req_data_ptr);

	if ((NULL == lp_ptr) || (NULL == primitive_ptr) ||
	    (NULL == action_ptr) || (NULL == rsp_data_ptr)) {
		    rc = ESIF_E_PARAMETER_IS_NULL;
		    goto exit;
	}

	tuple_ptr = &primitive_ptr->tuple;
	method = action_ptr->get_p1_u32(action_ptr);

	/* derrive our domain index */
	rc = esif_lp_domain_index(tuple_ptr->domain, &domain_index);
	if (ESIF_OK != rc || domain_index > lp_ptr->domain_count) {
		ESIF_TRACE_DYN_SET(
			"Domain index out of bounds - %d\n",
			domain_index);
		return ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	lpd_ptr = &lp_ptr->domains[domain_index];

	ESIF_TRACE_DYN_GET("Method %c%c%c%c di %d\n",
			   ((u8 *)&method)[0],
			   ((u8 *)&method)[1],
			   ((u8 *)&method)[2],
			   ((u8 *)&method)[3],
			   domain_index);

	switch (method) {
	/* Add or Substract Two Power Domain Values */
	case 'PDDA':  /* ADDP */
	case 'PBUS':  /* SUBP */
	{
		/* 1st and 2nd Domain in DSP p2 and P3: 'D1' and 'D2' */
		u16 dsp_p2 = (u16)action_ptr->get_p2_u32(action_ptr);
		u16 dsp_p3 = (u16)action_ptr->get_p3_u32(action_ptr);
		u8 d1 = 0;
		u8 d2 = 0;  /* Domain Number: 1,2,.. */

		rc = esif_lp_domain_index(dsp_p2, &d1);
		if (ESIF_OK != rc || d1 > lp_ptr->domain_count) {
			ESIF_TRACE_DYN_SET("addp d1 %d\n", d2);
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}

		rc = esif_lp_domain_index(dsp_p3, &d2);
		if (ESIF_OK != rc || d2 > lp_ptr->domain_count) {
			ESIF_TRACE_DYN_SET("addp d2 %d\n", d2);
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}


		/* Not In Polling Mode, Go Get The Value */
		if (0 == lpd_ptr->poll) {
			u32 pwr1, pwr2;
			struct esif_primitive_tuple tuple1 =
			{GET_RAPL_POWER, dsp_p2, 255};
			struct esif_primitive_tuple tuple2 =
			{GET_RAPL_POWER, dsp_p3, 255};

			struct esif_data data_in = {ESIF_DATA_VOID, NULL, 0, 0};
			struct esif_data data_out1 =
			{ESIF_DATA_UINT32, &pwr1, sizeof(pwr1), sizeof(pwr1)};
			struct esif_data data_out2 =
			{ESIF_DATA_UINT32, &pwr2, sizeof(pwr2), sizeof(pwr2)};

			rc = esif_execute_primitive(lp_ptr,
						    &tuple1,
						    &data_in,
						    &data_out1,
						    NULL);
			if (ESIF_OK != rc)
				goto exit;

			rc = esif_execute_primitive(lp_ptr,
						    &tuple2,
						    &data_in,
						    &data_out2,
						    NULL);
			if (ESIF_OK != rc)
				goto exit;

			if (method == 'PDDA')  /* addp */
				val = pwr1 + pwr2;
			else if (method == 'PBUS')  /* subp */
					val = pwr1 - pwr2;

			ESIF_TRACE_DYN_GET(
				"Add/Sub Power %d = dmn[%d] pwr %d +/- dmn[%d] pwr %d In Non-Poll Mode\n",
				val,
				d1,
				pwr1,
				d2,
				pwr2);
		} else {
			if (method == 'PDDA')
				val = lp_ptr->domains[d1].rapl_power +
				      lp_ptr->domains[d2].rapl_power;
			else if (method == 'PBUS')
				val = lp_ptr->domains[d1].rapl_power -
				      lp_ptr->domains[d2].rapl_power;

			ESIF_TRACE_DYN_GET(
				"Add/Sub Power %d = dmn[%d] pwr %d +/- dmn[%d] pwr %d In Polling Mode\n",
				val,
				d1,
				lp_ptr->domains[d1].rapl_power,
				d2,
				lp_ptr->domains[d2].rapl_power);
		}
		break;
	}

	/* MAX Of Two Temperatures e.g. DTS0, DTS1 */
	case 'TXAM':	/* MAXT */
	{
		enum esif_rc rc = ESIF_OK;

		/* First and Second domain */
		u16 d1 = (u16)action_ptr->get_p2_u32(action_ptr);
		u16 d2 = (u16)action_ptr->get_p3_u32(action_ptr);
		u32 t1 = 0;   /* First Domain Temperature */
		u32 t2 = 0;   /* Second Domain Temperature */

		struct esif_primitive_tuple tuple1 = {GET_TEMPERATURE, d1, 255};
		struct esif_primitive_tuple tuple2 = {GET_TEMPERATURE, d2, 255};
		struct esif_data t_data_in = {ESIF_DATA_VOID, NULL, 0, 0};
		struct esif_data t1_data_out = {ESIF_DATA_TEMPERATURE, &t1,
				sizeof(t1), sizeof(t1)};
		struct esif_data t2_data_out = {ESIF_DATA_TEMPERATURE, &t2,
				sizeof(t2), sizeof(t2)};

		ESIF_TRACE_DYN_GET("Temperature MAX(domain1 %d, domain2 %d)\n",
				    d1, d2);

		rc = esif_execute_primitive(lp_ptr,
					    &tuple1,
					    &t_data_in,
					    &t1_data_out,
					    NULL);
		if (ESIF_OK != rc)
			goto exit;

		rc = esif_execute_primitive(lp_ptr,
					    &tuple2,
					    &t_data_in,
					    &t2_data_out,
					    NULL);
		if (ESIF_OK != rc)
			goto exit;

		/* okay now compare temperature */
		val = esif_ccb_max(t1, t2);

		ESIF_TRACE_DYN_GET(
			"Temperature MAX(temp1 %d, temp2 %d, result %d)\n",
			t1, t2, val);
		break;
	}

	/* GRP0 Get RAPL Power D0 */
	case '0PRG':	/* GRP0 */
	{
		val = lp_ptr->domains[0].rapl_power;
		ESIF_TRACE_DYN_GET("GET_RAPL_POWER(D0) power %d in %s(%d)\n",
			val,
			esif_power_unit_desc(NORMALIZE_POWER_UNIT_TYPE),
			NORMALIZE_POWER_UNIT_TYPE);

		/*
		 * In polling mode, if number of samples aren't ready enough to
		 * count the power, return an I-AGAIN information.
		*/
		if ((lpd_ptr->poll_mask & ESIF_POLL_POWER) &&
		    lpd_ptr->rapl_energy_units_last == 0) {
			rc = ESIF_I_AGAIN;
		}
		break;
	}

	/* GRP1 Get RAPL Power D1 */
	case '1PRG':	/* GRP1 */
	{
		val = lp_ptr->domains[1].rapl_power;
		ESIF_TRACE_DYN_GET( "GET_RAPL_POWER(D1) power %d in %s(%d)\n",
			val,
			esif_power_unit_desc( NORMALIZE_POWER_UNIT_TYPE),
			NORMALIZE_POWER_UNIT_TYPE);

		if ((lpd_ptr->poll_mask & ESIF_POLL_POWER) &&
		    lpd_ptr->rapl_energy_units_last == 0) {
			rc = ESIF_I_AGAIN;
		}

		break;
	}

	/* GRP1 Get RAPL Power D2 */
	case '2PRG':	/* GRP2 */
	{
		val = lp_ptr->domains[2].rapl_power;
		ESIF_TRACE_DYN_GET( "GET_RAPL_POWER(D2) power %d in %s(%d)\n",
			val,
			esif_power_unit_desc(NORMALIZE_POWER_UNIT_TYPE),
			NORMALIZE_POWER_UNIT_TYPE);

		if ((lpd_ptr->poll_mask & ESIF_POLL_POWER) &&
		    lpd_ptr->rapl_energy_units_last == 0) {
			rc = ESIF_I_AGAIN;
		}
		break;
	}

	/* GTT0 Get Temperature Threshold 0 */
	case '0TTG':	/* GTT0 */
	{
		if (lpd_ptr->temp_cache0 > 0) {
			cached = ESIF_TRUE;
			val    = lpd_ptr->temp_cache0;
		} else {
			val = lpd_ptr->temp_aux0;
		}
		ESIF_TRACE_DYN_GET(
			"GET_TEMPERATURE_THRESHOLD_0 for %s tuple (%d.%d.%d) "
			"di %d cached %d temp %d (aux0) in %s(%d)\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			cached,
			val,
			esif_temperature_type_desc(NORMALIZE_TEMP_TYPE),
			NORMALIZE_TEMP_TYPE);

		if (0 == val)
			val = ESIF_DOMAIN_TEMP_INVALID;
		break;
	}

	/* GTT1 Get Temeperture Threshold 1 */
	case '1TTG':	/* GTT1 */
	{
		if (lpd_ptr->temp_cache1 > 0) {
			cached = ESIF_TRUE;
			val    = lpd_ptr->temp_cache1;
		} else {
			val = lpd_ptr->temp_aux1;
		}
		ESIF_TRACE_DYN_GET(
			"GET_TEMPERATURE_THRESHOLD_1 for %s tuple (%d.%d.%d) "
			"di %d cached %d temp %d (aux0) in %s(%d)\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			cached,
			val,
			esif_temperature_type_desc(NORMALIZE_TEMP_TYPE),
			NORMALIZE_TEMP_TYPE);

		if (0 == val)
			val = ESIF_DOMAIN_TEMP_INVALID;

		break;
	}

	/* Get Power Threshold 0 */
	case '0TPG':	/* GPT0 */
	{
		val = lpd_ptr->power_aux0;
		ESIF_TRACE_DYN_GET(
			"GET_POWER_THRESHOLD_0 for %s tuple (%d.%d.%d) "
			"di %d power %d (aux0) in %s(%d)\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			lpd_ptr->power_aux0,
			esif_power_unit_desc(NORMALIZE_POWER_UNIT_TYPE),
			NORMALIZE_POWER_UNIT_TYPE);
		break;
	}

	/* Get Power Threshold 1 */
	case '1TPG':	/* GPT1 */
	{
		val = lpd_ptr->power_aux1;
		ESIF_TRACE_DYN_GET(
			"GET_POWER_THRESHOLD_1 for %s tuple (%d.%d.%d) "
			"di %d power %d (aux1) in %s(%d)\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			lpd_ptr->power_aux1,
			esif_power_unit_desc(NORMALIZE_POWER_UNIT_TYPE),
			NORMALIZE_POWER_UNIT_TYPE);
		break;
	}

	case 'FFAG':	/* GAFF */
	{
		val = g_affinity;
		ESIF_TRACE_DYN_GET(
			"GET_PROC_LOGICAL_PROCESSOR_AFFINITY for %s "
			"tuple (%d.%d.%d) di %d affinity %08x\n",
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			g_affinity);
		break;
	}

	case 'TAPL':    /* LPAT */
	{
		struct esif_lp_dsp *dsp_ptr = lp_ptr->dsp_ptr;

		if (ESIF_FALSE == dsp_ptr->dsp_has_algorithm(dsp_ptr, 
					ESIF_ALGORITHM_TYPE_TEMP_LPAT)) {
			rc = ESIF_E_UNSUPPORTED_ALGORITHM;
			goto exit;
		}

		if (dsp_ptr->table) {
			/* Already Have A Table (ACPI or Static) */
			if (rsp_data_ptr->buf_len < dsp_ptr->table_size) {
				rc = ESIF_E_NEED_LARGER_BUFFER;
				goto exit;
			}
			esif_ccb_memcpy(rsp_data_ptr->buf_ptr, dsp_ptr->table, 
					dsp_ptr->table_size);
			rsp_data_ptr->data_len = dsp_ptr->table_size;	
		} else {
			/* No Table Yet, So Create A Static One */
			if (rsp_data_ptr->buf_len < sizeof(esif_lpat_table)) {
				rc = ESIF_E_NEED_LARGER_BUFFER;
				goto exit;
			}
			rsp_data_ptr->data_len = sizeof(esif_lpat_table);
			esif_ccb_memcpy(rsp_data_ptr->buf_ptr, &esif_lpat_table,
					rsp_data_ptr->data_len);
		}
		/* Done! Don't Deal With Any Type */
		goto exit;
	}

	case 'LTUP':	/* PUTL */
		rc = esif_get_action_code_putl(&val);
		break;

	case 'PCPR':	/* RPCP */
		rc = esif_get_action_code_rpcp(lp_ptr, tuple_ptr, action_ptr, rsp_data_ptr);
		goto exit;
		break;

	default:
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}

	ESIF_TRACE_DYN_GET("type = %s, value = %d\n",
			   esif_data_type_str(rsp_data_ptr->type), val);

	switch (rsp_data_ptr->type) {
	case ESIF_DATA_UINT8:
		rsp_data_ptr->data_len = sizeof(u8);
		if (rsp_data_ptr->buf_len >= sizeof(u8))
			*((u8 *)rsp_data_ptr->buf_ptr) = (u8)val;
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_UINT16:
		rsp_data_ptr->data_len = sizeof(u16);
		if (rsp_data_ptr->buf_len >= sizeof(u16))
			*((u16 *)rsp_data_ptr->buf_ptr) = (u16)val;
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_UINT32:
		rsp_data_ptr->data_len = sizeof(u32);
		if (rsp_data_ptr->buf_len >= sizeof(u32))
			*((u32 *)rsp_data_ptr->buf_ptr) = (u32)val;
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_UINT64:
	case ESIF_DATA_FREQUENCY:
		rsp_data_ptr->data_len = sizeof(u64);
		if (rsp_data_ptr->buf_len >= sizeof(u64))
			*((u64 *)rsp_data_ptr->buf_ptr) = (u64)val;
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	default:
		ESIF_TRACE_DYN_GET("Data Type Not Implemented = %s\n",
				   esif_data_type_str(rsp_data_ptr->type));
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}
	
exit:
	ESIF_TRACE_DYN_GET("RC: %s(%d)\n", esif_rc_str(rc), rc);
	return rc;
}


/* Init */
enum esif_rc esif_action_code_init(void)
{
	ESIF_TRACE_DYN_INIT("Initialize CODE Action\n");
	return ESIF_OK;
}


/* Exit */
void esif_action_code_exit(void)
{
	ESIF_TRACE_DYN_INIT("Exit CODE Action\n");
}


static enum esif_rc esif_set_action_code_fndg(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_lp_action *action_ptr,
	const struct esif_data *req_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u32 tuning_available = 0LL;
	long long center_freq = 0ULL;
	long long target_freq = 0LL;
	u32 left_spread_percent = 0ULL;
	u32 right_spread_percent = 0ULL; 
	long long left_spread_limit = 0LL;
	long long right_spread_limit = 0LL;
	long long delta = 0LL;
	long long result = 0LL;
	u32 msr = 0;
	u8 bit_from = 0;
	u8 bit_to = 0;

	if ((NULL == lp_ptr) || (NULL == tuple_ptr) ||
	    (NULL == action_ptr) || (NULL == req_data_ptr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/*
	 * Verify support first
	 */

	rc = esif_get_simple_primitive(lp_ptr,
				       GET_PROC_RF_TUNING_AVAILABLE,
				       tuple_ptr->domain,
				       255,
				       ESIF_DATA_UINT32,
				       &tuning_available,
				       sizeof(tuning_available));
	if(ESIF_OK != rc) {
		goto exit;
	}
	if(tuning_available == 0) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	/*
	 * Get the default frequency value for set point calculations.
	 */
	rc = esif_get_simple_primitive(lp_ptr,
				       GET_RFPROFILE_DEFAULT_CENTER_FREQUENCY,
				       tuple_ptr->domain,
				       255,
				       ESIF_DATA_FREQUENCY,
				       &center_freq,
				       sizeof(center_freq));
	if(rc != ESIF_OK)
		goto exit;

	if(center_freq == 0LL) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	/*
	 * Get the target frequency and determine the change in frequency from
	 * the center frequency.
	 */
	if(req_data_ptr->buf_len < 4) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	if(req_data_ptr->buf_len >= 8) {
		target_freq = (*(u64*)req_data_ptr->buf_ptr);
	} else {
		target_freq = (*(u32*)req_data_ptr->buf_ptr);
	}

	delta = target_freq - center_freq;

	/*
	 * Perform bounds checking on the requested frequency.
	 */
	rc =  esif_get_simple_primitive(lp_ptr,
					GET_RFPROFILE_CLIP_PERCENT_RIGHT,
					tuple_ptr->domain,
					255,
					ESIF_DATA_UINT32,
					&right_spread_percent,
					sizeof(right_spread_percent));
	if(rc != ESIF_OK)
		goto exit;

	rc =  esif_get_simple_primitive(lp_ptr,
					GET_RFPROFILE_CLIP_PERCENT_LEFT,
					tuple_ptr->domain,
					255,
					ESIF_DATA_UINT32,
					&left_spread_percent,
					sizeof(left_spread_percent));
	if(rc != ESIF_OK)
		goto exit;

	left_spread_limit = (left_spread_percent * center_freq) / 100 /
			     ESIF_PERCENT_CONV_FACTOR;

	right_spread_limit = (right_spread_percent * center_freq) / 100 /
			     ESIF_PERCENT_CONV_FACTOR;

	if((delta < -left_spread_limit) || (right_spread_limit < delta)) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}
	
	/*
	 * Calculate the required encoding.  From the BWG the encoding is found
	 * as int(value * 2^16 + 0.5); where value is the percentage from the
	 * center frequency . (Forumula shown is for a positive percentage, the
	 * forumula for a negative value not shown.) If we use the following:
	 * value = (target_freq - center_freq) / center_freq or
	 * value = delta / center_freq
	 * then
	 * result = ((delta * 2^16) / center_freq)  + 0.5
	 * if we say 0.5 = center_freq / (2 * center_freq)
	 * we can then combine the parts as follows
	 * result = (((delta * 2^16 ) * 2) + center_freq) / (2 * center_freq)
	 * and this all simplifies to the formulas used below...
	 */
	if(delta >= 0)
		result = ((delta << 17) + center_freq) / (2LL * center_freq);
	else
		result = ((delta << 17) - center_freq) / (2LL * center_freq);

	/*
	 * Get the MSR information for programming
	 */
	msr = action_ptr->get_p2_u32(action_ptr);
	bit_from = (u8)action_ptr->get_p4_u32(action_ptr);
	bit_to = (u8)action_ptr->get_p3_u32(action_ptr);

	rc = read_mod_write_msr_bit_range(msr, bit_from, bit_to, result);

exit:
	return rc;
}


static enum esif_rc esif_set_action_code_rpcp(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_lp_action *action_ptr,
	const struct esif_data *req_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u32 req_buf_len;
	u32 p_state_index;
	u8 *p_state_data_ptr = NULL;
	u32 p_state_setting = 0;
	union esif_data_variant *p_state_var_ptr = NULL;

	struct esif_primitive_tuple tuple_get_p_state_data = {0};
	struct esif_data esif_void = {ESIF_DATA_VOID, NULL, 0, 0};
	struct esif_data esif_p_state_data = {0};

	struct esif_primitive_tuple tuple_set_p_state = {0};
	struct esif_data esif_p_state_setting = {0};

	UNREFERENCED_PARAMETER(action_ptr);

	if ((NULL == lp_ptr) || (NULL == req_data_ptr) || (NULL == tuple_ptr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (req_data_ptr->buf_len < sizeof(u32)) {
		rc = ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE;
		goto exit;
	}

	if (NULL == req_data_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/*
	 * First get the required buffer size for the P-State data
	 */
	tuple_get_p_state_data.id =  GET_PERF_SUPPORT_STATES;
	tuple_get_p_state_data.domain = tuple_ptr->domain;
	tuple_get_p_state_data.instance = tuple_ptr->instance;

	esif_p_state_data.type = ESIF_DATA_BINARY;
	esif_p_state_data.buf_ptr = &p_state_index;
	esif_p_state_data.buf_len = sizeof(p_state_index);
	esif_p_state_data.data_len = 0;

	rc = esif_execute_primitive(lp_ptr,
				    &tuple_get_p_state_data,
				    &esif_void, &esif_p_state_data,
				    NULL);
	if(rc != ESIF_E_NEED_LARGER_BUFFER) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	req_buf_len = esif_p_state_data.data_len;

	/*
	 * Allocate our buffer based on the size returned
	 */
	if (req_buf_len == 0) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	p_state_data_ptr = esif_ccb_malloc(req_buf_len);
	if (NULL == p_state_data_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	/*
	 * Now get real data
	 */
	esif_p_state_data.type = ESIF_DATA_BINARY;
	esif_p_state_data.buf_ptr = p_state_data_ptr;
	esif_p_state_data.buf_len = req_buf_len;
	esif_p_state_data.data_len = 0;

	rc = esif_execute_primitive(lp_ptr,
				    &tuple_get_p_state_data,
				    &esif_void, &esif_p_state_data,
				    NULL);
	if(rc != ESIF_OK) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	/*
	 * Get the P-State setting based on the passed in index
	 */
	p_state_index = *((u32 *)req_data_ptr->buf_ptr);

	if (p_state_index >= 
		(esif_p_state_data.data_len / sizeof(*p_state_var_ptr))) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	p_state_var_ptr = (union esif_data_variant *) p_state_data_ptr;
	if (p_state_var_ptr[p_state_index].type != ESIF_DATA_UINT64) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	p_state_setting = (u32)(p_state_var_ptr[p_state_index].integer.value /
		          GFX_PSTATE_FREQ_RPX_CONV_FACTOR);

	ESIF_TRACE_DEBUG("Gfx frequency from table for index %d = %d", p_state_index, p_state_setting);

	/*
	 * Now we set the new P-State value
	 */
	tuple_set_p_state.id = SET_GFX_RPCP;
	tuple_set_p_state.domain = tuple_ptr->domain;
	tuple_set_p_state.instance = tuple_ptr->instance;

	esif_p_state_setting.type = ESIF_DATA_UINT32;
	esif_p_state_setting.buf_len = sizeof(p_state_setting);
	esif_p_state_setting.buf_ptr = &p_state_setting;
	esif_p_state_setting.data_len = esif_p_state_setting.buf_len;

	rc = esif_execute_primitive(lp_ptr,
				    &tuple_set_p_state,
				    &esif_p_state_setting, &esif_void,
				    NULL);

exit:
	if (NULL != p_state_data_ptr)
		esif_ccb_free(p_state_data_ptr);

	return rc;
}


static enum esif_rc esif_get_action_code_rpcp(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_lp_action *action_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	struct gfx_freq_range freq_range_data = {0};
	u32 num_freqs = 0;
	u32 *freq_tbl_ptr = NULL;
	union esif_data_variant *cur_resp_ptr = NULL;
	u32 data_size;
	u8 i;

	UNREFERENCED_PARAMETER(action_ptr);

	if ((NULL == lp_ptr) || (NULL == tuple_ptr) || (NULL == rsp_data_ptr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/*
	 * Get the upper and lower frequency limits
	 */
	rc = esif_get_simple_primitive(lp_ptr,
				       GET_PROC_RP_STATE_CAPABILITY,
				       tuple_ptr->domain,
				       0,
				       ESIF_DATA_UINT32,
				       &freq_range_data.rp0_freq,
				       sizeof(freq_range_data.rp0_freq));
	if(ESIF_OK != rc) {
		goto exit;
	}
	freq_range_data.rp0_freq *= GFX_PSTATE_FREQ_RPX_CONV_FACTOR;

	rc = esif_get_simple_primitive(lp_ptr,
				       GET_PROC_RP_STATE_CAPABILITY,
				       tuple_ptr->domain,
				       1,
				       ESIF_DATA_UINT32,
				       &freq_range_data.rp1_freq,
				       sizeof(freq_range_data.rp1_freq));
	if(ESIF_OK != rc) {
		goto exit;
	}
	freq_range_data.rp1_freq *= GFX_PSTATE_FREQ_RPX_CONV_FACTOR;

	rc = esif_get_simple_primitive(lp_ptr,
				       GET_PROC_RP_STATE_CAPABILITY,
				       tuple_ptr->domain,
				       2,
				       ESIF_DATA_UINT32,
				       &freq_range_data.rpn_freq,
				       sizeof(freq_range_data.rpn_freq));
	if(ESIF_OK != rc) {
		goto exit;
	}
	freq_range_data.rpn_freq *= GFX_PSTATE_FREQ_RPX_CONV_FACTOR;

	freq_range_data.freq_increment = GFX_PSTATE_FREQ_INCREMENT;
	if (freq_range_data.freq_increment > freq_range_data.rp0_freq) {
		freq_range_data.freq_increment = freq_range_data.rp0_freq;
	}

	ESIF_TRACE_DEBUG("RP0 = %d, RP1 = %d, RPn = %d, Increment = %d\n",
		freq_range_data.rp0_freq,
		freq_range_data.rp1_freq,
		freq_range_data.rpn_freq,
		freq_range_data.freq_increment);


	rc = create_gfx_pstate_tbl_from_range_data (&freq_range_data,
						    &freq_tbl_ptr,
						    &num_freqs);
	if (rc != ESIF_OK) {
		goto exit;
	}

	data_size = num_freqs * sizeof(union esif_data_variant);
	rsp_data_ptr->data_len = data_size;

	if (rsp_data_ptr->buf_len < data_size) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		/* May be called to get size required; so warn only. */
		ESIF_TRACE_WARN("Response buffer is too small, "
				 "%d available, %d required\n",
				 rsp_data_ptr->buf_len, data_size);
		goto exit;
	}

	//
	// Report the data as an array of variants
	//
	cur_resp_ptr = (union esif_data_variant*)rsp_data_ptr->buf_ptr;

	for (i = 0; i < num_freqs; i++) {
		cur_resp_ptr->type = ESIF_DATA_UINT64;
		cur_resp_ptr++->integer.value = (UINT64)freq_tbl_ptr[i];
	}

exit:
	if (freq_tbl_ptr != NULL) {
		esif_ccb_free(freq_tbl_ptr);
	}
	return rc;
}


static u32 calc_num_freqs_in_gfx_range(struct gfx_freq_range* freq_range_ptr)
{
	u32 num_freqs = 0;
	u32 freq_increment;

	if(NULL == freq_range_ptr) {
		goto exit;
	}

	if ((freq_range_ptr->rpn_freq > freq_range_ptr->rp1_freq) || 
	    (freq_range_ptr->rp1_freq > freq_range_ptr->rp0_freq)) {
		ESIF_TRACE_ERROR("Lower frequency above higher frequency.\n");
		goto exit;
	}

	if (freq_range_ptr->rp0_freq == 0) {
		ESIF_TRACE_ERROR("Frequency data is all 0.\n");
		goto exit;
	}

	/*
	 * Make sure the increment is a reasonable value or else the
	 * calculations below could be wrong. If so, change to a value that
	 * results in no intermediate frequencies
	 */
	freq_increment =  freq_range_ptr->freq_increment;
	if ((freq_increment == 0) || 
	    (freq_increment > freq_range_ptr->rp0_freq)) {
		freq_increment = freq_range_ptr->rp0_freq;
	}

	/*
	 * Calculate the number of frequencies we will report so that we can
	 * verify the buffer size before using it
	 */
	num_freqs = 1;	/* RP0 frequency */
	if (freq_range_ptr->rp0_freq > freq_range_ptr->rp1_freq) {
		/*
		 * Add the +1 to RP1 so the calculated mid frequencies doesn't
		 * include RP1 itself if the two are seperated by an exact
		 * multiple of the increment
		 */
		num_freqs += (freq_range_ptr->rp0_freq -
			     (freq_range_ptr->rp1_freq + 1)) /
			     freq_increment;
		num_freqs += 1;	/* RP1 frequency */
	}
	if (freq_range_ptr->rp1_freq > freq_range_ptr->rpn_freq) {
		/*
		 * Add the +1 to RPn so the calculated mid frequencies doesn't
		 * include RPn itself if the two are seperated by an exact
		 * multiple of the increment
		 */
		num_freqs += (freq_range_ptr->rp1_freq -
			     (freq_range_ptr->rpn_freq + 1)) /
			     freq_increment;
		num_freqs += 1;	/* RPn frequency */
	}

exit:
	ESIF_TRACE_DEBUG(" Num frequencies = %d\n", num_freqs);
	return num_freqs;
}


/*
 * WARNING:  It is the responsibility of the caller to free the returned table
 * pointer if not NULL.
*/
static enum esif_rc create_gfx_pstate_tbl_from_range_data (
	struct gfx_freq_range *freq_range_ptr,
	u32 **freq_tbl_ptr,
	u32 *num_entries_ptr
	)
{
	static enum esif_rc rc = ESIF_OK;
	u32 num_freqs;
	u32 *cur_entry_ptr = NULL;
	u32 freq_incr;
	u32 cur_freq;

	if ((NULL == freq_range_ptr) ||
	    (NULL == freq_tbl_ptr) ||
	    (NULL == num_entries_ptr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	num_freqs = calc_num_freqs_in_gfx_range(freq_range_ptr);
	if (num_freqs == 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	*freq_tbl_ptr = (u32*)esif_ccb_malloc(num_freqs * sizeof(**freq_tbl_ptr));
	if (*freq_tbl_ptr == NULL) {
		rc = ESIF_E_NO_MEMORY;
		ESIF_TRACE_ERROR("Resource allocation failure\n");
		goto exit;
	}

	freq_incr = freq_range_ptr->freq_increment;
	if ((freq_incr == 0) || (freq_incr > freq_range_ptr->rp0_freq)) {
		freq_incr = freq_range_ptr->rp0_freq;
	}

	//
	// Report the data as an array of variants
	//
	cur_entry_ptr = *freq_tbl_ptr;
	cur_freq = freq_range_ptr->rp0_freq;
	while ((cur_freq > freq_range_ptr->rp1_freq) &&
	       (cur_freq <= freq_range_ptr->rp0_freq)) {

		*cur_entry_ptr++ = cur_freq;
		ESIF_TRACE_DEBUG("Frequency = %d\n", cur_freq);
		cur_freq -= freq_incr;
	} 

	cur_freq = freq_range_ptr->rp1_freq;
	while ((cur_freq > freq_range_ptr->rpn_freq) &&
	       (cur_freq <= freq_range_ptr->rp1_freq)) {

		*cur_entry_ptr++ = cur_freq;
		ESIF_TRACE_DEBUG("Frequency = %d\n", cur_freq);
		cur_freq -= freq_incr;
	}

	if (cur_freq <= freq_range_ptr->rpn_freq) {
		*cur_entry_ptr++ = freq_range_ptr->rpn_freq;
		ESIF_TRACE_DEBUG("Frequency = %d\n", freq_range_ptr->rpn_freq);
	}

	*num_entries_ptr = num_freqs;

exit:
	return rc;
}



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
