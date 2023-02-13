/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#pragma once

#include "esif_ccb.h"
#include "esif_ccb_rc.h"
#include "esif_sdk.h"
#include "esif_sdk_data.h"
#include "esif_sdk_event_type.h"
#include "esif_sdk_action_type.h"
#include "esif_sdk_iface.h"

/*
 *  TODO: All loadable action filenames must start with "upe_"
 */
#define ESIF_UPE_FILE_PREFIX "upe_"

/*
 * Name of the function that must be exported to initialize the interface (see function prototype below)
 */
#define ACTION_UPE_GET_INTERFACE_FUNCTION "GetActionInterface"

/*
 * Handle used to represent IETM (Participant 0)
 * Note:  All other participants require the handle provided during calls to the interface
 */
#define ACTION_UPE_IETM_HANDLE ESIF_HANDLE_PRIMARY_PARTICIPANT

/*
 * Value used to represent Domain 0 (only domain used in current version of the interface.)
 */
#define ACTION_UPE_DOMAIN_D0 '0D'

 /*
 * Value used to represent any participant
 */
#define ACTION_UPE_MATCH_ANY ESIF_HANDLE_MATCH_ANY_EVENT

 /*
 * Value used to represent default ESIF participant handle
 */
#define ACTION_UPE_ESIF_HANDLE_PRIMARY_PART ESIF_HANDLE_PRIMARY_PARTICIPANT

/*
 * Changes to the static interface do not require versioning, as they are built
 * into the binary.  Any changes to the interface for loadable action require\
 * versioning.
 */
typedef enum EsifActIfaceVer_e {
	ESIF_ACT_IFACE_VER_INVALID = -1,
	ESIF_ACT_IFACE_VER_STATIC = 0,
	ESIF_ACT_IFACE_VER_V1, /* Deprecated */
	ESIF_ACT_IFACE_VER_V2, /* Deprecated */
	ESIF_ACT_IFACE_VER_V3, /* Moved to participant handle-based interface; deprecated V1 and V2 support*/
	ESIF_ACT_IFACE_VER_V4, /* Added support for participant specific event registration; deprecated V3 support*/
	ESIF_ACT_FACE_VER_MAX = ESIF_ACT_IFACE_VER_V4
}EsifActIfaceVer, *EsifActIfaceVerPtr;


#define ACTION_UPE_IFACE_VERSION ESIF_ACT_IFACE_VER_V4

/* Forward declaration for use by UPE */
typedef union EsifActIface_u  *EsifActIfacePtr;


/*
 * Creates the action.  Called after the interface has been retrieved and allows
 * the action to receive interface functions from ESIF and return an
 * instance-specific context.
 */
typedef eEsifError (ESIF_CALLCONV *ActCreateFunction)(
	/* For UPE's, this can be cast to a EsifUpeIfacePtr */
	const EsifActIfacePtr actIfacePtr,
	/*
	 * To be returned by the action during this function and will be passed back
	 * in calls to the action for the specific instance associated with the
	 * given participant UID
	 */
	esif_context_t *actCtxPtr		
	);

/*
 * Called to destroy a given instance of the action; as specified by the
 * action context that was received when calling the ActCreateFunction function.
 */
typedef eEsifError (ESIF_CALLCONV *ActDestroyFunction)(esif_context_t actCtx);

/*
 * Used to implement the "GET" functionality allowing ESIF to retrieve
 * information from the action.
 */
