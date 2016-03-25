/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

/*
 * Data Type Declarations
 */

enum esif_data_type {
	ESIF_DATA_ANGLE = 41,
	ESIF_DATA_AUTO = 36,
	ESIF_DATA_BINARY = 7,
	ESIF_DATA_BIT = 27,
	ESIF_DATA_BLOB = 34,
	ESIF_DATA_DECIBEL = 39,
	ESIF_DATA_DSP = 33,
	ESIF_DATA_ENUM = 19,
	ESIF_DATA_FREQUENCY = 40,
	ESIF_DATA_GUID = 5,
	ESIF_DATA_HANDLE = 20,
	ESIF_DATA_INSTANCE = 30,
	ESIF_DATA_INT16 = 12,
	ESIF_DATA_INT32 = 13,
	ESIF_DATA_INT64 = 14,
	ESIF_DATA_INT8 = 11,
	ESIF_DATA_IPV4 = 16,
	ESIF_DATA_IPV6 = 17,
	ESIF_DATA_PERCENT = 29,
	ESIF_DATA_POINTER = 18,
	ESIF_DATA_POWER = 26,
	ESIF_DATA_QUALIFIER = 28,
	ESIF_DATA_REGISTER = 15,
	ESIF_DATA_STRING = 8,
	ESIF_DATA_STRUCTURE = 32,
	ESIF_DATA_TABLE = 35,
	ESIF_DATA_TEMPERATURE = 6,
	ESIF_DATA_TIME = 31,
	ESIF_DATA_UINT16 = 2,
	ESIF_DATA_UINT32 = 3,
	ESIF_DATA_UINT64 = 4,
	ESIF_DATA_UINT8 = 1,
	ESIF_DATA_UNICODE = 9,
	ESIF_DATA_VOID = 24,
	ESIF_DATA_XML = 38,
};

static ESIF_INLINE esif_string esif_data_type_str(enum esif_data_type type)
{
	switch (type) {
	ESIF_CASE_ENUM(ESIF_DATA_ANGLE);
	ESIF_CASE_ENUM(ESIF_DATA_AUTO);
	ESIF_CASE_ENUM(ESIF_DATA_BINARY);
	ESIF_CASE_ENUM(ESIF_DATA_BIT);
	ESIF_CASE_ENUM(ESIF_DATA_BLOB);
	ESIF_CASE_ENUM(ESIF_DATA_DECIBEL);
	ESIF_CASE_ENUM(ESIF_DATA_DSP);
	ESIF_CASE_ENUM(ESIF_DATA_ENUM);
	ESIF_CASE_ENUM(ESIF_DATA_FREQUENCY);
	ESIF_CASE_ENUM(ESIF_DATA_GUID);
	ESIF_CASE_ENUM(ESIF_DATA_HANDLE);
	ESIF_CASE_ENUM(ESIF_DATA_INSTANCE);
	ESIF_CASE_ENUM(ESIF_DATA_INT16);
	ESIF_CASE_ENUM(ESIF_DATA_INT32);
	ESIF_CASE_ENUM(ESIF_DATA_INT64);
	ESIF_CASE_ENUM(ESIF_DATA_INT8);
	ESIF_CASE_ENUM(ESIF_DATA_IPV4);
	ESIF_CASE_ENUM(ESIF_DATA_IPV6);
	ESIF_CASE_ENUM(ESIF_DATA_PERCENT);
	ESIF_CASE_ENUM(ESIF_DATA_POINTER);
	ESIF_CASE_ENUM(ESIF_DATA_POWER);
	ESIF_CASE_ENUM(ESIF_DATA_QUALIFIER);
	ESIF_CASE_ENUM(ESIF_DATA_REGISTER);
	ESIF_CASE_ENUM(ESIF_DATA_STRING);
	ESIF_CASE_ENUM(ESIF_DATA_STRUCTURE);
	ESIF_CASE_ENUM(ESIF_DATA_TABLE);
	ESIF_CASE_ENUM(ESIF_DATA_TEMPERATURE);
	ESIF_CASE_ENUM(ESIF_DATA_TIME);
	ESIF_CASE_ENUM(ESIF_DATA_UINT16);
	ESIF_CASE_ENUM(ESIF_DATA_UINT32);
	ESIF_CASE_ENUM(ESIF_DATA_UINT64);
	ESIF_CASE_ENUM(ESIF_DATA_UINT8);
	ESIF_CASE_ENUM(ESIF_DATA_UNICODE);
	ESIF_CASE_ENUM(ESIF_DATA_VOID);
	ESIF_CASE_ENUM(ESIF_DATA_XML);
	}
	return ESIF_NOT_AVAILABLE;
}

