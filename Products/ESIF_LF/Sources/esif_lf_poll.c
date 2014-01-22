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

#include "esif_lf_poll.h"

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified against Windows SDK/DDK included headers which
 * we have no control over.
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define INIT_DEBUG            0
#define POLL_DEBUG            1
#define RAPL_DEBUG            2
#define ENERGY_DEBUG          3

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_POL, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_POLL(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_POL, POLL_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_RAPL(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_POL, RAPL_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_ENERGY(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_POL, ENERGY_DEBUG, format, ##__VA_ARGS__)


/* TODO: Fix This */
int g_background = 1000;/* 1 Seconds */


/* Poll For Power and Send Event If Threshold Crossed */
static enum esif_rc esif_poll_power(
	struct esif_lp_domain *lpd_ptr
	)
{
	enum esif_rc rc       = ESIF_OK;
	u32 energy_units      = 0;
	u32 energy_units_used = 0;

	struct esif_primitive_tuple tuple = {GET_RAPL_ENERGY, lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0, 0};
	struct esif_data rsp_data = {ESIF_DATA_UINT32, &energy_units,
					sizeof(energy_units), 0};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);
	if (ESIF_OK != rc)
		goto exit;

	ESIF_TRACE_DYN_ENERGY(
		"%s: GET_RAPL_ENERGY for %s energy_unit 0x%x rc %s(%d)\n",
		ESIF_FUNC,
		lpd_ptr->name_ptr,
		energy_units,
		esif_rc_str(rc),
		rc);

	/* Save the current and last RAPL energy counters */
	lpd_ptr->rapl_energy_units_last    = lpd_ptr->rapl_energy_units_current;
	lpd_ptr->rapl_energy_units_current = energy_units;

	/* Check For Wrap? */
	if (lpd_ptr->rapl_energy_units_last > energy_units) {
		energy_units_used =
			(0xFFFFFFFF -
			 lpd_ptr->rapl_energy_units_last) + energy_units;
	} else {
		energy_units_used = energy_units -
			lpd_ptr->rapl_energy_units_last;
	}

	/* TODO: Use a timestamp to make this as acurate as possible */
	/* should be within a few usecs so good enough for now */
	lpd_ptr->rapl_energy_units_per_sec = energy_units_used /
		(lpd_ptr->timer_period_msec / 1000);

	ESIF_TRACE_DYN_ENERGY(
		"%s: id %s ENERGY units current %08X, last %08X, used %u, used/sec %d\n",
		ESIF_FUNC,
		lpd_ptr->name_ptr,
		lpd_ptr->rapl_energy_units_last,
		lpd_ptr->rapl_energy_units_current,
		energy_units_used,
		lpd_ptr->rapl_energy_units_per_sec);

	/* Take two samples to get the right rapl_power and send_event. */
	if (lpd_ptr->rapl_energy_units_last == 0)
		goto exit;

	/*
	 * Now Calculate Power
	 */
	if (lpd_ptr->rapl_energy_units_per_sec > 0) {
		u32 energy_joules = 0;/* default micro joules unit */
		u32 power         = 0; /* default power */
		struct esif_lp_domain *lpd_d0_ptr = NULL; /* For Domain 0 */

		/*
		* Don't HAVE Action Type Assume MSR since all processors
		* have this
		*/
		struct esif_cpc_algorithm *algo_ptr =
			lpd_ptr->lp_ptr->dsp_ptr->get_algorithm(
				lpd_ptr->lp_ptr->dsp_ptr,
				ESIF_ACTION_MSR);

		/* Power Unit Data Only Lives in Domain 0 (D0) */
		lpd_d0_ptr = &lpd_ptr->lp_ptr->domains[0];

		if (algo_ptr && ESIF_ALGORITHM_TYPE_POWER_UNIT_CORE ==
		    algo_ptr->power_xform) {
			if (lpd_d0_ptr->unit_energy) {
				energy_joules = 1000000 /
					(1 << lpd_d0_ptr->unit_energy);
			} else {
				/* 1000000uj / (2 ^ 14) = 61uj */
				energy_joules = 61;
			}
		}

		if (algo_ptr && ESIF_ALGORITHM_TYPE_POWER_UNIT_ATOM ==
		    algo_ptr->power_xform) {
			if (lpd_d0_ptr->unit_energy) {
				energy_joules = (1 << lpd_d0_ptr->unit_energy);
			} else {
				/* 1uj * (2 ^ 5) = 32 uj */
				energy_joules = 32;
			}
		}

		power = (lpd_ptr->rapl_energy_units_per_sec *
			 energy_joules) / 1000; /* .001 of watt accuracy */

		/* Normalized from DeciW */
		esif_convert_power(ESIF_POWER_MILLIW,
				   NORMALIZE_POWER_UNIT_TYPE,
				   &power);
		ESIF_TRACE_DYN_RAPL("%s: POWER %d %s(%d)\n",
				    ESIF_FUNC,
				    power,
				    esif_power_unit_desc(
					    NORMALIZE_POWER_UNIT_TYPE),
				    NORMALIZE_POWER_UNIT_TYPE);

		lpd_ptr->rapl_power = power;

		/*
		 * Now Check For Threshold
		 */
		ESIF_TRACE_DYN_RAPL(
			"%s: THRESHOLD_CHECK hyst = %d aux0 = %d "
			"power = %d aux1 = %d units %s(%d)\n",
			ESIF_FUNC,
			lpd_ptr->power_hysteresis,
			lpd_ptr->power_aux0,
			power,
			lpd_ptr->power_aux1,
			esif_power_unit_desc(
				NORMALIZE_POWER_UNIT_TYPE),
			NORMALIZE_POWER_UNIT_TYPE);

		if (0 == lpd_ptr->power_aux0 && 0 == lpd_ptr->power_aux1) {
			/* Do Nothing */
		} else {
			if (0 == lpd_ptr->power_aux0) {
				if (power > lpd_ptr->power_aux1) {
					lpd_ptr->lp_ptr->pi_ptr->send_event(
						lpd_ptr->lp_ptr->pi_ptr,
						ESIF_EVENT_DOMAIN_POWER_THRESHOLD_CROSSED,
						lpd_ptr->id,
						NULL);
				} else if (power < lpd_ptr->power_aux0 ||
					   power > lpd_ptr->power_aux1) {
					lpd_ptr->lp_ptr->pi_ptr->send_event(
						lpd_ptr->lp_ptr->pi_ptr,
						ESIF_EVENT_DOMAIN_POWER_THRESHOLD_CROSSED,
						lpd_ptr->id,
						NULL);
				}
			}
		}
	}
exit:
	return rc;
}


/* Poll For Temperature and Send Event If Threshold Crossed */
static enum esif_rc esif_poll_temperature(
	struct esif_lp_domain *lpd_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u32 temp        = 0;

	struct esif_primitive_tuple tuple = {GET_TEMPERATURE, lpd_ptr->id, 255};
	struct esif_data req_data = {ESIF_DATA_VOID, NULL, 0, 0};
	struct esif_data rsp_data = {ESIF_DATA_TEMPERATURE, &temp,
					sizeof(temp), 0};

	rc = esif_execute_primitive(lpd_ptr->lp_ptr,
				    &tuple,
				    &req_data,
				    &rsp_data,
				    NULL);
	if (ESIF_OK != rc)
		goto exit;

	ESIF_TRACE_DYN_TEMP("%s: TEMPERATURE %d %s(%d)\n",
			    ESIF_FUNC,
			    temp,
			    esif_temperature_type_desc(NORMALIZE_TEMP_TYPE),
			    NORMALIZE_TEMP_TYPE);

	ESIF_TRACE_DYN_TEMP(
		"%s: THRESHOLD_CHECK hyst = %d aux0 = %d temp = %d aux1 = %d units %s(%d)\n",
		ESIF_FUNC,
		lpd_ptr->temp_hysteresis,
		lpd_ptr->temp_aux0,
		temp,
		lpd_ptr->temp_aux1,
		esif_temperature_type_desc(NORMALIZE_TEMP_TYPE),
		NORMALIZE_TEMP_TYPE);

	if (ESIF_TRUE == lpd_ptr->temp_notify_sent) {
		ESIF_TRACE_DYN_TEMP(
			"%s: SKIPPING ESIF_EVENT_TEMP_THRESHOLD_CROSSED already sent\n",
			ESIF_FUNC);
		goto exit;
	}

	if (((lpd_ptr->temp_aux0 != ESIF_DOMAIN_TEMP_INVALID) &&
	     (temp < lpd_ptr->temp_aux0 - lpd_ptr->temp_hysteresis)) ||
	    ((lpd_ptr->temp_aux1 != ESIF_DOMAIN_TEMP_INVALID) &&
	     (temp >= lpd_ptr->temp_aux1))) {
		lpd_ptr->lp_ptr->pi_ptr->send_event(lpd_ptr->lp_ptr->pi_ptr,
						    ESIF_EVENT_DOMAIN_TEMP_THRESHOLD_CROSSED,
						    lpd_ptr->id,
						    NULL);

		lpd_ptr->temp_notify_sent = ESIF_TRUE;
		ESIF_TRACE_DYN_TEMP(
			"%s: ESIF_EVENT_TEMP_THRESHOLD_CROSSED sent\n",
			ESIF_FUNC);
	}
exit:
	return rc;
}


void esif_poll(
	void *context_ptr
	)
{
	struct esif_lp_domain *lpd_ptr = (struct esif_lp_domain *)context_ptr;

	if (NULL == lpd_ptr)
		return;

	ESIF_TRACE_DYN_POLL("%s: Timer %s:%s\n",
			    ESIF_FUNC,
			    lpd_ptr->lp_ptr->pi_name,
			    lpd_ptr->name_ptr);

	/* No DSP No Work */
	if (NULL != lpd_ptr->lp_ptr->dsp_ptr) {
		/* Do Work By Capability In Prioritized Order */
		if (lpd_ptr->poll_mask & ESIF_POLL_POWER)
			esif_poll_power(lpd_ptr);

		if (lpd_ptr->poll_mask & ESIF_POLL_TEMPERATURE)
			esif_poll_temperature(lpd_ptr);
	} else {
		ESIF_TRACE_DYN_POLL("%s: no DSP can't do work\n", ESIF_FUNC);
	}

	/* RESET The Timer */
	esif_ccb_timer_set_msec(&lpd_ptr->timer,
				lpd_ptr->timer_period_msec,
				esif_poll,
				lpd_ptr);
}


/* Start Poll */
void esif_poll_start(
	struct esif_lp_domain *lpd_ptr
	)
{
	if (ESIF_TRUE == lpd_ptr->poll || 0 == g_background)
		return;

	lpd_ptr->timer_period_msec = g_background;
	esif_ccb_timer_init(&lpd_ptr->timer);
	esif_ccb_timer_set_msec(&lpd_ptr->timer,
				lpd_ptr->timer_period_msec,
				esif_poll,
				lpd_ptr);

	ESIF_TRACE_DYN_POLL("%s: Timer started for %s period %d\n",
			    ESIF_FUNC, lpd_ptr->name_ptr, g_background);
	lpd_ptr->poll = ESIF_TRUE;
}


/* Stop All Poll Domain For Participants Instance */
void esif_poll_start_all(
	struct esif_lp *lp_ptr
	)
{
	u8 domain_index = 0;

	for (domain_index = 0; domain_index < lp_ptr->domain_count;
	     domain_index++)
		esif_poll_start(&lp_ptr->domains[domain_index]);
}


/* Stop Poll */
void esif_poll_stop(
	struct esif_lp_domain *lpd_ptr
	)
{
	if (ESIF_FALSE == lpd_ptr->poll)
		return;

	esif_ccb_timer_kill(&lpd_ptr->timer);

	ESIF_TRACE_DYN_POLL("%s: Timer stopped for %s period %d\n",
			    ESIF_FUNC, lpd_ptr->name_ptr, g_background);

	/* Reset Power History */
	lpd_ptr->rapl_energy_units_last    = 0;
	lpd_ptr->rapl_energy_units_current = 0;
	lpd_ptr->rapl_power = 0;
	lpd_ptr->poll = ESIF_FALSE;
}


/* Initialize Poll Manager */
enum esif_rc esif_poll_init(void)
{
	return ESIF_OK;
}


/* Exit Poll Manager */
void esif_poll_exit(void)
{
	ESIF_TRACE_DYN_INIT("%s: Exit Polling\n", ESIF_FUNC);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
