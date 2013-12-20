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

#include "esif_pm.h"
#include "esif_lf.h"
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
#define PM_DEBUG              1

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_PMG, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_PM(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_PMG, PM_DEBUG, format, ##__VA_ARGS__)

/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */

/* Participant Manager Entry */
struct esif_pme {
	enum esif_pm_participant_state  state;	/* Current Participant State */
	struct esif_participant_iface   *pi_ptr; /* Participant INTERFACE */
	struct esif_lp *lp_ptr; /* Lower Participant Instance */
};

/* Package Manager */
struct esif_pm {
	u8  pme_count;                            /* Current Reference Count */
	struct esif_pme  pme[MAX_PARTICIPANT_ENTRY]; /* Maximum Participants */
	esif_ccb_lock_t  lock;                       /* Package Manager Lock */
};

/*
** Package Manager. Keeps Track of all participant interfaces, lower
** participants and there corresponding state
*/
static struct esif_pm g_pm = {0};

/* Allocate Lower Participant Instance */
static struct esif_lp *esif_pm_lp_alloc(void)
{
	struct esif_lp *lp_ptr = NULL;

	esif_ccb_read_lock(&g_mempool_lock);
	lp_ptr = (struct esif_lp *)
		esif_ccb_mempool_zalloc(g_mempool[ESIF_MEMPOOL_TYPE_PM]);
	esif_ccb_read_unlock(&g_mempool_lock);

	return lp_ptr;
}


/* Free Lower Participant Instance */
static void esif_pm_lp_free(struct esif_lp *lp_ptr)
{
	if (NULL == lp_ptr)
		return;

	esif_ccb_read_lock(&g_mempool_lock);
	esif_ccb_mempool_free(g_mempool[ESIF_MEMPOOL_TYPE_PM], lp_ptr);
	esif_ccb_read_unlock(&g_mempool_lock);
}


/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */

/*
** Participant State Management
*/

/* Get Participant State */
enum esif_pm_participant_state esif_lf_pm_lp_get_state(
	const struct esif_lp *lp_ptr
	)
{
	enum esif_pm_participant_state state;

	state = ESIF_PM_PARTICIPANT_STATE_AVAILABLE;

	if (lp_ptr->instance < MAX_PARTICIPANT_ENTRY) {
		esif_ccb_read_lock(&g_pm.lock);
		state = g_pm.pme[lp_ptr->instance].state;
		esif_ccb_read_unlock(&g_pm.lock);
	}
	ESIF_TRACE_DYN_PM("%s: instance %d current state %s(%d)\n",
			  ESIF_FUNC,
			  lp_ptr->instance,
			  esif_pm_participant_state_str(state),
			  state);
	return state;
}


/* Set Participant State */
enum esif_rc esif_lf_pm_lp_set_state(
	struct esif_lp *lp_ptr,
	const enum esif_pm_participant_state state
	)
{
	enum esif_rc rc = ESIF_OK;
	u8 i = 0;

	if (NULL == lp_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	ESIF_TRACE_DYN_PM("%s: instance %d new state %d\n",
			  ESIF_FUNC,
			  lp_ptr->instance,
			  state);

	if (lp_ptr->instance >= MAX_PARTICIPANT_ENTRY) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	esif_ccb_write_lock(&g_pm.lock);
	g_pm.pme[lp_ptr->instance].state = state;
	esif_ccb_write_unlock(&g_pm.lock);

	/* Have DSP Instrument If We Have DSP */
	if (ESIF_PM_PARTICIPANT_STATE_REGISTERED == state) {
		rc = esif_lf_instrument_participant(lp_ptr);
		if (ESIF_OK != rc)
			goto exit;

		/* Have DSP Instrument Domain */
		for (i = 0; i < lp_ptr->domain_count; i++) {
			struct esif_lp_domain *lpd_ptr = &lp_ptr->domains[i];

			/* Assign Hysteresis Here with Best Effort */
			if (lpd_ptr->capabilities &
			    ESIF_CAPABILITY_TEMP_STATUS) {
				struct esif_data esif_void =
				{ESIF_DATA_VOID, NULL, 0, 0};
				struct esif_data esif_hyst = {
					ESIF_DATA_UINT32,
					&lpd_ptr->temp_hysteresis,
					sizeof(u32),
					sizeof(u32)
				};
				struct esif_data esif_tjmax = {
					ESIF_DATA_UINT32,
					&lpd_ptr->temp_tjmax,
					sizeof(u32),
					sizeof(u32)
				};
				struct esif_primitive_tuple tuple_hyst = {
					GET_TEMPERATURE_THRESHOLD_HYSTERESIS,
					lpd_ptr->id,
					255
				};
				struct esif_primitive_tuple tuple_tjmax = {
					GET_PROC_TJMAX,
					lpd_ptr->id,
					255
				};

				esif_execute_primitive(lpd_ptr->lp_ptr,
						       &tuple_hyst,
						       &esif_void,
						       &esif_hyst,
						       NULL);


				/* Get Unit Data Needed By Power Conversion */
				esif_execute_primitive(lpd_ptr->lp_ptr,
						       &tuple_tjmax,
						       &esif_void,
						       &esif_tjmax,
						       NULL);
				ESIF_TRACE_DYN_PM(
					"%s: dmn %s hyst %d tjmax %d\n",
					ESIF_FUNC,
					lpd_ptr->name_ptr,
					lpd_ptr->temp_hysteresis,
					lpd_ptr->temp_tjmax);

				lpd_ptr->temp_notify_sent = ESIF_FALSE;
				lpd_ptr->temp_aux0 = ESIF_DOMAIN_TEMP_INVALID;
				lpd_ptr->temp_aux1 = ESIF_DOMAIN_TEMP_INVALID;


/*
* TODO: LIFU we need to grab and intitialize these here
* lpd_ptr->temp_tjmax = GET_PROC_TJ_MAX
*
* Dont use DSP constant tc1 for this any longer use this value it will change
* from 100 to 90 to 105 depending on SKU get it from the hardware here.
*
* We need to get it as soon as possible there is a silicon bug that
* causes this value to be reported as 0x40 hex after a connected standy
* cycle for now we need to work around this by reading it early and hanging
* onto the value.
*
* Once you have completed this we can remove TJMAX from the DSP.  We will leave
* the two TC values for future use, but the first will no longer be TJMAX.
*/
			}

			esif_lf_instrument_capability(lpd_ptr);

			if (lpd_ptr->capabilities &
			    ESIF_CAPABILITY_POWER_STATUS) {
				struct esif_data esif_void   =
				{ESIF_DATA_VOID, NULL, 0, 0};
				struct esif_data esif_energy = {
					ESIF_DATA_UINT32,
					&lpd_ptr->unit_energy,
					sizeof(u32),
					sizeof(u32)
				};
				struct esif_data esif_power = {
					ESIF_DATA_UINT32,
					&lpd_ptr->unit_power,
					sizeof(u32),
					sizeof(u32)
				};
				struct esif_data esif_time = {
					ESIF_DATA_UINT32,
					&lpd_ptr->unit_time,
					sizeof(u32),        sizeof(u32)
				};
				struct esif_primitive_tuple tuple_energy = {
					GET_RAPL_ENERGY_UNIT, lpd_ptr->id, 255
				};
				struct esif_primitive_tuple tuple_power = {
					GET_RAPL_POWER_UNIT, lpd_ptr->id, 255
				};
				struct esif_primitive_tuple tuple_time = {
					GET_RAPL_TIME_UNIT, lpd_ptr->id, 255
				};

				/* Get Unit Data Needed By Power Conversion */
				esif_execute_primitive(lpd_ptr->lp_ptr,
						       &tuple_energy,
						       &esif_void,
						       &esif_energy,
						       NULL);
				esif_execute_primitive(lpd_ptr->lp_ptr,
						       &tuple_power,
						       &esif_void,
						       &esif_power,
						       NULL);
				esif_execute_primitive(lpd_ptr->lp_ptr,
						       &tuple_time,
						       &esif_void,
						       &esif_time,
						       NULL);
				ESIF_TRACE_DYN_PM(
					"%s: dmn %s id %d energy %d power %d "
					"time %d\n",
					ESIF_FUNC,
					lpd_ptr->name_ptr,
					lpd_ptr->id,
					lpd_ptr->unit_energy,
					lpd_ptr->unit_power,
					lpd_ptr->unit_time);

				lpd_ptr->power_aux0 = 0;
				lpd_ptr->power_aux1 = 10000000;	/* Something
								 *other than
								 *zero */

			/*
			** TODO: LIFU We need to grab and initialize these here
			** lpd_ptr->unit_power = GET_RAPL_POWER_UNIT
			** lpd_ptr->unit_energy = GET_RAPL_ENERGY_UNIT
			** lpd_ptr->unit_time = GET_RAPL_TIME_UNIT
			*/
				lpd_ptr->poll_mask |= ESIF_POLL_POWER;
				esif_poll_start(lpd_ptr);
			}
		}
	} else if (ESIF_PM_PARTICIPANT_STATE_NEEDDSP == state) {
		/*
		 * Uninstrument domains that currently have a DSP
		 */
		for (i = 0; i < lp_ptr->domain_count; i++) {
			struct esif_lp_domain *lpd_ptr = &lp_ptr->domains[i];
			if (lpd_ptr->capabilities &
			    ESIF_CAPABILITY_POWER_STATUS) {
				lpd_ptr->poll_mask |= ESIF_POLL_POWER;
				esif_poll_stop(lpd_ptr);
			}
			esif_lf_uninstrument_capability(lpd_ptr);
		}
		esif_lf_uninstrument_participant(lp_ptr);
	}
exit:
	return rc;
}


/*
** Participant Lookups
*/

/* Get Lower Participant INTERFACE By Instance ID */
struct esif_participant_iface *esif_lf_pm_pi_get_by_instance_id(
	const u8 instance)
{
	struct esif_participant_iface *pi_ptr = NULL;
	ESIF_TRACE_DYN_PM("%s: instance %d\n", ESIF_FUNC, instance);

	if (instance >= MAX_PARTICIPANT_ENTRY)
		goto exit;

	esif_ccb_read_lock(&g_pm.lock);
	if (g_pm.pme[instance].state > ESIF_PM_PARTICIPANT_STATE_AVAILABLE)
		pi_ptr = g_pm.pme[instance].pi_ptr;

	esif_ccb_read_unlock(&g_pm.lock);
exit:
	if (NULL == pi_ptr)
		ESIF_TRACE_DYN_PM(
			"%s: instance %d NOT found or OUT OF BOUNDS\n",
			ESIF_FUNC,
			instance);
	return pi_ptr;
}


/* Get Lower Participant By Instance ID */
struct esif_lp *esif_lf_pm_lp_get_by_instance_id(const u8 instance)
{
	struct esif_lp *lp_ptr = NULL;
	ESIF_TRACE_DYN_PM("%s: instance %d\n", ESIF_FUNC, instance);

	if (instance >= MAX_PARTICIPANT_ENTRY)
		goto exit;

	esif_ccb_read_lock(&g_pm.lock);
	if (g_pm.pme[instance].state > ESIF_PM_PARTICIPANT_STATE_AVAILABLE)
		lp_ptr = g_pm.pme[instance].lp_ptr;

	esif_ccb_read_unlock(&g_pm.lock);
exit:
	if (NULL == lp_ptr)
		ESIF_TRACE_DYN_PM(
			"%s: instance %d NOT found or OUT OF BOUNDS\n",
			ESIF_FUNC,
			instance);
	return lp_ptr;
}


/* Get Lower Participant By Particiapnt Interface */
struct esif_lp *esif_lf_pm_lp_get_by_pi(
	const struct esif_participant_iface *pi_ptr)
{
	u8 i = 0;
	struct esif_lp *lp_ptr = NULL;

	if (NULL == pi_ptr)
		goto exit;

	esif_ccb_read_lock(&g_pm.lock);
	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (g_pm.pme[i].pi_ptr == pi_ptr) {
			/* Found */
			lp_ptr = g_pm.pme[i].lp_ptr;
			break;
		}
	}

	esif_ccb_read_unlock(&g_pm.lock);
exit:
	return lp_ptr;	/* Not Found Will Be NULL */
}


/* Get Particiapnt Interface By Lower Participant */
struct esif_participant_iface *esif_pm_get_pi_by_lp(struct esif_lp *lp_ptr)
{
	u8 i = 0;
	struct esif_participant_iface *pi_ptr = NULL;

	if (NULL == lp_ptr)
		goto exit;

	esif_ccb_read_lock(&g_pm.lock);
	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (g_pm.pme[i].lp_ptr == lp_ptr) {
			/* Found */
			pi_ptr = g_pm.pme[i].pi_ptr;
			break;
		}
	}

	esif_ccb_read_unlock(&g_pm.lock);
exit:
	return pi_ptr;	/* Not Found Will Be NULL */
}


