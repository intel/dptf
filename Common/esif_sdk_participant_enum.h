/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
 * Participant Enumeration Types
 */

typedef enum esif_participant_enum {
	ESIF_PARTICIPANT_ENUM_INVALID = -1,
	ESIF_PARTICIPANT_ENUM_ACPI = 0,
	ESIF_PARTICIPANT_ENUM_PCI = 1,
	ESIF_PARTICIPANT_ENUM_PLAT = 2,
	ESIF_PARTICIPANT_ENUM_CONJURE = 3,
	ESIF_PARTICIPANT_ENUM_USB = 4,
	ESIF_PARTICIPANT_ENUM_SYSFS = 5,
} esif_participant_enum_t;

/* Max Enum Value for Iteration purposes */
#define MAX_ESIF_PARTICIPANT_ENUM_VALUE  ESIF_PARTICIPANT_ENUM_SYSFS

/* Enumeration String */
static ESIF_INLINE esif_string esif_participant_enum_str(
	esif_participant_enum_t index)
{
	switch (index) {
	ESIF_CASE_ENUM(ESIF_PARTICIPANT_ENUM_INVALID);
	ESIF_CASE_ENUM(ESIF_PARTICIPANT_ENUM_ACPI);
	ESIF_CASE_ENUM(ESIF_PARTICIPANT_ENUM_PCI);
	ESIF_CASE_ENUM(ESIF_PARTICIPANT_ENUM_PLAT);
	ESIF_CASE_ENUM(ESIF_PARTICIPANT_ENUM_CONJURE);
	ESIF_CASE_ENUM(ESIF_PARTICIPANT_ENUM_USB);
	ESIF_CASE_ENUM(ESIF_PARTICIPANT_ENUM_SYSFS);
	}
	return ESIF_NOT_AVAILABLE;
}

#ifdef ESIF_ATTR_USER
#ifdef esif_ccb_stricmp
static ESIF_INLINE esif_participant_enum_t esif_participant_enum_str2enum(esif_string name)
{
	struct esif_participant_enum_map_t {
		esif_participant_enum_t type;
		esif_string name;
	}
	esif_participant_enum_map[] = {
		ESIF_MAP_ENUM(ESIF_PARTICIPANT_ENUM_INVALID),
		ESIF_MAP_ENUM(ESIF_PARTICIPANT_ENUM_ACPI),
		ESIF_MAP_ENUM(ESIF_PARTICIPANT_ENUM_PCI),
		ESIF_MAP_ENUM(ESIF_PARTICIPANT_ENUM_PLAT),
		ESIF_MAP_ENUM(ESIF_PARTICIPANT_ENUM_CONJURE),
		ESIF_MAP_ENUM(ESIF_PARTICIPANT_ENUM_USB),
		ESIF_MAP_ENUM(ESIF_PARTICIPANT_ENUM_SYSFS),
	};

	/* Match ESIF_PARTICIPANT_ENUM_NAME or TYPENAME */
	for (size_t j = 0; j < ESIF_ARRAY_LEN(esif_participant_enum_map); j++) {
		if (esif_ccb_stricmp(esif_participant_enum_map[j].name, name) == 0)
			return esif_participant_enum_map[j].type;
		if (esif_ccb_stricmp(esif_participant_enum_map[j].name + 22, name) == 0)
			return esif_participant_enum_map[j].type;
	}
	return (esif_participant_enum_t)(-1);
}
#endif
#endif
