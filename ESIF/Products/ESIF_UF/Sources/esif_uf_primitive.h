/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_PRIMITIVE_
#define _ESIF_UF_PRIMITIVE_

#include "esif_uf_primitive.h"
#include "esif_participant.h"
#include "esif_dsp.h"

#define ESIF_DATA_RETYPE(name, old_type, new_type, set_flag)	\
if (name == old_type) {	\
	name = new_type;	\
	set_flag = 1;		\
}


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Execute Primitive
 * NOTE: This version should only be called by functions external to the
 * participant, as it will require acquisition of locks for the
 * Participant Manager and participant. EsifUp_ExecutePrimitive must be called
 * from within the participant/domain when locks are already held or when
 * executing a primitive from within the context of an action, for
 * example within the "delegate" or "sysfs" actions.
 */
eEsifError EsifExecutePrimitive(
	const UInt8 participantId,
	const UInt32 primitiveId,
	const EsifString qualifier,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

Bool EsifPrimitiveVerifyOpcode(
	const UInt8 participantId,
	const UInt32 primitiveId,
	const EsifString qualifier,
	const UInt8 instance,
	enum esif_primitive_opcode opcode
	);

void EsifUfDumpPrimitive(
	EsifDspPtr dspPtr,
	EsifFpcPrimitivePtr primitivePtr
	);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_PRIMITIVE_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
