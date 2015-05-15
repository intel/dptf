/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#ifndef _ESIF_UF_XFORM_
#define _ESIF_UF_XFORM_

#include "esif.h"
#include "esif_temp.h"
#include "esif_power.h"
#include "esif_time.h"
#include "esif_percent.h"
#include "esif_dsp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Transforms */
enum esif_rc EsifUfXformTime(
	const enum esif_time_type type,
	esif_time_t *time_ptr,
	const enum esif_action_type action,
	const EsifDspPtr dsp_ptr,
	const enum esif_primitive_opcode opcode
);

enum esif_rc EsifUfXformPower(
	const enum esif_power_unit_type type,
	esif_power_t *power_ptr,
	const enum esif_action_type action,
	const EsifDspPtr dsp_ptr,
	const enum esif_primitive_opcode opcode
);

enum esif_rc EsifUfXformTemp(
	const enum esif_temperature_type type,
	esif_temp_t *temp_ptr,
	const enum esif_action_type action,
	const EsifDspPtr dsp_ptr,
	const enum esif_primitive_opcode opcode
);

void EsifUfExecuteTransform(
	const EsifDataPtr transformDataPtr,
	const enum esif_action_type action,
	const EsifDspPtr dsp_ptr,
	const enum esif_primitive_opcode opcode);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_XFORM_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

