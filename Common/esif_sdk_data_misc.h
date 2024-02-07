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

#include "esif_sdk_data.h"

/*
* Header structures for PPM parameter and the PPM package that holds all
* the PPM parameters associated with it.
*/
#define DEFAULT_PARAM_COUNT 1
#define PPM_HEADER_REVISION 1

typedef struct EsifPpmParamValues_s
{
	u32 powerSource; //AC: 0, DC : 1
	u32 paramValue; // 32bits
	esif_guid_t powerSchemeGuid;
	esif_guid_t subgroupGuid;
	esif_guid_t paramGuid;
} EsifPpmParamValues;

typedef struct EsifPpmParamValuesHeader_s
{
	u32 revision; // 1,2, ...
	u32 numberElement;
	EsifPpmParamValues ppmParamValuesArray[DEFAULT_PARAM_COUNT];	// Additional items may be present following this
} EsifPpmParamValuesHeader;

/* Have ESIF Allocate Buffer */
#define ESIF_DATA_ALLOCATE 0xFFFFFFFF

#define ACPI_OSC_ARG_COUNT 4

#define LOADABLE_ACTION_DEVICE_STRING_FIELD_LEN 255
#define LOADABLE_ACTION_DEVICE_DATA_COUNT 10

/*
 * OS and Kernel/User Agnostic
 */

#pragma pack(push, 1)

/* Thermal Event */
struct esif_data_complex_thermal_event {
	u32 temperature;
	u32 tripPointTemperature;
	char participantName[ESIF_NAME_LEN];
};

/* Operating System Capabilities */
struct esif_data_complex_osc {
	esif_guid_t  guid;
	u32          revision;
	u32          count;
	u32          status;		/* Required */
	u32          capabilities;	/* Always Used By DPTF */
};

/* GUID Pair */
struct esif_data_complex_guid_pair {
	esif_guid_t  guid1; /* Example SUB_GROUP */
	esif_guid_t  guid2; /* Example Power Setting */
};

#define ESIF_TABLE_NO_REVISION  0xffff
struct esif_table_hdr {
	u8   revision;
	u16  rows;
	u16  cols;
};


struct loadable_action_device_string_data {
	char stringData[LOADABLE_ACTION_DEVICE_STRING_FIELD_LEN];
};
struct loadable_action_device {
	char deviceName[LOADABLE_ACTION_DEVICE_STRING_FIELD_LEN];
	char deviceDescription[LOADABLE_ACTION_DEVICE_STRING_FIELD_LEN];
	struct loadable_action_device_string_data deviceStringData[LOADABLE_ACTION_DEVICE_DATA_COUNT];
	u32 deviceNumericData[LOADABLE_ACTION_DEVICE_DATA_COUNT];
};
struct loadable_action_devices {
	int numDevicesLoaded;
	struct loadable_action_device deviceData[1]; //n number of loadable_action_device objects
};

/*
* Request structure for Overclocking Mailbox primitives
*/
#define OCMB_REQUEST_VERSION_1 1 /* Start at 1, so cleared structure isn't valid */
#define OCMB_REQUEST_VERSION OCMB_REQUEST_VERSION_1

struct ocmb_affinity_data {
	u16 group;
	u64 affinity_mask; /* 0 indicates no affinity required */
	u32 data; /* Only used for SETs.  GET data returned in the response buffer (See ocmb_get_response_data)*/
	/*
	* Notes:
	* 1) If the affinity_mask is 0, only a single data element is accessed on
	* current core without affinity change.
	* 2) For GET's, if the affinity_mask is not all F's, only the data elements
	* corresponding to the bitmask position are valid upon return.
	* 3) For GET's, a response buffer large enough to hold 64 * sizeof(ocmb_get_response_data) is
	* still required, but only the first item is valid.
	*/
};

struct ocmb_request {
	u32 version;
	struct {
		u32 cmd_rsp : 8;
		u32 param1 : 8;
		u32 param2 : 8;
		u32 rsvd : 8;
	} cmd;
	u32 count; /* Number of ocmb_affinity_data structures; must be non-0 */
	/* Array of ocmb_affinity_data structure follow here. See ocmb_affinity_data */
};

