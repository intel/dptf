/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
 * Algorithm Type
 */

enum esif_algorithm_type {
	ESIF_ALGORITHM_TYPE_PERCENT_BINARY = 22,
	ESIF_ALGORITHM_TYPE_PERCENT_CENTI = 21,
	ESIF_ALGORITHM_TYPE_PERCENT_DECI = 23,
	ESIF_ALGORITHM_TYPE_PERCENT_NONE = 20,
	ESIF_ALGORITHM_TYPE_PERCENT_WHOLE = 24,
	ESIF_ALGORITHM_TYPE_POWER_DECIW = 0,
	ESIF_ALGORITHM_TYPE_POWER_MICROW = 25,
	ESIF_ALGORITHM_TYPE_POWER_MILLIW = 1,
	ESIF_ALGORITHM_TYPE_POWER_NONE = 2,
	ESIF_ALGORITHM_TYPE_POWER_UNIT_ATOM = 12,
	ESIF_ALGORITHM_TYPE_POWER_UNIT_CORE = 3,
	ESIF_ALGORITHM_TYPE_TEMP_C = 18,
	ESIF_ALGORITHM_TYPE_TEMP_C_DIV2 = 15,
	ESIF_ALGORITHM_TYPE_TEMP_DECIC = 14,
	ESIF_ALGORITHM_TYPE_TEMP_DECIK = 5,
	ESIF_ALGORITHM_TYPE_TEMP_DTS_ATOM = 11,
	ESIF_ALGORITHM_TYPE_TEMP_LPAT = 13,
	ESIF_ALGORITHM_TYPE_TEMP_MILLIC = 9,
	ESIF_ALGORITHM_TYPE_TEMP_NONE = 6,
	ESIF_ALGORITHM_TYPE_TEMP_PCH_CORE = 4,
	ESIF_ALGORITHM_TYPE_TEMP_TJMAX_ATOM = 10,
	ESIF_ALGORITHM_TYPE_TEMP_TJMAX_CORE = 7,
	ESIF_ALGORITHM_TYPE_TIME_DECIS = 16,
	ESIF_ALGORITHM_TYPE_TIME_MILLIS = 17,
	ESIF_ALGORITHM_TYPE_TIME_NONE = 8,
	ESIF_ALGORITHM_TYPE_TIME_TAU = 19,
};

static ESIF_INLINE esif_string esif_algorithm_type_str(
enum esif_algorithm_type type)
{
	switch (type) {
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_PERCENT_BINARY);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_PERCENT_CENTI);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_PERCENT_DECI);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_PERCENT_NONE);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_PERCENT_WHOLE);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_POWER_DECIW);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_POWER_MICROW);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_POWER_MILLIW);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_POWER_NONE);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_POWER_UNIT_ATOM);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_POWER_UNIT_CORE);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_C);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_C_DIV2);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_DECIC);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_DECIK);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_DTS_ATOM);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_LPAT);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_MILLIC);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_NONE);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_PCH_CORE);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_TJMAX_ATOM);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TEMP_TJMAX_CORE);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TIME_DECIS);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TIME_MILLIS);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TIME_NONE);
	ESIF_CASE_ENUM(ESIF_ALGORITHM_TYPE_TIME_TAU);
	}
	return ESIF_NOT_AVAILABLE;
}