static ESIF_INLINE size_t esif_data_type_sizeof(enum esif_data_type type)
{
	switch (type) {
	ESIF_CASE_VAL(ESIF_DATA_ANGLE, sizeof(unsigned int));
	ESIF_CASE_VAL(ESIF_DATA_BIT, sizeof(unsigned char));
	ESIF_CASE_VAL(ESIF_DATA_DECIBEL, sizeof(unsigned int));
	ESIF_CASE_VAL(ESIF_DATA_ENUM, sizeof(int));
	ESIF_CASE_VAL(ESIF_DATA_FREQUENCY, sizeof(unsigned long long));
	ESIF_CASE_VAL(ESIF_DATA_GUID, 16);
	ESIF_CASE_VAL(ESIF_DATA_INSTANCE, sizeof(unsigned short));
	ESIF_CASE_VAL(ESIF_DATA_INT16, sizeof(short));
	ESIF_CASE_VAL(ESIF_DATA_INT32, sizeof(int));
	ESIF_CASE_VAL(ESIF_DATA_INT64, sizeof(long long));
	ESIF_CASE_VAL(ESIF_DATA_INT8, sizeof(char));
	ESIF_CASE_VAL(ESIF_DATA_IPV4, 4);
	ESIF_CASE_VAL(ESIF_DATA_IPV6, 16);
	ESIF_CASE_VAL(ESIF_DATA_PERCENT, sizeof(unsigned int));
	ESIF_CASE_VAL(ESIF_DATA_POINTER, sizeof(void *));
	ESIF_CASE_VAL(ESIF_DATA_POWER, sizeof(unsigned int));
	ESIF_CASE_VAL(ESIF_DATA_QUALIFIER, sizeof(unsigned short));
	ESIF_CASE_VAL(ESIF_DATA_REGISTER, sizeof(void *));
	ESIF_CASE_VAL(ESIF_DATA_TEMPERATURE, sizeof(unsigned int));
	ESIF_CASE_VAL(ESIF_DATA_TIME, sizeof(unsigned int));
	ESIF_CASE_VAL(ESIF_DATA_UINT16, sizeof(unsigned short));
	ESIF_CASE_VAL(ESIF_DATA_UINT32, sizeof(unsigned int));
	ESIF_CASE_VAL(ESIF_DATA_UINT64, sizeof(unsigned long long));
	ESIF_CASE_VAL(ESIF_DATA_UINT8, sizeof(unsigned char));
	ESIF_CASE_VAL(ESIF_DATA_VOID, 0);
	default: return 0;
	}
}

#ifdef ESIF_ATTR_USER
#ifdef esif_ccb_stricmp
static ESIF_INLINE enum esif_data_type esif_data_type_str2enum(esif_string name)
{
	int j;
	struct esif_data_type_map_t {
		enum esif_data_type type;
		esif_string name;
	}
	esif_data_type_map[] = {
		ESIF_MAP_ENUM(ESIF_DATA_ANGLE),
		ESIF_MAP_ENUM(ESIF_DATA_AUTO),
		ESIF_MAP_ENUM(ESIF_DATA_BINARY),
		ESIF_MAP_ENUM(ESIF_DATA_BIT),
		ESIF_MAP_ENUM(ESIF_DATA_BLOB),
		ESIF_MAP_ENUM(ESIF_DATA_DECIBEL),
		ESIF_MAP_ENUM(ESIF_DATA_DSP),
		ESIF_MAP_ENUM(ESIF_DATA_ENUM),
		ESIF_MAP_ENUM(ESIF_DATA_FREQUENCY),
		ESIF_MAP_ENUM(ESIF_DATA_GUID),
		ESIF_MAP_ENUM(ESIF_DATA_HANDLE),
		ESIF_MAP_ENUM(ESIF_DATA_INSTANCE),
		ESIF_MAP_ENUM(ESIF_DATA_INT16),
		ESIF_MAP_ENUM(ESIF_DATA_INT32),
		ESIF_MAP_ENUM(ESIF_DATA_INT64),
		ESIF_MAP_ENUM(ESIF_DATA_INT8),
		ESIF_MAP_ENUM(ESIF_DATA_IPV4),
		ESIF_MAP_ENUM(ESIF_DATA_IPV6),
		ESIF_MAP_ENUM(ESIF_DATA_PERCENT),
		ESIF_MAP_ENUM(ESIF_DATA_POINTER),
		ESIF_MAP_ENUM(ESIF_DATA_POWER),
		ESIF_MAP_ENUM(ESIF_DATA_QUALIFIER),
		ESIF_MAP_ENUM(ESIF_DATA_REGISTER),
		ESIF_MAP_ENUM(ESIF_DATA_STRING),
		ESIF_MAP_ENUM(ESIF_DATA_STRUCTURE),
		ESIF_MAP_ENUM(ESIF_DATA_TABLE),
		ESIF_MAP_ENUM(ESIF_DATA_TEMPERATURE),
		ESIF_MAP_ENUM(ESIF_DATA_TIME),
		ESIF_MAP_ENUM(ESIF_DATA_UINT16),
		ESIF_MAP_ENUM(ESIF_DATA_UINT32),
		ESIF_MAP_ENUM(ESIF_DATA_UINT64),
		ESIF_MAP_ENUM(ESIF_DATA_UINT8),
		ESIF_MAP_ENUM(ESIF_DATA_UNICODE),
		ESIF_MAP_ENUM(ESIF_DATA_VOID),
		ESIF_MAP_ENUM(ESIF_DATA_XML),
	};

	/* Match ESIF_DATA_TYPENAME or TYPENAME */
	for (j = 0; j < ESIF_ARRAY_LEN(esif_data_type_map); j++) {
		if (esif_ccb_stricmp(esif_data_type_map[j].name, name) == 0)
			return esif_data_type_map[j].type;
		if (esif_ccb_stricmp(esif_data_type_map[j].name+10, name) == 0)
			return esif_data_type_map[j].type;
	}
	return ESIF_DATA_VOID;
}
#endif
#endif