/* Create Participant Instance */
struct esif_lp *esif_lf_pm_lp_create(struct esif_participant_iface *pi_ptr)
{
	u8 i = 0;
	struct esif_lp *lp_ptr = NULL;

	if (NULL == pi_ptr)
		goto exit;

	lp_ptr = esif_pm_lp_alloc();
	if (NULL == lp_ptr)
		goto exit;

	ESIF_TRACE_DYN_PM("%s: pi %p, lp %p\n", ESIF_FUNC, pi_ptr, lp_ptr);

	/* Lock Package Manager */
	esif_ccb_write_lock(&g_pm.lock);

	/* Find Available Slot In Package Manager Table */
	if (pi_ptr->flags & ESIF_FLAG_DPTFZ) {
		i = 0;	/* Treat DPTFZ Special As ID 0 */
	} else {
		/*
		 * Simple Table Lookup For Now. Scan Table And Find First Empty
		 * Slot indicated by AVAILABLE state
		 */
		for (i = ESIF_INSTANCE_FIRST; i < MAX_PARTICIPANT_ENTRY; i++) {
			if (ESIF_PM_PARTICIPANT_STATE_AVAILABLE ==
			    g_pm.pme[i].state) {
				break;
			}
		}
	}

	/* If No Available Slots Return */
	if (i >= MAX_PARTICIPANT_ENTRY) {
		esif_ccb_free(lp_ptr);
		lp_ptr = NULL;
		esif_ccb_write_unlock(&g_pm.lock);
		goto exit;
	}