/* Used for OCMB GETs which return an array of the defined structures.*/
struct ocmb_get_response_data {
	u32 data; 
	u32 status; /* OCMB command completion code (0 - success), not IPF status */
};


/*
* Request structure for Affinitized MSR primitives
*/
#define AMSR_REQUEST_VERSION_1 1 /* Start at 1, so cleared structure isn't valid */
#define AMSR_REQUEST_VERSION AMSR_REQUEST_VERSION_1

struct amsr_affinity_data {
	u16 group;
	u64 affinity_mask; /* 0 indicates no affinity required */
	u64 data; /* Only used for SETs.  GET data returned in the response buffer as a u64 array */
	/*
	* Notes:
	* 1) If the affinity_mask is 0, only a single data element is accessed on
	* current core without affinity change.
	* 2) For GET's, if the affinity_mask is not all F's, only the data elements
	* corresponding to the bitmask position are valid upon return.
	* 3) For GET's, a response buffer large enough to hold 64 * sizeof(u64) is
	* still required, but only the first item is valid.
	*/
};

struct amsr_request {
	u32 version;
	u32 count; /* Number of amsr_affinity_data structures; must be non-0 */
	/* Array of amsr_affinity_data structure follow here. See amsr_affinity_data */
};


/* Process Notification Event Data */
#define PROCESS_NOTIFICATION_REVISION 1
struct esif_data_process_notification
{
	u32 revision;
	esif_handle_t process_id;
	esif_handle_t parent_process_id;
	char image_path[ESIF_MAX_PATH];
};

#pragma pack(pop)

#define OFFSET_OF(type, member) ((u32)(size_t)&(((type*)0)->member))

#define SIZE_OF(type, member) (sizeof(((type*)0)->member))

#define OFFSET_PTR(ptr, offset, type) ((type)(((char *)(ptr)) + (offset)))


/*
 *  ESIF Data is used to marshall most data between ESIF and the
 *  application it contains.  ESIF data uses the populare TLV
 *  format for describing the data.  It can be used to request
 *  specific data from an object or to identify the data type that
 *  is returned.  While a simple structure it is very helpful.  Here
 *  are a set of convinience initializers to make working with this
 *  structure easier.
 */
#define ESIF_DATA(var, typ, ptr, len) \
	struct esif_data var = { \
		ESIF_ELEMENT(.type) typ, \
		ESIF_ELEMENT(.buf_ptr) ptr, \
		ESIF_ELEMENT(.buf_len) len \
	}


#define ESIF_DATA_VOID(var) \
	struct esif_data var = { \
		ESIF_ELEMENT(.type)    ESIF_DATA_VOID, \
		ESIF_ELEMENT(.buf_ptr) NULL, \
		ESIF_ELEMENT(.buf_len) 0 \
	}


#define ESIF_DATA_AUTO(var) \
	struct esif_data var = { \
		ESIF_ELEMENT(.type)    ESIF_DATA_AUTO, \
		ESIF_ELEMENT(.buf_ptr) ESIF_DATA_ALLOCATE, \
		ESIF_ELEMENT(.buf_len) 0 \
	}

#define ESIF_DATA_STRING_ASSIGN(obj, buf, buf_size)             \
	obj.type     = ESIF_DATA_STRING;                                \
	obj.buf_ptr  = buf;                                          \
	obj.buf_len  = buf_size;                                     \
	obj.data_len = (u32)(esif_ccb_strlen(buf, buf_size) + 1);

#define ESIF_DATA_UINT32_ASSIGN(obj, buf, buf_size)             \
	obj.type     = ESIF_DATA_UINT32;                                \
	obj.buf_ptr  = buf;                                          \
	obj.buf_len  = buf_size;                                     \
	obj.data_len = sizeof(UInt32);

