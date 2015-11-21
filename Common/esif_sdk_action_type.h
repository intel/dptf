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
 * Action Type Declarations
 */

enum esif_action_type {
	ESIF_ACTION_ACPI = 4,
	ESIF_ACTION_ACPIDECIC = 55,
	ESIF_ACTION_ACPILPAT = 40,
	ESIF_ACTION_CODE = 38,
	ESIF_ACTION_CONFIG = 20,
	ESIF_ACTION_CONST = 33,
	ESIF_ACTION_DDIGFXDISP = 35,
	ESIF_ACTION_DDIGFXPERF = 36,
	ESIF_ACTION_DELEGATE = 60,
	ESIF_ACTION_DPPEBRT = 32,
	ESIF_ACTION_DPTFWWAN = 50,
	ESIF_ACTION_GFXESC = 30,
	ESIF_ACTION_IOSF = 34,
	ESIF_ACTION_KDELEGATE = 52,
	ESIF_ACTION_KODE = 29,
	ESIF_ACTION_KONST = 1,
	ESIF_ACTION_LAL = 11,
	ESIF_ACTION_MMIO = 2,
	ESIF_ACTION_MMIODIV2 = 56,
	ESIF_ACTION_MMIOTJMAX = 48,
	ESIF_ACTION_MSR = 22,
	ESIF_ACTION_MSRHDC = 58,
	ESIF_ACTION_PERC = 61,
	ESIF_ACTION_PSM = 44,
	ESIF_ACTION_RFPWIFI = 43,
	ESIF_ACTION_RFPWWAN = 41,
	ESIF_ACTION_SYSFS = 59,
	ESIF_ACTION_SYSTEM = 31,
	ESIF_ACTION_SYSTEMIO = 24,
	ESIF_ACTION_USBFAN = 49,
	ESIF_ACTION_VIRTUAL = 39,
};

static ESIF_INLINE esif_string esif_action_type_str(enum esif_action_type type)
{
	switch (type) {
	ESIF_CASE_ENUM(ESIF_ACTION_ACPI);
	ESIF_CASE_ENUM(ESIF_ACTION_ACPIDECIC);
	ESIF_CASE_ENUM(ESIF_ACTION_ACPILPAT);
	ESIF_CASE_ENUM(ESIF_ACTION_CODE);
	ESIF_CASE_ENUM(ESIF_ACTION_CONFIG);
	ESIF_CASE_ENUM(ESIF_ACTION_CONST);
	ESIF_CASE_ENUM(ESIF_ACTION_DDIGFXDISP);
	ESIF_CASE_ENUM(ESIF_ACTION_DDIGFXPERF);
	ESIF_CASE_ENUM(ESIF_ACTION_DELEGATE);
	ESIF_CASE_ENUM(ESIF_ACTION_DPPEBRT);
	ESIF_CASE_ENUM(ESIF_ACTION_DPTFWWAN);
	ESIF_CASE_ENUM(ESIF_ACTION_GFXESC);
	ESIF_CASE_ENUM(ESIF_ACTION_IOSF);
	ESIF_CASE_ENUM(ESIF_ACTION_KDELEGATE);
	ESIF_CASE_ENUM(ESIF_ACTION_KODE);
	ESIF_CASE_ENUM(ESIF_ACTION_KONST);
	ESIF_CASE_ENUM(ESIF_ACTION_LAL);
	ESIF_CASE_ENUM(ESIF_ACTION_MMIO);
	ESIF_CASE_ENUM(ESIF_ACTION_MMIODIV2);
	ESIF_CASE_ENUM(ESIF_ACTION_MMIOTJMAX);
	ESIF_CASE_ENUM(ESIF_ACTION_MSR);
	ESIF_CASE_ENUM(ESIF_ACTION_MSRHDC);
	ESIF_CASE_ENUM(ESIF_ACTION_PERC);
	ESIF_CASE_ENUM(ESIF_ACTION_PSM);
	ESIF_CASE_ENUM(ESIF_ACTION_RFPWIFI);
	ESIF_CASE_ENUM(ESIF_ACTION_RFPWWAN);
	ESIF_CASE_ENUM(ESIF_ACTION_SYSFS);
	ESIF_CASE_ENUM(ESIF_ACTION_SYSTEM);
	ESIF_CASE_ENUM(ESIF_ACTION_SYSTEMIO);
	ESIF_CASE_ENUM(ESIF_ACTION_USBFAN);
	ESIF_CASE_ENUM(ESIF_ACTION_VIRTUAL);
	}
	return ESIF_NOT_AVAILABLE;
}

/*
 * Action MSR Hint Declarations
 */

enum esif_action_msr_hint {
	ESIF_ACTION_MSR_HINT_UNI = 1,
	ESIF_ACTION_MSR_HINT_MAX = 2,
	ESIF_ACTION_MSR_HINT_MIN = 3,
	ESIF_ACTION_MSR_HINT_MUL = 4,
};

static ESIF_INLINE esif_string esif_action_msr_hint_str(
	enum esif_action_msr_hint hint
	)
{
	switch (hint) {
	ESIF_CASE_ENUM(ESIF_ACTION_MSR_HINT_UNI);
	ESIF_CASE_ENUM(ESIF_ACTION_MSR_HINT_MAX);
	ESIF_CASE_ENUM(ESIF_ACTION_MSR_HINT_MIN);
	ESIF_CASE_ENUM(ESIF_ACTION_MSR_HINT_MUL);
	}
	return ESIF_NOT_AVAILABLE;
}