	/*
	 * Take Slot
	 */
	g_pm.pme[i].state = ESIF_PM_PARTICIPANT_STATE_CREATED;
	g_pm.pme_count++;
	esif_ccb_write_unlock(&g_pm.lock);

	/*
	 * Initialize Our New Lower Participant
	 */
	lp_ptr->instance = i;		/* Our PME Instance    */
	lp_ptr->enable   = ESIF_TRUE;	/* Enable By Default   */
	lp_ptr->pi_ptr   = pi_ptr;	/* Keep The Original Interface Data */

	/* Assume One Domain */
	lp_ptr->domain_count = 1;

	ESIF_TRACE_DYN_PM("%s: Assigned Instance: %d of %d\n",
			  ESIF_FUNC, lp_ptr->instance, MAX_PARTICIPANT_ENTRY);

	esif_ccb_write_lock(&g_pm.lock);
	g_pm.pme[i].pi_ptr = pi_ptr;
	g_pm.pme[i].lp_ptr = lp_ptr;
	esif_ccb_write_unlock(&g_pm.lock);

exit:
	return lp_ptr;
}


/* Destroy Paticipant Instance */
void esif_lf_pm_lp_destroy(struct esif_lp *lp_ptr)
{
	if (NULL == lp_ptr)
		return;

	/* Stop All Polling */
	esif_poll_stop_all(lp_ptr);

	ESIF_TRACE_DYN_PM("%s: lp %p\n", ESIF_FUNC, lp_ptr);

	esif_ccb_write_lock(&g_pm.lock);

	g_pm.pme[lp_ptr->instance].state  = ESIF_PM_PARTICIPANT_STATE_AVAILABLE;
	g_pm.pme[lp_ptr->instance].lp_ptr = NULL;
	g_pm.pme[lp_ptr->instance].pi_ptr = NULL;
	g_pm.pme_count--;
	esif_pm_lp_free(lp_ptr);
	/* Note we don't free PI since that is someone elses */
	esif_ccb_write_unlock(&g_pm.lock);
}