#pragma pack(push, 1)
/* BINARY Primitive Tuple Parameter for passing from DPTF to a DSP Action */
typedef struct esif_primitive_tuple_parameter {
	union esif_data_variant  id;
	union esif_data_variant  domain;
	union esif_data_variant  instance;
} EsifPrimitiveTupleParameter;

typedef struct dim_2d_s {
	UInt32 width;
	UInt32 height;
} Dim2D;

/* Note:  Only applicable to physical monitors anddoesn't include virtual monitors */
typedef struct aggregate_monitor_data_s {
	UInt32 numberOfMonitors;
	UInt32 numberOfExternalMonitors;
	UInt32 numberOfExternalWiredMontiors;
	UInt32 numberOfExternalWirelessMonitors;

	UInt64 highestDisplayResolution; /* Pixel count */
	UInt64 highestExternalDisplayResolution;
	UInt64 highestWiredDisplayResolution;
	UInt64 highestWirelessDisplayResolution;
	UInt64 totalExernalDisplayResolution; /* Sum of widths only */
} AggregateMonitorData;

#pragma pack(pop)

struct esif_data_rfprofile
{
	union esif_data_variant is5G; // Bool - is5G
	union esif_data_variant servingCellInfo; // UInt32 - servingCellInfo
	union esif_data_variant centerFrequency; // Frequency - centerFrequency
	union esif_data_variant frequencySpread; // Frequency - frequencySpread
	union esif_data_variant connectStatus; // ULONG - Connection Status
	union esif_data_variant channelNumber; // UInt32 - channelNumber
	union esif_data_variant band; // UInt32 - band
};

#define SOCWC_NUMBER_WORKLOAD_CLASSIFICATIONS	5
#define RF_PROFILE_STRUCT_VERSION 1

#pragma pack(push, 1)
typedef struct RfProfile_s
{
	UInt32 version;
	UInt32 is5G;  // Boolean - is5G, modem only   
	UInt32 channelPriority;   // UInt32 - 0: primary, 1: secondary, ...   
	UInt64 centerFrequency;  // UInt64 - centerFrequency in Hz   
	UInt32 frequencySpread; // UInt32 - frequencySpread in Hz   
	UInt32 connectStatus;  // UInt32 - 0: disconnected, 1: connected   
	UInt32 channelNumber;  // UInt32 - channelNumber   
	UInt32 band; // UInt32 - band  
	UInt32 rssi; // UInt32 - rssi 
}RfProfile, * RfProfilePtr;
#pragma pack(pop)

#define MAX_ACTIVE_RF_CHANNELS 5

#pragma pack(push, 1)
typedef struct ActiveRfChannels_s
{
	UInt8  numberOfChannels;
	RfProfile rfChannels[MAX_ACTIVE_RF_CHANNELS];
} ActiveRfChannels, * ActiveRfChannelsPtr;
#pragma pack(pop)

#define RAPL_ENERGY_INFO_REVISION 1

#pragma pack(push,1)
typedef struct RaplEnergyInfo_s
{
	UInt32 revision;
	UInt64 timestamp;
	UInt32 energyCounter;
} RaplEnergyInfo;
#pragma pack(pop)

#define AFFINITY_CMD_VERSION 1

typedef struct AffinityCommand_s {
	UInt32 version; // must be AFFINITY_CMD_VERSION
	UInt32 affinity_mask;
	char process_name[MAX_PATH];
} AffinityCommand;

#define APPLICATION_OPTIMIZATION_APPCOMPAT_GUID {0x99, 0x5c, 0xc3, 0x75, 0xc3, 0x09, 0x4b, 0xaa, 0xa9, 0x9d, 0xe9, 0x70, 0xd0, 0x89, 0x3a, 0x9c}
#define APP_COMPAT_CREATE_CMD_VERSION 1

typedef struct AppCompatEntry_s
{
	UInt32 processor_count;
	char process_name[MAX_PATH];
} AppCompatEntry;

