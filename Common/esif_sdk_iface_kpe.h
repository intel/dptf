/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#define ESIF_KPE_METHOD_IDGFX_GET_SOC_DGPU_WEIGHTS			'WDSG' /* GSDW */

/* SET */
#define ESIF_KPE_METHOD_IDGFX_SET_POWER_LIMIT				'0LPS' /* SPL0 */
#define ESIF_KPE_METHOD_IDGFX_SET_PL1_TAU					'UATS' /* STAU */
#define ESIF_KPE_METHOD_IDGFX_SET_ENERGY_THRESHOLD			'0TES' /* SET0 */
#define ESIF_KPE_METHOD_IDGFX_SET_ENERGY_THRESH_INT_DISABLE	'0IED' /* DEI0 */

/* GET/SET */
#define ESIF_KPE_METHOD_IDGFX_BURST_POWER_LIMIT				'0LPB' /* BPL0 */
#define ESIF_KPE_METHOD_IDGFX_PEAK_POWER_LIMIT_AC			'0APP' /* PPA0 */
#define ESIF_KPE_METHOD_IDGFX_PEAK_POWER_LIMIT_BATTERY		'0BPP' /* PPB0 */

/*
* IPU KPE (MIPI Camera KPE Interface)
* Refer to Appendix A.7 of the KPE interface doc
*/
/* GET */
#define ESIF_KPE_METHOD_IPU_GET_PARTICIPANT_CAPABILITIES			'PACP' /* PCAP */
#define ESIF_KPE_METHOD_IPU_GET_PERF_SUPPORT_STATES					'SSPP' /* PPSS */
#define ESIF_KPE_METHOD_IPU_GET_PARTICIPANT_PERF_PRESENT_CAPABILITY	'CPPP' /* PPPC */

/* SET */
#define ESIF_KPE_METHOD_IPU_SET_PERF_PRESENT_CAPABILITY				'CPPS' /* SPPC */
#define ESIF_KPE_METHOD_IPU_SET_START_IP_ALIGNMENT					'APIS' /* SIPA */
#define ESIF_KPE_METHOD_IPU_SET_STOP_IP_ALIGNMENT					'APIE' /* EIPA */
#define ESIF_KPE_METHOD_IPU_SET_IP_ALIGNMENT_SUPPORT_STATE			'SSIS' /* SISS */

#define ESIF_KPE_SISS_IPF_NOT_CAPABLE 0
#define ESIF_KPE_SISS_IPF_CAPABLE 1


/*
* AUDIO KPE (AUDIO KPE Interface)
* Refer to Appendix A.6 of the KPE interface doc
*/
/* GET */
#define ESIF_KPE_METHOD_AUDIO_GET_PARTICIPANT_CAPABILITIES			'PACP' /* PCAP */
#define ESIF_KPE_METHOD_AUDIO_GET_IP_SCHEDULE_ALIGNMENT_SETTING		'SAIC' /* CIAS */

/* SET */
#define ESIF_KPE_METHOD_AUDIO_SET_START_IP_ALIGNMENT				'APIS' /* SIPA */
#define ESIF_KPE_METHOD_AUDIO_SET_STOP_IP_ALIGNMENT					'APIE' /* EIPA */
#define ESIF_KPE_METHOD_AUDIO_SET_IP_ALIGNMENT_SUPPORT_STATE		'SSIS' /* SISS */

#pragma pack(push, 1)

#define IP_ALIGNMENT_SETTINGS_STRUCT_VERSION 	1

/* AUDIO will be responsible for the IP alignment event signaling */
#define IP_ALIGNMENT_EVENT_GENERATED_BY_AUDIO 0
/* IPF will be responsible for the IP alignment event signaling */ 
#define IP_ALIGNMENT_EVENT_GENERATED_BY_IPF 1

/* Supporter not set as trigger source */
#define IP_ALIGNMENT_SETTINGS_SET_TRIGGER_SOURCE_FALSE	0
/* Request supporter act as trigger source */
#define IP_ALIGNMENT_SETTINGS_SET_TRIGGER_SOURCE_TRUE	1

/* For ESIF_KPE_METHOD_IPU_SET_START_IP_ALIGNMENT command */
typedef struct IpAlignmentSettings_s {
	u32 version;			// Set to IP_ALIGNMENT_SETTINGS_STRUCT_VERSION
	u32 eventSource;		// IP_ALIGNMENT_EVENT_GENERATED_BY_XXX
	u32 setTriggerSource;	// 1 - Indicates KPE should accept trigger source role,
							// false 0 - By default
	u32 eventCadence;		// Unit: microseconds, e.g. 10000 for 10ms
	u64 eventLastSyncTime;	// Absolute timestamp in ticks (Based on KeQueryPerformanceCounter)
} IpAlignmentSettings, *IpAlignmentSettingsPtr;


/* Returned from WIFI GWFC Method */
typedef union WifiCapabilities_u {
	struct {
		u32 ThermalMonitoring : 1;		/* Bit 0 */
		u32 PowerControl : 1;			/* Bit 1 */
		u32 CnviSupport : 1;			/* Bit 2 */
		u32 CnviDdrScanProtect : 1;		/* Bit 3 */
		u32 CnviDdrAssocProtect : 1;	/* Bit 4 */
		u32 CnviDdrThruputProtect : 1;	/* Bit 5 */
		u32 UtilMonitoring : 1;			/* Bit 6 */
		u32 ReportsRssi : 1;			/* Bit 7 */
		u32 ReportsChanChange : 1;		/* Bit 8 */
		u32 CnviDlvrRfim : 1;			/* Bit 9 */
		u32 IpaSupported : 1;			/* Bit 10 */
		u32 IpaTriggerSource : 1;		/* Bit 11 */
		u32 rsvd : 20;
	} u;
	u32 asDword;
} WifiCapabilities;

/* Returned from IPU PCAP Method */
typedef union IpuCapabilities_u {
	struct {
		u32 FpsThrottling : 1;			/* Bit 0 */
		u32 IpaSupported : 1;			/* Bit 1 */
		u32 IpaTriggerSource : 1;		/* Bit 2 */
		u32 rsvd : 29;
	} u;
	u32 asDword;
} IpuCapabilities;


/* Returned from Audio PCAP Method */
typedef union AudioCapabilities_u {
	struct {
		u32 IpaSupported : 1;			/* Bit 0 */
		u32 IpaTriggerSource : 1;		/* Bit 1 */
		u32 rsvd : 30;
	} u;
	u32 asDword;
} AudioCapabilities;

/* Definitions associated with ESIF_EVENT_IP_ALIGNMENT_STATE_CHANGED event from KPE */
# define IP_ALIGNMENT_EVENT_STRUCTURE 1

/*
* KPE has stopped the IP alignment event signaling
* When the event generation stops, this is the timestamp when 
* the last event was generated.
*/
#define IP_ALIGNMENT_STOPPED 0
/*
* KPE has started the IP alignment event signaling
* When the event generation starts, this is the timestamp when 
* the first event is generated.
*/
#define IP_ALIGNMENT_STARTED 1

typedef struct IpAlignmentEventArg_s {
	u32 version;					// Must be set to IP_ALIGNMENT_EVENT_STRUCTURE
	u32 ipAlignmentState;			// IP_ALIGNMENT_STOPPED/STARTED
	u32 ipAlignmentCadence;			// Unit: microseconds
	u64 ipAlignmentLastSyncTime;	// Timestamp in ticks (based on KeQueryPerformanceCounter)
} IpAlignmentEventArg;


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