/* Initialize Participant Manager */
enum esif_rc esif_lf_pm_init(void)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_ccb_mempool *mempool_ptr = NULL;

	ESIF_TRACE_DYN_INIT("%s: Initialize Participant Manager\n", ESIF_FUNC);

	esif_ccb_write_lock(&g_mempool_lock);
	mempool_ptr =
		esif_ccb_mempool_create(ESIF_MEMPOOL_FW_PM,
					sizeof(struct esif_lp));
	if (NULL == mempool_ptr) {
		esif_ccb_write_unlock(&g_mempool_lock);
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	g_mempool[ESIF_MEMPOOL_TYPE_PM] = mempool_ptr;
	esif_ccb_write_unlock(&g_mempool_lock);
	esif_ccb_lock_init(&g_pm.lock);
exit:
	return rc;
}


/* Exit Participant Manager */
void esif_lf_pm_exit(void)
{
	esif_ccb_write_lock(&g_mempool_lock);
	esif_ccb_mempool_destroy(g_mempool[ESIF_MEMPOOL_TYPE_PM]);
	g_mempool[ESIF_MEMPOOL_TYPE_PM] = NULL;
	esif_ccb_write_unlock(&g_mempool_lock);

	esif_ccb_lock_uninit(&g_pm_lock);
	ESIF_TRACE_DYN_INIT("%s: Exit Participant Manager\n", ESIF_FUNC);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
