/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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


#include "esif_sdk_data.h"
#include "esif_sdk_iface.h"
#include "esif_sdk_iface_participant.h"

/*
 * Name of the function that must be exported to initialize the interface (see function prototype below)
 */
#define CONJURE_GET_INTERFACE_FUNCTION "GetConjureInterface"

#define ESIF_PARTICIPANT_CONJURE_CLASS_GUID {0xe3, 0x78, 0x02, 0xdf, 0xdf, 0x3d, 0x46, 0xa7, 0xb9, 0x9b, 0x1f, 0x1c, 0x78, 0x5f, 0xd9, 0x1b}

typedef enum EsifCnjIfaceVer_e {
	ESIF_CNJ_IFACE_VER_INVALID = -1,
	ESIF_CNJ_IFACE_VER_V1 = 1,
	ESIF_CNJ_FACE_VER_MAX = ESIF_CNJ_IFACE_VER_V1
}EsifCnjIfaceVer, *EsifCnjIfaceVerPtr;


#define CONJURE_IFACE_VERSION ESIF_CNJ_IFACE_VER_V1
/*
    =============================================================================
    Conjure Service Interface (Conjure -> ESIF)
    =============================================================================
 */

/*
    Upper Framework Register Participant. Provides the same functionality as the LF
    version.  Maybe called many times by one Conjure Library.
 */
typedef eEsifError (ESIF_CALLCONV *RegisterParticipantFunction)(const EsifParticipantIfacePtr pi, esif_handle_t *participantInstance);

/*
    Upper Framework UnRegister Participant.  Provides the same functionality as the LF
    version.  Maybe called many times by one Conjure Library.
 */
typedef eEsifError (ESIF_CALLCONV *UnRegisterParticipantFunction)(esif_handle_t participantHandle);

/* ESIF Conjure Services Interface */
#pragma pack(push, 1)

typedef struct _t_EsifConjureServiceInterface {
	/* Header */
	eIfaceType  fIfaceType;
	UInt16      fIfaceVersion;
	UInt16      fIfaceSize;

	/* Function Pointers */
	RegisterParticipantFunction    fRegisterParticipantFuncPtr;
	UnRegisterParticipantFunction  fUnRegisterParticipantFuncPtr;
} EsifConjureServiceInterface, *EsifConjureServiceInterfacePtr, **EsifConjureServiceInteracePtrLocation;

#pragma pack(pop)

/*
    =============================================================================
    Conjure Interface (ESIF -> Conjure)
    =============================================================================
 */

typedef eEsifError (ESIF_CALLCONV *ConjureGetAboutFunction)(EsifDataPtr conjureAbout);
typedef eEsifError (ESIF_CALLCONV *ConjureGetDescriptionFunction)(EsifDataPtr conjureDescription);
typedef eEsifError (ESIF_CALLCONV *ConjureGetGuidFunction)(EsifDataPtr conjureGuid);
typedef eEsifError (ESIF_CALLCONV *ConjureGetNameFunction)(EsifDataPtr conjureName);
typedef eEsifError (ESIF_CALLCONV *ConjureGetVersionFunction)(EsifDataPtr conjureVersion);

typedef eEsifError (ESIF_CALLCONV *ConjureCreateFunction)(
	EsifConjureServiceInterfacePtr esifServiceInterface,
	const void *esifHandle,/* ESIF will provide Conjure MUST save for use with callbacks */
	void **conjureHandleLocation/* The Conjure MUST provide esif will save for use with callbacks */
);

/*
    DEPENDENCY
    The remaining funcitons are dependent on conjure create.
 */
typedef eEsifError (ESIF_CALLCONV *ConjureDestroyFunction)(void *conjureHandle);

/* ESIF Conjure Interface */

#pragma pack(push, 1)

typedef struct _t_EsifConjureInterface {
	EsifIfaceHdr hdr;

	esif_flags_t flags;

	char name[ESIF_NAME_LEN];
	char desc[ESIF_DESC_LEN];

	UInt16 cnjVersion; /* Version of the conjure (not the interface) */

	/* Function Pointers */
	ConjureCreateFunction      fConjureCreateFuncPtr;
	ConjureDestroyFunction     fConjureDestroyFuncPtr;
} EsifConjureInterface, *EsifConjureInterfacePtr, **EsifConjureInteracePtrLocation;

#pragma pack(pop)

