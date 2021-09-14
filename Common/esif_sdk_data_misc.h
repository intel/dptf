/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#pragma pack(pop)

#define OFFSET_OF(type, member) ((u32)(size_t)&(((type*)0)->member))

#define SIZE_OF(type, member) (sizeof(((type*)0)->member))

#define OFFSET_PTR(ptr, offset, type) ((type)(((char *)(ptr)) + (offset)))

#ifdef ESIF_ATTR_USER

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

#endif /* ESIF_ATTR_USER */
