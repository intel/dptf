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

/*
 * ACPI Devices
 */

#define ESIF_ACPI_DEVICE_INT3400	"INT3400"	/* Manager Participant */
#define ESIF_ACPI_DEVICE_INT3401	"INT3401"	/* Processor Participant */
#define ESIF_ACPI_DEVICE_INT3402	"INT3402"	/* Memory Participant */
#define ESIF_ACPI_DEVICE_INT3403	"INT3403"	/* Generic Participant */
#define ESIF_ACPI_DEVICE_INT3404	"INT3404"	/* Fan Participant */
#define ESIF_ACPI_DEVICE_INT3405	"INT3405"	/* FIVR Participant */
#define ESIF_ACPI_DEVICE_INT3406	"INT3406"	/* Display Participant */
#define ESIF_ACPI_DEVICE_INT3407	"INT3407"	/* Power Participant */
#define ESIF_ACPI_DEVICE_INT3408	"INT3408"	/* Wireless Participant */
#define ESIF_ACPI_DEVICE_INT3409	"INT3409"	/* Ambient Temperature Sensor Participant */
#define ESIF_ACPI_DEVICE_INT340A	"INT340A"	/* Storage Participant */
#define ESIF_ACPI_DEVICE_INT340B	"INT340B"	/* Intel(R) RealSense(TM) 3D Camera Participant */
#define ESIF_ACPI_DEVICE_INT340C	"INT340C"	/* Thunderbolt Participant */
#define ESIF_ACPI_DEVICE_INT340D	"INT340D"	/* Discrete Graphics Participant */
#define ESIF_ACPI_DEVICE_INT3530	"INT3530"	/* MultiChip Package */
#define ESIF_ACPI_DEVICE_INT3532	"INT3532"	/* Battery Participant */
#define ESIF_ACPI_DEVICE_INTC1040	"INTC1040"	/* Manager Participant [TGL,RKL,JSL] */
#define ESIF_ACPI_DEVICE_INTC1041	"INTC1041"	/* Manager Participant [ADL] */
#define ESIF_ACPI_DEVICE_INTC1042	"INTC1042"	/* Manager Participant [MTL] */
#define ESIF_ACPI_DEVICE_INTC1043	"INTC1043"	/* Generic Participant [TGL,RKL,JSL] */
#define ESIF_ACPI_DEVICE_INTC1044	"INTC1044"	/* Fan Participant [TGL,RKL,JSL] */
#define ESIF_ACPI_DEVICE_INTC1045	"INTC1045"	/* FIVR Participant [TGL,RKL,JSL] */
#define ESIF_ACPI_DEVICE_INTC1046	"INTC1046"	/* Generic Participant [ADL] */
#define ESIF_ACPI_DEVICE_INTC1047	"INTC1047"	/* Power Participant [TGL,RKL,JSL] */
#define ESIF_ACPI_DEVICE_INTC1048	"INTC1048"	/* Fan Participant [ADL] */
#define ESIF_ACPI_DEVICE_INTC1049	"INTC1049"	/* FIVR Participant [ADL] */
#define ESIF_ACPI_DEVICE_INTC1050	"INTC1050"	/* Battery Participant [TGL,RKL,JSL] */
#define ESIF_ACPI_DEVICE_INTC1060	"INTC1060"	/* Power Participant [ADL] */
#define ESIF_ACPI_DEVICE_INTC1061	"INTC1061"	/* Battery Participant [ADL] */
#define ESIF_ACPI_DEVICE_INTC1062	"INTC1062"	/* Generic Participant [MTL] */
#define ESIF_ACPI_DEVICE_INTC1063	"INTC1063"	/* Fan Participant [MTL] */
#define ESIF_ACPI_DEVICE_INTC1064	"INTC1064"	/* FIVR Participant [MTL] */
#define ESIF_ACPI_DEVICE_INTC1065	"INTC1065"	/* Power Participant [MTL] */
#define ESIF_ACPI_DEVICE_INTC1066	"INTC1066"	/* Battery Participant [MTL] */
#define ESIF_ACPI_DEVICE_INTC1068	"INTC1068"	/* Manager Participant [LNL] */
#define ESIF_ACPI_DEVICE_INTC1069	"INTC1069"	/* Generic Participant [LNL] */
#define ESIF_ACPI_DEVICE_INTC106A	"INTC106A"	/* Fan Participant [LNL] */
#define ESIF_ACPI_DEVICE_INTC106B	"INTC106B"	/* FIVR Participant [LNL] */
#define ESIF_ACPI_DEVICE_INTC106C	"INTC106C"	/* Power Participant [LNL] */
#define ESIF_ACPI_DEVICE_INTC106D	"INTC106D"	/* Battery Participant [LNL] */
#define ESIF_ACPI_DEVICE_INTC10A0	"INTC10A0"	/* Manager Participant [RPL] */
#define ESIF_ACPI_DEVICE_INTC10A1	"INTC10A1"	/* Generic Participant [RPL] */
#define ESIF_ACPI_DEVICE_INTC10A2	"INTC10A2"	/* Fan Participant [RPL] */
#define ESIF_ACPI_DEVICE_INTC10A3	"INTC10A3"	/* FIVR Participant [RPL] */
#define ESIF_ACPI_DEVICE_INTC10A4	"INTC10A4"	/* Power Participant [RPL] */
#define ESIF_ACPI_DEVICE_INTC10A5	"INTC10A5"	/* Battery Participant [RPL] */
#define ESIF_ACPI_DEVICE_INTC10D4	"INTC10D4"	/* Manager Participant [PTL] */
#define ESIF_ACPI_DEVICE_INTC10D5	"INTC10D5"	/* Generic Participant [PTL] */
#define ESIF_ACPI_DEVICE_INTC10D6	"INTC10D6"	/* Fan Participant [PTL] */
#define ESIF_ACPI_DEVICE_INTC10D7	"INTC10D7"	/* FIVR Participant [PTL] */
#define ESIF_ACPI_DEVICE_INTC10D8	"INTC10D8"	/* Power Participant [PTL] */
#define ESIF_ACPI_DEVICE_INTC10D9	"INTC10D9"	/* Battery Participant [PTL] */