typedef eEsifError (ESIF_CALLCONV *ActExecuteGetFunction)(
	esif_context_t actCtx, 
	const esif_handle_t participantHandle, 
	const esif_string devicePathPtr, 
	const EsifDataPtr p1Ptr, 
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr, 
	const EsifDataPtr p4Ptr, 
	const EsifDataPtr p5Ptr, 
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

/*
 * Used to implement the "SET" functionality allowing ESIF to provide
 * information to the action.
 */
typedef eEsifError (ESIF_CALLCONV *ActExecuteSetFunction)(
	esif_context_t actCtx, 
	const esif_handle_t participantHandle, 
	const esif_string devicePathPtr, 
	const EsifDataPtr p1Ptr, 
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr, 
	const EsifDataPtr p4Ptr, 
	const EsifDataPtr p5Ptr, 
	const EsifDataPtr requestPtr
	);

/*
 * Used to received events from ESIF.  The event types that are received are
 * dependent upon the UPE type.  (ESIF events are asynchronous and there is no
 * guarantee of what time in the past a given even occured.  For example, an S3
 * entry event may not be received until after reseume from S3.)
 */
typedef eEsifError (ESIF_CALLCONV *ActReceiveEventFunction)(
	esif_context_t actCtx,					/* Context provided by the UPE during action creation */
	const esif_handle_t participantHandle,	/* Pass back in calls to ESIF for the participant instance */
	enum esif_event_type eventType,			/* Event type */	
	UInt16 domain, 							/* Ignored in initial implementation */
	const EsifDataPtr dataPtr				/* Event data if required (may be NULL) */
	);

/*
 * Used to send events to ESIF - The function pointer is provided during the
 * call to create the action.
 * Note:  When executing in the context of a GET/SET call to the action, the participant handle
 * is normally the same as that passed to those interface functions.  If the action executes
 * primitives outside the context of such a function, it is the responsibility of the action to
 * maintain a binding between the devices it controls and the partcipant handles for such calls.
 * Such a binding may be initiated during the first call to access a given participant.
*/
typedef eEsifError (ESIF_CALLCONV *ActSendEventFunction)(/*  */
	const esif_handle_t participantHandle,	/* Pass back in calls to ESIF for the participant instance */
	enum esif_event_type eventType,			/* Event type */
	UInt16 domain,							/* Must Be '0D' */
	const EsifDataPtr dataPtr				/* Event data if required (dependent on event type) */
	);

/*
 * Used to send debug trace information to ESIF - The function pointer is
 * provided during the call to create the action.  (Trace message should be
 * implemented such that they are only evaluated if the specified trace level
 * is currently active.)
 */
typedef eEsifError(ESIF_CALLCONV *ActWriteLogFunction)(
	const EsifDataPtr message,	/* Message For Log */
	const eLogType logType		/* Log Type e.g. crticial, debug, info,e tc */
);

/*
* Used to register for events.
*/
typedef eEsifError(ESIF_CALLCONV *ActEventRegistrationFunction)(
	const esif_handle_t participantHandle,	/* Pass back in registration to ESIF for the participant instance */
	UInt16 domain,
	const enum esif_action_type actionType,
	const enum esif_event_type eventType,
	Bool registerEvent
);

/*
 * Used to execute primitives through ESIF.
 * Note:  When executing in the context of a GET/SET call to the action, the participant handle
 * is normally the same as that passed to those interface functions.  If the action executes
 * primitives outside the context of such a function, it is the responsibility of the action to
 * maintain a binding between the devices it controls and the partcipant handles for such calls.
 * Such a binding may be initiated during the first call to access a given participant.
 */
typedef eEsifError(ESIF_CALLCONV *ActExecutePrimitiveFunction)(
	const esif_handle_t participantHandle,	/* Pass back in calls to ESIF for the participant instance */
	const UInt16 primitiveId,
	const UInt16 domain,					/* Must Be '0D' */
	const UInt8 instance,					/* Primitive instance */
	const EsifDataPtr requestPtr,			/* Input data to the primitive */
	EsifDataPtr responsePtr					/* Output data returned by primitivie execution */
);


#pragma pack(push,1)

typedef struct EsifActIfaceUpeV4_s {
	EsifIfaceHdr hdr;

	enum esif_action_type type;
	esif_flags_t flags;

	char name[ESIF_NAME_LEN];
	char desc[ESIF_DESC_LEN];

	UInt16 actVersion; /* Version of the action (not the interface) */

	ActCreateFunction	createFuncPtr;
	ActDestroyFunction	destroyFuncPtr;

	ActExecuteGetFunction  getFuncPtr;
	ActExecuteSetFunction  setFuncPtr;

	ActReceiveEventFunction rcvEventFuncPtr;

	/*
	* Below this point are value provided by ESIF when the creation function is
	* called - Not available at the time the interface is retrieved
	*/
	ActSendEventFunction sendEventFuncPtr;	/* Filled in by ESIF */

											/*
											* traceLevel - Initial value only (The trace level will be updated through
											* ESIF_EVENT_LOG_VERBOSITY_CHANGED messages)
											*/
	eLogType traceLevel;					/* Filled in by ESIF */
	ActWriteLogFunction writeLogFuncPtr;	/* Filled in by ESIF */

											/*
											* Register for events
											*/
	ActEventRegistrationFunction eventRegistrationFuncPtr;	/* Filled in by ESIF */

															/*
															* Use to execute primitives using ESIF
															*/
	ActExecutePrimitiveFunction	execPrimitiveFuncPtr;	/* Filled in by ESIF */

} EsifActIfaceUpeV4, *EsifActIfaceUpeV4Ptr;


typedef union EsifUpeIface_u {
	EsifIfaceHdr hdr;
	EsifActIfaceUpeV4 actIfaceV4;
} EsifUpeIface, *EsifUpeIfacePtr;


/*
 * The following are for system simulation support
 */

typedef struct EsifSimTuple_s {
	u16  id;	/* Primitive ID - GET_TEMP etc. */
	u16  domain;	/* DO, NA, ...              */
	u16  instance;	/* ff: no instance required */
}EsifSimTuple, *EsifSimTuplePtr;

typedef struct EsifSimRequest_s {
	EsifSimTuple primitiveTuple;
	EsifDataPtr orgRequestPtr;
} EsifSimRequest, *EsifSimRequestPtr;

#pragma pack(pop)



#ifdef __cplusplus
extern "C" {
#endif

/*
 * Entry function this symbol must be available when the app.so/dll modules is
 * loaded.  For Unix/Linux nothing special is required for widnows you must
 * export this symbol see the REFERENCE IMPLEMENATION for the optional methods
 * for doing this.  Note this is the only function that is linked to. 
 */
ESIF_EXPORT eEsifError  GetActionInterface(EsifActIfacePtr actIfacePtr);

#ifdef __cplusplus
}
#endif