typedef struct AppCompatCreateCommand_s {
	UInt32 version; // must be APP_COMPAT_CREATE_CMD_VERSION
	esif_guid_t app_compat_guid;
	UInt32 num_process;
	// num_process * AppCompatEntry structures follow here. 
} AppCompatCreateCommand;

#define APP_COMPAT_DELETE_CMD_VERSION 1

typedef struct AppCompatDeleteCommand_s {
	UInt32 version; // must be APP_COMPAT_DELETE_CMD_VERSION
	esif_guid_t app_compat_guid;
} AppCompatDeleteCommand;


// Event data for ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED
enum sensor_user_presence_state_e {
	SENSOR_USER_PRESENCE_STATE_NOT_PRESENT = 0,
	SENSOR_USER_PRESENCE_STATE_DISENGAGED = 1,
	SENSOR_USER_PRESENCE_STATE_ENGAGED = 2,
	SENSOR_USER_PRESENCE_STATE_INVALID = 99
};

// Event data for ESIF_EVENT_SENSOR_USER_PRESENCE_WITH_ENROLLMENT_CHANGED
enum sensor_user_presence_with_enrollment_state_e {
	SENSOR_USER_PRESENCE_WITH_ENROLLMENT_STATE_NOT_PRESENT = 0,
	SENSOR_USER_PRESENCE_WITH_ENROLLMENT_STATE_DISENGAGED = 1,
	SENSOR_USER_PRESENCE_WITH_ENROLLMENT_STATE_ENGAGED = 2,
	SENSOR_USER_PRESENCE_WITH_ENROLLMENT_STATE_DISENGAGED_WITH_FACEID = 3,
	SENSOR_USER_PRESENCE_WITH_ENROLLMENT_STATE_ENGAGED_WITH_FACEID = 4,
	SENSOR_USER_PRESENCE_WITH_ENROLLMENT_STATE_INVALID = 99
};

// Event data for ESIF_EVENT_PLATFORM_USER_PRESENCE_CHANGED
enum platform_user_presence_state_e {
	PLATFORM_USER_PRESENCE_STATE_NOT_PRESENT = 0,
	PLATFORM_USER_PRESENCE_STATE_DISENGAGED = 1,
	PLATFORM_USER_PRESENCE_STATE_ENGAGED = 2,
	PLATFORM_USER_PRESENCE_STATE_FACE_ENGAGED = 3,
	PLATFORM_USER_PRESENCE_STATE_INVALID = 99
};

// Event data for ESIF_EVENT_PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_CHANGED
enum platform_user_presence_with_enrollment_state_e {
	PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_NOT_PRESENT = 0,
	PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_DISENGAGED = 1,
	PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_ENGAGED = 2,
	PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_FACE_ENGAGED = 3,
	PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_DISENGAGED_WITH_FACEID = 4,
	PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_ENGAGED_WITH_FACEID = 5,
	PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_INVALID = 99
};


// For use with ESIF_EVENT_FACE_ID_CAPABILITY_CHANGED
enum face_id_capability_state_e {
	FACE_ID_CAPABILITY_STATE_NOT_SUPPORTED = 0,
	FACE_ID_CAPABILITY_STATE_NOT_ENROLLED = 1,
	FACE_ID_CAPABILITY_STATE_ENROLLED = 2
};

// For use with ESIF_EVENT_IP_ALIGNMENT_STATUS
enum ip_alignment_state_e {
	IP_ALIGNMENT_STATUS_INVALID = -1,
	IP_ALIGNMENT_STATUS_INACTIVE = 0,
	IP_ALIGNMENT_STATUS_ACTIVE = 1
};


typedef enum sensor_user_presence_state_e SensorUserPresenceState;
typedef enum sensor_user_presence_with_enrollment_state_e SensorUserPresenceWithEnrollmentState;
typedef enum platform_user_presence_state_e PlatformUserPresenceState;
typedef enum platform_user_presence_with_enrollment_state_e PlatformUserPresenceWithEnrollmentState;
typedef enum face_id_capability_state_e FaceIdCapabilityState;
typedef enum ip_alignment_state_e IpAlignmentState;

