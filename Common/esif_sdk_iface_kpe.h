/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#pragma once

#include "esif_sdk.h"
#include "esif_sdk_data.h"
#include "esif_sdk_action_type.h"
#include "esif_sdk_class_guid.h"
#include "esif_sdk_event_type.h"
#include "esif_ccb_rc.h"

#define ESIF_DRIVER_VERSION 2 /* KPE interface version */

/*
* The below section defines the method opcodes for each of the KPE's that
* are supported. Further details can be found in the KPE interface document
*/

/*
* PSM KPE (Power Sharing Manager KPE for wireless)
* Refer to Appendix A.1 of the KPE interface doc
*/
/* GET */
#define ESIF_KPE_METHOD_PSM_GET_TEMP						'PMT_' /* _TMP */
#define ESIF_KPE_METHOD_PSM_GET_TEMP_HYST					'HSTG' /* GTSH */
#define ESIF_KPE_METHOD_PSM_GET_PART_POWER_CONTROL_CAPS		'CCPP' /* PPCC */
#define ESIF_KPE_METHOD_PSM_GET_CURRENT_POWER				'0PRG' /* GRP0 */
#define ESIF_KPE_METHOD_PSM_GET_POWER_LIMIT					'0LPG' /* GPL0 */

/* SET */
#define ESIF_KPE_METHOD_PSM_SET_LOWER_TEMP_THRESHOLD		'0TAP' /* PAT0 */
#define ESIF_KPE_METHOD_PSM_SET_UPPER_TEMP_THRESHOLD		'1TAP' /* PAT1 */
#define ESIF_KPE_METHOD_PSM_SET_POWER_LIMIT					'0LPS' /* SPL0 */

/*
* DGFX KPE (Discrete GFX KPE)
* Refer to Appendix A.2 of the KPE interface doc
*/
/* GET */
#define ESIF_KPE_METHOD_DGFX_GET_TEMP						'PMT_' /* _TMP */
#define ESIF_KPE_METHOD_DGFX_GET_POWER_LIMIT				'0LPG' /* GPL0 */
#define ESIF_KPE_METHOD_DGFX_GET_ENERGY						'0CEG' /* GEC0 */
#define ESIF_KPE_METHOD_DGFX_GET_ENERGY_UNIT				'0UEG' /* GEU0 */
#define ESIF_KPE_METHOD_DGFX_GET_ACTIVITY_COUNTER			'CAGG' /* GGAC */
#define ESIF_KPE_METHOD_DGFX_GET_TIMESTAMP_COUNTER			'CSTG' /* GTSC */
#define ESIF_KPE_METHOD_DGFX_GET_INSTANTANEOUS_POWER		'0PIG' /* GIP0 */

/* SET */
#define ESIF_KPE_METHOD_DGFX_SET_POWER_LIMIT				'0LPS' /* SPL0 */
#define ESIF_KPE_METHOD_DGFX_SET_ENERGY_THRESHOLD			'0TES' /* SET0 */
#define ESIF_KPE_METHOD_DGFX_SET_ENERGY_THRESH_INT_DISABLE	'0IED' /* DEI0 */

/* GET/SET */
#define ESIF_KPE_METHOD_DGFX_PEAK_POWER_AC					'0APP' /* PPA0 */
#define ESIF_KPE_METHOD_DGFX_PEAK_POWER_LIMIT_AC_SMALL		'1APP' /* PPA1 */
#define ESIF_KPE_METHOD_DGFX_PEAK_POWER_LIMIT_BATTERY		'0BPP' /* PPB0 */
#define ESIF_KPE_METHOD_DGFX_MAX_PERF_STATE_AC				'0APD' /* DPA0 */
#define ESIF_KPE_METHOD_DGFX_MAX_PERF_STATE_AC_SMALL		'1APD' /* DPA1 */
#define ESIF_KPE_METHOD_DGFX_MAX_PERF_STATE_BATTERY			'0BPD' /* DPB0 */

