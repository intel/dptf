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
#include "esif_action.h"	/* EISF Action                      */
#include "esif_participant.h"	/* EISF Participant                 */
#include "esif_lf_poll.h"	/* EISF Poll                        */

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


/* Temperature will be C and Power will be in mw */
enum esif_rc esif_set_action_code(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_lp_action *action_ptr,
	const struct esif_data *req_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u8 domain_index = 0;
	struct esif_lp_domain *lpd_ptr = NULL;
	esif_flags_t poll_mask         = 0;
	u32 method = action_ptr->get_p1_u32(action_ptr);

	if (NULL == lp_ptr || NULL == tuple_ptr || NULL == req_data_ptr)
		return ESIF_E_PARAMETER_IS_NULL;

	/* derrive our domain index */
	rc = esif_lp_domain_index(tuple_ptr->domain, &domain_index);
	if (ESIF_OK != rc || domain_index > lp_ptr->domain_count) {
		ESIF_TRACE_DYN_SET("%s: di %d\n", ESIF_FUNC, domain_index);
		return ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	lpd_ptr = &lp_ptr->domains[domain_index];

	ESIF_TRACE_DYN_SET("%s: method 0x%08x di %d\n",
			   ESIF_FUNC,
			   method,
			   domain_index);

	switch (method) {
	/* Set Power Trip Points */
	case '0TPS':	/* SPT0 */
		lpd_ptr->power_aux0 = *(u32 *)req_data_ptr->buf_ptr;
		ESIF_TRACE_DYN_SET(
			"%s: SET_POWER_THRESHOLD_0 for %s tuple (%d.%d.%d) "
			"di %d power %d (aux0) %s(%d)\n",
			ESIF_FUNC,
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
			"%s: SET_POWER_THRESHOLD_1 for %s tuple (%d.%d.%d) "
			"di %d power %d (aux1) %s(%d)\n",
			ESIF_FUNC,
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

		/* Reset State */
		lpd_ptr->temp_notify_sent = ESIF_FALSE;

		ESIF_TRACE_DYN_SET(
			"%s: SET_TEMPERATURE_THRESHOLD_0 for %s "
			"tuple (%d.%d.%d) di %d temp %d (aux0) in %s(%d)\n",
			ESIF_FUNC,
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

		/* Reset State */
		lpd_ptr->temp_notify_sent = ESIF_FALSE;

		ESIF_TRACE_DYN_SET(
			"%s: SET_TEMPERATURE_THRESHOLD_1 for %s "
			"tuple (%d.%d.%d) di %d temp %d (aux0) in %s(%d)\n",
			ESIF_FUNC,
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
			"%s: SET_PROC_LOGICAL_PROCESSOR_AFFINITY for %s "
			"tuple (%d.%d.%d) di %d affinity %08x\n",
			ESIF_FUNC,
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
			"%s: STOP_DOMAIN_POLLING for %s tuple (%d.%d.%d) "
			"di %d\n",
			ESIF_FUNC,
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index);

		/* Stop Domain Polling Thread If Necessary */
		esif_poll_stop(lpd_ptr);
	} else {
		ESIF_TRACE_DYN_SET(
			"%s: START_DOMAIN_POLLING for %s tuple (%d.%d.%d) "
			"di %d for %d msec\n",
			ESIF_FUNC,
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			lpd_ptr->timer_period_msec);

		/* Start Domain Polling Thread If Necessary */
		esif_poll_start(lpd_ptr);
	}
	return rc;
}

enum esif_rc esif_get_action_code(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_lp_action *action_ptr,
	const struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u32 val         = 0;
	u8 domain_index = 0;
	struct esif_lp_domain *lpd_ptr = NULL;
	u8 cached       = ESIF_FALSE;
	u32 method      = action_ptr->get_p1_u32(action_ptr);

	UNREFERENCED_PARAMETER(req_data_ptr);

	if (NULL == lp_ptr || NULL == tuple_ptr || NULL == req_data_ptr ||
	    NULL == rsp_data_ptr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	/* derrive our domain index */
	rc = esif_lp_domain_index(tuple_ptr->domain, &domain_index);
	if (ESIF_OK != rc || domain_index > lp_ptr->domain_count) {
		ESIF_TRACE_DYN_SET("%s: di %d\n", ESIF_FUNC, domain_index);
		return ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	lpd_ptr = &lp_ptr->domains[domain_index];

	ESIF_TRACE_DYN_GET("%s: method 0x%08x di %d\n",
			   ESIF_FUNC,
			   method,
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
			ESIF_TRACE_DYN_SET("%s: addp d1 %d\n", ESIF_FUNC, d2);
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}

		rc = esif_lp_domain_index(dsp_p3, &d2);
		if (ESIF_OK != rc || d2 > lp_ptr->domain_count) {
			ESIF_TRACE_DYN_SET("%s: addp d2 %d\n", ESIF_FUNC, d2);
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
				"%s: Add/Sub Power %d = dmn[%d] pwr %d +/- dmn[%d] pwr %d In Non-Poll Mode\n",
				ESIF_FUNC,
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
				"%s: Add/Sub Power %d = dmn[%d] pwr %d +/- dmn[%d] pwr %d In Polling Mode\n",
				ESIF_FUNC,
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

		ESIF_TRACE_DYN_GET(
			"%s: Temperature MAX(domain1 %d, domain2 %d)\n",
			ESIF_FUNC,
			d1,
			d2);

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
			"%s: Temperature MAX(temp1 %d, temp2 %d, result %d)\n",
			ESIF_FUNC,
			t1,
			t2,
			val);
		break;
	}

	/* GRP0 Get RAPL Power D0 */
	case '0PRG':	/* GRP0 */
	{
		val = lp_ptr->domains[0].rapl_power;
		ESIF_TRACE_DYN_GET(
			"%s: GET_RAPL_POWER(D0) power %d in %s(%d)\n",
			ESIF_FUNC,
			val,
			esif_power_unit_desc(
				NORMALIZE_POWER_UNIT_TYPE),
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
		ESIF_TRACE_DYN_GET(
			"%s: GET_RAPL_POWER(D1) power %d in %s(%d)\n",
			ESIF_FUNC,
			val,
			esif_power_unit_desc(
				NORMALIZE_POWER_UNIT_TYPE),
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
		ESIF_TRACE_DYN_GET(
			"%s: GET_RAPL_POWER(D2) power %d in %s(%d)\n",
			ESIF_FUNC,
			val,
			esif_power_unit_desc(
				NORMALIZE_POWER_UNIT_TYPE),
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
			"%s: GET_TEMPERATURE_THRESHOLD_0 for %s tuple (%d.%d.%d) di %d cached %d temp %d (aux0) in %s(%d)\n",
			ESIF_FUNC,
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			cached,
			val,
			esif_temperature_type_desc(
				NORMALIZE_TEMP_TYPE),
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
			"%s: GET_TEMPERATURE_THRESHOLD_1 for %s tuple (%d.%d.%d) di %d cached %d temp %d (aux0) in %s(%d)\n",
			ESIF_FUNC,
			lpd_ptr->name_ptr,
			tuple_ptr->id,
			tuple_ptr->domain,
			tuple_ptr->instance,
			domain_index,
			cached,
			val,
			esif_temperature_type_desc(
				NORMALIZE_TEMP_TYPE),
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
			"%s: GET_POWER_THRESHOLD_0 for %s tuple (%d.%d.%d) di %d power %d (aux0) in %s(%d)\n",
			ESIF_FUNC,
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
	}

	/* Get Power Threshold 1 */
	case '1TPG':	/* GPT1 */
	{
		val = lpd_ptr->power_aux1;
		ESIF_TRACE_DYN_GET(
			"%s: GET_POWER_THRESHOLD_1 for %s tuple (%d.%d.%d) di %d power %d (aux1) in %s(%d)\n",
			ESIF_FUNC,
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
	}

	case 'FFAG':	/* GAFF */
	{
		val = g_affinity;
		ESIF_TRACE_DYN_GET(
			"%s: GET_PROC_LOGICAL_PROCESSOR_AFFINITY for %s tuple (%d.%d.%d) di %d affinity %08x\n",
			ESIF_FUNC,
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

	default:
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}

	ESIF_TRACE_DYN_GET("%s: type = %d, value = %d\n",
			   ESIF_FUNC,
			   rsp_data_ptr->type,
			   val);

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
		ESIF_TRACE_DYN_GET("%s: Data Type Not Implemented = %d\n",
				   ESIF_FUNC,
				   rsp_data_ptr->type);
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}
	
exit:
	return rc;
}


/* Init */
enum esif_rc esif_action_code_init(void)
{
	ESIF_TRACE_DYN_INIT("%s: Initialize CODE Action\n", ESIF_FUNC);
	return ESIF_OK;
}


/* Exit */
void esif_action_code_exit(void)
{
	ESIF_TRACE_DYN_INIT("%s: Exit CODE Action\n", ESIF_FUNC);
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
					ESIF_DATA_PERCENT,
					&right_spread_percent,
					sizeof(right_spread_percent));
	if(rc != ESIF_OK)
		goto exit;

	rc =  esif_get_simple_primitive(lp_ptr,
					GET_RFPROFILE_CLIP_PERCENT_LEFT,
					tuple_ptr->domain,
					255,
					ESIF_DATA_PERCENT,
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

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
