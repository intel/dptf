/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "esif.h"
#include "esif_ccb.h"
#include "esif_ccb_rc.h"

//
// WARNING:  The value of enumeration is the index into the g_arbitrationFunctions table.  
// The two items must match up at all times.
//
typedef enum esif_arbitration_type_e {
	ESIF_ARBITRATION_INVALID = 0,
	ESIF_ARBITRATION_UIN32_GREATER_THAN = 1,
	ESIF_ARBITRATION_UIN32_LESS_THAN = 2,
} esif_arbitration_type_t;

#define ESIF_ARBITRATION_TYPE_MAX ESIF_ARBITRATION_UIN32_LESS_THAN

#define ESIF_ARB_DATA_SIZE (sizeof(u32))
#define ESIF_ARB_LIMIT_MIN 0
#define ESIF_ARB_LIMIT_MAX ((u32)(-1))
#define ESIF_ARB_LIMIT_MAX_PERCENT 10000
#define ESIF_ARB_LIMIT_MIN_PERCENT 0

/* Provides arbitration state information for a specific primitive/instance */
typedef struct EsifArbEntryInfo_s {
	esif_handle_t participantId; /* Association metadata*/
	esif_primitive_type_t primitiveId;
	UInt16 domain;
	UInt8 instance;

	atomic_t arbitrationEnabled; /* Primitives are accepted/limited for entry */

	esif_arbitration_type_t arbType; /* Arbitration function type */
	UInt32 upperLimit; /* Limiting information */
	UInt32 lowerLimit;
} EsifArbEntryInfo;

/* Provides arbitration state information for a specific participant */
typedef struct EsifArbCtxInfo_s {
	size_t size;
	esif_handle_t participantId; /* Association metadata*/

	Bool isArbitrated; /* Has the context been populated on participant */
	Bool arbitrationEnabled;

	/* Array size depends on the count */
	size_t count;
	EsifArbEntryInfo arbEntryInfo[1];
} EsifArbCtxInfo;

/* Provides arbitration state information for all participants */
typedef struct EsifArbInfo_s {
	size_t size;
	Bool arbitrationEnabled;

	/* Array size depends on the count */
	size_t count; /* Number of participants */
	size_t arbitratedCount; /* Number of arbitrated participants */
	EsifArbCtxInfo arbCtxInfo[1];
} EsifArbInfo;


#ifdef __cplusplus
extern "C" {
#endif

#if defined(ESIF_FEAT_OPT_ARBITRATOR_ENABLED)
	/*
	 * Executes a primitive with arbitration
	 */
	esif_error_t EsifArbMgr_ExecutePrimitive(
		const esif_handle_t appHandle,
		const esif_handle_t participantId,
		const UInt32 primitiveId,
		const EsifString domainStr,
		const UInt8 instance,
		const EsifDataPtr requestPtr,
		EsifData *responsePtr
	);

	/*
	 * Gets arbitration information at various arbitration layers
	 *
	 * WARNING!!! Caller is expected to free returned pointer if not NULL
	 *
	 * If participantId == ESIF_INVALID_HANDLE; gets arbitation information
	 * for all participants
	 * Else if primitiveId is 0; gets all arbitration information for a
	 * participant
	 * Else gets information for a specific part/primitive/domain/instance
	 *
	 */
	EsifArbInfo *EsifArbMgr_GetInformation(
		const esif_handle_t participantId,
		const UInt32 primitiveId,
		const UInt16 domain,
		const UInt8 instance
	);

	/*
	 * Sets arbitration state at various arbitration layers
	 *
	 * If participantId == ESIF_INVALID_HANDLE; affects arbitation of all primitives
	 * Else if primitiveId is 0; affects arbitration for the partcipant
	 * Else affects specific primitive/domain/instance
	 *
	 * Note: When enabling, we will also enable at the higher levels so that the
	 * target level is enabled arbitrated
	 */
	esif_error_t EsifArbMgr_SetArbitrationState(
		const esif_handle_t participantId,
		const UInt32 primitiveId,
		const UInt16 domain,
		const UInt8 instance,
		const Bool isEnabled
	);

	/* Used to release the arbitration information for a given prim/inst */
	esif_error_t EsifArbMgr_StopArbitration(
		const esif_handle_t participantId,
		const UInt32 primitiveId,
		const UInt16 domain,
		const UInt8 instance
		);

	/*
	 * Sets the limits for a given participant/primitive/instance
	 * Note:  Creates the entry if not present
	 */
	esif_error_t EsifArbMgr_SetLimits(
		const esif_handle_t participantId,
		const UInt32 primitiveId,
		const UInt16 domain,
		const UInt8 instance,
		UInt32 *upperLimitPtr,
		UInt32 *lowerLimitPtr
	);

	/*
	 * Sets the arbitration type for a given participant/primitive/instance
	 * Note:  Creates the entry if not present
	 */
	esif_error_t EsifArbMgr_SetArbitrationFunction(
		const esif_handle_t participantId,
		const UInt32 primitiveId,
		const UInt16 domain,
		const UInt8 instance,
		esif_arbitration_type_t arbType
	);

	/* Boilerplate lifecycle items */
	esif_error_t EsifArbMgr_Init(void);
	void EsifArbMgr_Exit(void);

	esif_error_t EsifArbMgr_Start(void);
	void EsifArbMgr_Stop(void);

#else /* !ESIF_FEAT_OPT_ARBITRATOR_ENABLED */

#define EsifArbMgr_ExecutePrimitive(app, part, prim, dom, inst, req, rsp) EsifExecutePrimitive(part, prim, dom, inst, req, rsp)
#define EsifArbMgr_GetInformation(part, prim, dom, inst) NULL
#define EsifArbMgr_SetArbitrationState(part, prim, dom, inst, isEnabled) ESIF_E_NOT_SUPPORTED
#define EsifArbMgr_StopArbitration(part, prim, dom, inst) ESIF_E_NOT_SUPPORTED
#define EsifArbMgr_SetLimits(part, prim, dom, inst, upr, lwr) ESIF_E_NOT_SUPPORTED
#define EsifArbMgr_SetArbitrationFunction(part, prim, dom, inst, type) ESIF_E_NOT_SUPPORTED

	/* Inline to allow pointers to be take for UF intialization table */
	static ESIF_INLINE esif_error_t EsifArbMgr_Init(void) { return ESIF_OK; }
	static ESIF_INLINE void EsifArbMgr_Exit(void) {}
	static ESIF_INLINE esif_error_t EsifArbMgr_Start(void) { return ESIF_OK; }
	static ESIF_INLINE void EsifArbMgr_Stop(void) {}

#endif /* !ESIF_FEAT_OPT_ARBITRATOR_ENABLED */

#ifdef __cplusplus
}
#endif


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