/*
* IDGFX KPE (IA Discrete GFX KPE)
* Refer to Appendix A.3 of the KPE interface doc
*/
/* GET */
#define ESIF_KPE_METHOD_IDGFX_GET_TEMP						'PMT_' /* _TMP */
#define ESIF_KPE_METHOD_IDGFX_GET_POWER_LIMIT				'0LPG' /* GPL0 */
#define ESIF_KPE_METHOD_IDGFX_GET_INSTANTANEOUS_POWER		'0PIG' /* GIP0 */
#define ESIF_KPE_METHOD_IDGFX_GET_PL1_TAU					'UATG' /* GTAU */
#define ESIF_KPE_METHOD_IDGFX_GET_ENERGY					'0CEG' /* GEC0 */
#define ESIF_KPE_METHOD_IDGFX_GET_ENERGY_UNIT				'0UEG' /* GEU0 */
#define ESIF_KPE_METHOD_IDGFX_GET_ACTIVITY_COUNTER_WIDTH	'WCAG' /* GACW */
#define ESIF_KPE_METHOD_IDGFX_GET_ACTIVITY_COUNTER			'CAGG' /* GGAC */
#define ESIF_KPE_METHOD_IDGFX_GET_TIMESTAMP_COUNTER_WIDTH	'WCTG' /* GTCW */
#define ESIF_KPE_METHOD_IDGFX_GET_TIMESTAMP_COUNTER			'CSTG' /* GTSC */

/* SET */
#define ESIF_KPE_METHOD_IDGFX_SET_POWER_LIMIT				'0LPS' /* SPL0 */
#define ESIF_KPE_METHOD_IDGFX_SET_PL1_TAU					'UATS' /* STAU */
#define ESIF_KPE_METHOD_IDGFX_SET_ENERGY_THRESHOLD			'0TES' /* SET0 */
#define ESIF_KPE_METHOD_IDGFX_SET_ENERGY_THRESH_INT_DISABLE	'0IED' /* DEI0 */

/* GET/SET */
#define ESIF_KPE_METHOD_IDGFX_BURST_POWER_LIMIT				'0LPB' /* BPL0 */
#define ESIF_KPE_METHOD_IDGFX_PEAK_POWER_LIMIT_AC			'0APP' /* PPA0 */
#define ESIF_KPE_METHOD_IDGFX_PEAK_POWER_LIMIT_BATTERY		'0BPP' /* PPB0 */


#pragma pack(push, 1)

struct esif_driver_iface {
	esif_ver_t version;    /* Interface version - ESIF_DRIVER_VERSION */
	esif_guid_t class_guid;/* KPE class GUID - ESIF_LF_PE_CLASS_GUID */
	esif_flags_t flags;    /* Dependent on KPE type */

	char name[ESIF_NAME_LEN]; /* KPE name */
	char desc[ESIF_DESC_LEN]; /* KPE description */

	char driver_name[ESIF_NAME_LEN]; /* Driver name */
	char device_name[ESIF_NAME_LEN]; /* Driver device description */

	void *context_ptr; /* Context passed back to the KPE when called */

	enum esif_action_type action_type; /* Action exported by the KPE */

	enum esif_rc (*get_handler)( /* Action GET handler */
		const void *context_ptr,
		const u32 p1,
		const u32 p2,
		const u32 p3,
		const u32 p4,
		const u32 p5,
		const struct esif_data *request_ptr,
		struct esif_data *response_ptr
		);

	enum esif_rc (*set_handler)( /* Action SET handler */
		const void *context_ptr,
		const u32 domain_handle,
		const u32 p1,
		const u32 p2,
		const u32 p3,
		const u32 p4,
		const u32 p5,
		const struct esif_data *request_ptr
		);

	enum esif_rc (*send_event)( /* Used to send events to ESIF */
		const struct esif_driver_iface *di_ptr,
		const enum esif_event_type type,
		const u32 domain_handle,
		const struct esif_data *data_ptr
		);

	enum esif_rc (*recv_event)( /* Used to send events to the KPE */
		const void *context_ptr,
		const enum esif_event_type type,
		const u32 domain_handle, /* Reserved */
		const struct esif_data *data_ptr
		);
};


#pragma pack(pop)