static ESIF_INLINE esif_string esif_acpi_device_str(esif_string acpi_device)
{
	static struct esif_acpi_device_map_t {
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
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1040,
		"Manager Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1041,
		"Manager Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1042,
		"Manager Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1043,
		"Generic Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1044,
		"Fan Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1045,
		"FIVR Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1046,
		"Generic Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1047,
		"Power Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1048,
		"Fan Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1049,
		"FIVR Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1050,
		"Battery Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1060,
		"Power Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1061,
		"Battery Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1062,
		"Generic Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1063,
		"Fan Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1064,
		"FIVR Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1065,
		"Power Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1066,
		"Battery Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1068,
		"Manager Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC1069,
		"Generic Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC106A,
		"Fan Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC106B,
		"FIVR Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC106C,
		"Power Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC106D,
		"Battery Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10A0,
		"Manager Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10A1,
		"Generic Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10A2,
		"Fan Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10A3,
		"FIVR Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10A4,
		"Power Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10A5,
		"Battery Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10D4,
		"Manager Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10D5,
		"Generic Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10D6,
		"Fan Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10D7,
		"FIVR Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10D8,
		"Power Participant"),
	ESIF_MAP(ESIF_ACPI_DEVICE_INTC10D9,
		"Battery Participant"),
	};
	for (int j = 0; j < ESIF_ARRAY_LEN(esif_acpi_device_map); j++) {
		if (strncmp(acpi_device, esif_acpi_device_map[j].id, esif_ccb_strlen(esif_acpi_device_map[j].id, 8)) == 0)
			return esif_acpi_device_map[j].name;
	}
	return ESIF_NOT_AVAILABLE;
}


#if defined(ESIF_FEAT_OPT_USE_VIRT_DRVRS)

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
	{ ESIF_ACPI_DEVICE_INTC1040, 0 },
	{ ESIF_ACPI_DEVICE_INTC1041, 0 },
	{ ESIF_ACPI_DEVICE_INTC1042, 0 },
	{ ESIF_ACPI_DEVICE_INTC1043, 0 },
	{ ESIF_ACPI_DEVICE_INTC1044, 0 },
	{ ESIF_ACPI_DEVICE_INTC1045, 0 },
	{ ESIF_ACPI_DEVICE_INTC1046, 0 },
	{ ESIF_ACPI_DEVICE_INTC1047, 0 },
	{ ESIF_ACPI_DEVICE_INTC1048, 0 },
	{ ESIF_ACPI_DEVICE_INTC1049, 0 },
	{ ESIF_ACPI_DEVICE_INTC1050, 0 },
	{ ESIF_ACPI_DEVICE_INTC1060, 0 },
	{ ESIF_ACPI_DEVICE_INTC1061, 0 },
	{ ESIF_ACPI_DEVICE_INTC1062, 0 },
	{ ESIF_ACPI_DEVICE_INTC1063, 0 },
	{ ESIF_ACPI_DEVICE_INTC1064, 0 },
	{ ESIF_ACPI_DEVICE_INTC1065, 0 },
	{ ESIF_ACPI_DEVICE_INTC1066, 0 },
	{ ESIF_ACPI_DEVICE_INTC1068, 0 },
	{ ESIF_ACPI_DEVICE_INTC1069, 0 },
	{ ESIF_ACPI_DEVICE_INTC106A, 0 },
	{ ESIF_ACPI_DEVICE_INTC106B, 0 },
	{ ESIF_ACPI_DEVICE_INTC106C, 0 },
	{ ESIF_ACPI_DEVICE_INTC106D, 0 },
	{ ESIF_ACPI_DEVICE_INTC10A0, 0 },
	{ ESIF_ACPI_DEVICE_INTC10A1, 0 },
	{ ESIF_ACPI_DEVICE_INTC10A2, 0 },
	{ ESIF_ACPI_DEVICE_INTC10A3, 0 },
	{ ESIF_ACPI_DEVICE_INTC10A4, 0 },
	{ ESIF_ACPI_DEVICE_INTC10A5, 0 },
	{ ESIF_ACPI_DEVICE_INTC10D4, 0 },
	{ ESIF_ACPI_DEVICE_INTC10D5, 0 },
	{ ESIF_ACPI_DEVICE_INTC10D6, 0 },
	{ ESIF_ACPI_DEVICE_INTC10D7, 0 },
	{ ESIF_ACPI_DEVICE_INTC10D8, 0 },
	{ ESIF_ACPI_DEVICE_INTC10D9, 0 },
	{ "", 0 },
};

#pragma pack(pop)
#endif
