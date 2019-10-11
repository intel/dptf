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
 * ACPI Devices
 */

#define ESIF_ACPI_DEVICE_INT3400	"INT3400"
#define ESIF_ACPI_DEVICE_INT3401	"INT3401"
#define ESIF_ACPI_DEVICE_INT3402	"INT3402"
#define ESIF_ACPI_DEVICE_INT3403	"INT3403"
#define ESIF_ACPI_DEVICE_INT3404	"INT3404"
#define ESIF_ACPI_DEVICE_INT3405	"INT3405"
#define ESIF_ACPI_DEVICE_INT3406	"INT3406"
#define ESIF_ACPI_DEVICE_INT3407	"INT3407"
#define ESIF_ACPI_DEVICE_INT3408	"INT3408"
#define ESIF_ACPI_DEVICE_INT3409	"INT3409"
#define ESIF_ACPI_DEVICE_INT340A	"INT340A"
#define ESIF_ACPI_DEVICE_INT340B	"INT340B"
#define ESIF_ACPI_DEVICE_INT340C	"INT340C"
#define ESIF_ACPI_DEVICE_INT340D	"INT340D"
#define ESIF_ACPI_DEVICE_INT3530	"INT3530"
#define ESIF_ACPI_DEVICE_INT3532	"INT3532"

static ESIF_INLINE esif_string esif_acpi_device_str(esif_string acpi_device)
{
	struct esif_acpi_device_map_t {
		esif_string id;
		esif_string name;
	}
	esif_acpi_device_map[] = {
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3400,
		"Manager Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3401,
		"Processor Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3402,
		"Memory Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3403,
		"Generic Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3404,
		"Fan Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3405,
		"FIVR Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3406,
		"Display Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3407,
		"Power Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3408,
		"Wireless Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3409,
		"Ambient Temperature Sensor Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT340A,
		"Storage Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT340B,
		"Intel(R) RealSense(TM) 3D Camera Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT340C,
		"Thunderbolt Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT340D,
		"Discrete Graphics Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3530,
		"MultiChip Package"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INT3532,
		"Battery Participant"),
	};
	int j;
	for (j = 0; j < ESIF_ARRAY_LEN(esif_acpi_device_map); j++) {
		if (strncmp(acpi_device, esif_acpi_device_map[j].id, 7) == 0)
			return esif_acpi_device_map[j].name;
	}
	return ESIF_NOT_AVAILABLE;
}


#if defined(ESIF_FEAT_OPT_USE_VIRT_DRVRS)
#if defined(ESIF_ATTR_OS_LINUX)

#pragma pack(push, 1)

const struct acpi_device_id esif_acpi_ids[] = {
	{ ESIF_ACPI_DEVICE_INT3400, 0 },
	{ ESIF_ACPI_DEVICE_INT3401, 0 },
	{ ESIF_ACPI_DEVICE_INT3402, 0 },
	{ ESIF_ACPI_DEVICE_INT3403, 0 },
	{ ESIF_ACPI_DEVICE_INT3404, 0 },
	{ ESIF_ACPI_DEVICE_INT3405, 0 },
	{ ESIF_ACPI_DEVICE_INT3406, 0 },
	{ ESIF_ACPI_DEVICE_INT3407, 0 },
	{ ESIF_ACPI_DEVICE_INT3408, 0 },
	{ ESIF_ACPI_DEVICE_INT3409, 0 },
	{ ESIF_ACPI_DEVICE_INT340A, 0 },
	{ ESIF_ACPI_DEVICE_INT340B, 0 },
	{ ESIF_ACPI_DEVICE_INT340C, 0 },
	{ ESIF_ACPI_DEVICE_INT340D, 0 },
	{ ESIF_ACPI_DEVICE_INT3530, 0 },
	{ ESIF_ACPI_DEVICE_INT3532, 0 },
	{ "", 0 },
};

#pragma pack(pop)
#endif
#endif
