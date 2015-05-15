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

enum esif_apct_cond
{
	ESIF_APCT_COND_DO_NOT_CARE = 1,
	ESIF_APCT_COND_PLATFORM_ORIENTATION = 2,
	ESIF_APCT_COND_USER_PROXIMITY = 3,
	ESIF_APCT_COND_IN_MOTION = 4,
	ESIF_APCT_COND_DOCK_MODE = 5,
	ESIF_APCT_COND_WORKLOAD = 6,
	ESIF_APCT_COND_COOLING_MODE = 7,
	ESIF_APCT_COND_POWER_SOURCE = 8,
	ESIF_APCT_COND_BATTERY_PERCENTAGE = 9,
	ESIF_APCT_COND_LID_STATE = 10,
	ESIF_APCT_COND_PLATFORM_TYPE = 11,
	ESIF_APCT_COND_PLATFORM_SKU = 12,
	ESIF_APCT_COND_UTILIZATION = 13,
	ESIF_APCT_COND_CONFIG_TDP_LEVEL = 14,
	ESIF_APCT_COND_DUTY_CYCLE = 15,
	ESIF_APCT_COND_POWER = 16,
	ESIF_APCT_COND_TEMPERATURE = 17,
	ESIF_APCT_COND_DISPLAY_ORIENTATION = 18,
	ESIF_APCT_COND_OEM_VARIABLE_00 = 19,
	ESIF_APCT_COND_OEM_VARIABLE_01 = 20,
	ESIF_APCT_COND_OEM_VARIABLE_02 = 21,
	ESIF_APCT_COND_OEM_VARIABLE_03 = 22,
	ESIF_APCT_COND_OEM_VARIABLE_04 = 23,
	ESIF_APCT_COND_OEM_VARIABLE_05 = 24,
};

static ESIF_INLINE esif_string esif_apct_cond_str(
	enum esif_apct_cond cond)
{
	switch (cond) {
	ESIF_CASE_ENUM(ESIF_APCT_COND_DO_NOT_CARE);
	ESIF_CASE_ENUM(ESIF_APCT_COND_PLATFORM_ORIENTATION);
	ESIF_CASE_ENUM(ESIF_APCT_COND_USER_PROXIMITY);
	ESIF_CASE_ENUM(ESIF_APCT_COND_IN_MOTION);
	ESIF_CASE_ENUM(ESIF_APCT_COND_DOCK_MODE);
	ESIF_CASE_ENUM(ESIF_APCT_COND_WORKLOAD);
	ESIF_CASE_ENUM(ESIF_APCT_COND_COOLING_MODE);
	ESIF_CASE_ENUM(ESIF_APCT_COND_POWER_SOURCE);
	ESIF_CASE_ENUM(ESIF_APCT_COND_BATTERY_PERCENTAGE);
	ESIF_CASE_ENUM(ESIF_APCT_COND_LID_STATE);
	ESIF_CASE_ENUM(ESIF_APCT_COND_PLATFORM_TYPE);
	ESIF_CASE_ENUM(ESIF_APCT_COND_PLATFORM_SKU);
	ESIF_CASE_ENUM(ESIF_APCT_COND_UTILIZATION);
	ESIF_CASE_ENUM(ESIF_APCT_COND_CONFIG_TDP_LEVEL);
	ESIF_CASE_ENUM(ESIF_APCT_COND_DUTY_CYCLE);
	ESIF_CASE_ENUM(ESIF_APCT_COND_POWER);
	ESIF_CASE_ENUM(ESIF_APCT_COND_TEMPERATURE);
	ESIF_CASE_ENUM(ESIF_APCT_COND_DISPLAY_ORIENTATION);
	ESIF_CASE_ENUM(ESIF_APCT_COND_OEM_VARIABLE_00);
	ESIF_CASE_ENUM(ESIF_APCT_COND_OEM_VARIABLE_01);
	ESIF_CASE_ENUM(ESIF_APCT_COND_OEM_VARIABLE_02);
	ESIF_CASE_ENUM(ESIF_APCT_COND_OEM_VARIABLE_03);
	ESIF_CASE_ENUM(ESIF_APCT_COND_OEM_VARIABLE_04);
	ESIF_CASE_ENUM(ESIF_APCT_COND_OEM_VARIABLE_05);
	}
	return ESIF_NOT_AVAILABLE;
}

enum esif_apct_plat_type
{
	ESIF_APCT_PLAT_TYPE_HANDHELD = 1,
	ESIF_APCT_PLAT_TYPE_TABLET = 2,
	ESIF_APCT_PLAT_TYPE_LAPTOP = 3,
	ESIF_APCT_PLAT_TYPE_DESKTOP = 4,
	ESIF_APCT_PLAT_TYPE_SERVER = 5,
	ESIF_APCT_PLAT_TYPE_WEARABLE = 6,
};

static ESIF_INLINE esif_string esif_apct_plat_type_str(
	enum esif_apct_plat_type plat_type)
{
	switch (plat_type) {
	ESIF_CASE_ENUM(ESIF_APCT_PLAT_TYPE_HANDHELD);
	ESIF_CASE_ENUM(ESIF_APCT_PLAT_TYPE_TABLET);
	ESIF_CASE_ENUM(ESIF_APCT_PLAT_TYPE_LAPTOP);
	ESIF_CASE_ENUM(ESIF_APCT_PLAT_TYPE_DESKTOP);
	ESIF_CASE_ENUM(ESIF_APCT_PLAT_TYPE_SERVER);
	ESIF_CASE_ENUM(ESIF_APCT_PLAT_TYPE_WEARABLE);
	}
	return ESIF_NOT_AVAILABLE;
}

enum esif_apct_user_prox
{
	ESIF_APCT_USER_PROX_NEAR = 1,
	ESIF_APCT_USER_PROX_AWAY = 2,
};

static ESIF_INLINE esif_string esif_apct_user_prox_str(
	enum esif_apct_user_prox prox)
{
	switch (prox) {
	ESIF_CASE_ENUM(ESIF_APCT_USER_PROX_NEAR);
	ESIF_CASE_ENUM(ESIF_APCT_USER_PROX_AWAY);
	}
	return ESIF_NOT_AVAILABLE;
}

enum esif_apct_dock_mode
{
	ESIF_APCT_DOCK_MODE_NONE = 1,
	ESIF_APCT_DOCK_MODE_ACTIVE = 2,
	ESIF_APCT_DOCK_MODE_PASSIVE = 3,
};

static ESIF_INLINE esif_string esif_apct_dock_mode_str(
	enum esif_apct_dock_mode dock_mode)
{
	switch (dock_mode) {
	ESIF_CASE_ENUM(ESIF_APCT_DOCK_MODE_NONE);
	ESIF_CASE_ENUM(ESIF_APCT_DOCK_MODE_ACTIVE);
	ESIF_CASE_ENUM(ESIF_APCT_DOCK_MODE_PASSIVE);
	}
	return ESIF_NOT_AVAILABLE;
}

enum esif_apct_operator
{
	ESIF_APCT_OPERATOR_AND = 1
};

static ESIF_INLINE esif_string esif_apct_operator_str(
	enum esif_apct_operator op)
{
	switch (op) {
	ESIF_CASE_ENUM(ESIF_APCT_OPERATOR_AND);
	}
	return ESIF_NOT_AVAILABLE;
}

enum esif_apct_comparison
{
	ESIF_APCT_COMPARISON_EQUAL = 1,
	ESIF_APCT_COMPARISON_LESS_OR_EQUAL = 2,
	ESIF_APCT_COMPARISON_GREATER_OR_EQUAL = 3,
	ESIF_APCT_COMPARISON_NOT_EQUAL = 4
};

static ESIF_INLINE esif_string esif_apct_comparison_str(
	enum esif_apct_comparison comparison)
{
	switch (comparison) {
	ESIF_CASE_ENUM(ESIF_APCT_COMPARISON_EQUAL);
	ESIF_CASE_ENUM(ESIF_APCT_COMPARISON_LESS_OR_EQUAL);
	ESIF_CASE_ENUM(ESIF_APCT_COMPARISON_GREATER_OR_EQUAL);
	ESIF_CASE_ENUM(ESIF_APCT_COMPARISON_NOT_EQUAL);
	}
	return ESIF_NOT_AVAILABLE;
}