/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ARBITRATION

#include "esif.h"
#if defined(ESIF_FEAT_OPT_ARBITRATOR_ENABLED)
#include "esif_uf.h"	/* Upper Framework */
#include "esif_pm.h"
#include "esif_uf_arbmgr.h"
#include "esif_link_list.h"
#include "esif_uf_primitive.h"	/* ESIF Primitive Execution */
#include "esif_participant.h"
#include "esif_lib_esifdata.h"
#include "esif_queue.h"
#include "esif_ccb_atomic.h"
#include "esif_uf_primitive_type.h"


/******************************************************************************
* General Definitions
******************************************************************************/

#define ESIF_TRACE_PRIMITIVE_DEBUG(msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(ESIF_TRACEMODULE_PRIMITIVE) | ESIF_TRACEMASK(ESIF_TRACEMODULE_ARBITRATION), \
			ESIF_TRACELEVEL_DEBUG, \
			msg, \
			##__VA_ARGS__ \
		)

/* Must only be used when "self" is known valid */
#define ESIF_TRACE_ARB_CTX(level, msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(ESIF_TRACEMODULE_ARBITRATION), \
			level, \
			"[%s] : " msg, \
			self->participantName ? self->participantName : "UNK", \
			##__VA_ARGS__ \
		)

/* Must only be used when "self" is in known valid */
#define ESIF_TRACE_ARB_ENTRY(level, msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(ESIF_TRACEMODULE_ARBITRATION), \
			level, \
			"[%s Prim = %u, Inst = %u] : " msg, \
			self->participantName ? self->participantName : "UNK", \
			self->primitiveId, self->instance, \
			##__VA_ARGS__ \
		)

#define ESIF_UF_ARBMGR_QUEUE_NAME "UfArbMgrPrimitiveQueue"
#define ESIF_UF_ARBMGR_QUEUE_SIZE 0xFFFFFFFF
#define ESIF_ARB_ENTRY_ITERATOR_MARKER 'UFAM'
#define ESIF_ARB_CTX_ENTRY_TABLE_GROWTH_RATE 10


/******************************************************************************
* Type Declarations
******************************************************************************/

/* Pre-defines */
struct EsifArbEntry_s;
struct EsifArbReq_s;

/*
 * Arbitration Function Prototype
 *
 *  Expected Return Values:
 *     1 if function determines Req 1 is higher priority
 *    -1 if function determines Req 2 is higher priority
 *     0 if function determines the requests are of equal priority
 *
 * Note: "Valid pointers" indicate if well-formed data for the given arbitration
 * type.
 */
typedef esif_error_t (*EsifArbMgr_ArbitratorFunc)(
	struct EsifArbReq_s *req1Ptr,
	Bool *isValid1Ptr,
	struct EsifArbReq_s *req2Ptr,
	Bool *isValid2Ptr,
	int *resultPtr
	);

/*
 * Arbitration Manager
 *
 * The arbitration manager has two primary responsibilities:
 * 1. Create/control access to the arbitration context singleton in the participant
 * 2. Provide a queue for arbitrated primitive requests for execution outside
 * of other arbitration locks.
 */
typedef struct EsifArbMgr_s {
	esif_ccb_lock_t mgrLock;
	atomic_t arbitrationEnabled;

	EsifQueue *primitiveQueuePtr; /* EsifArbPrimReq primitive requests */
	esif_thread_t primitiveQueueThread;
	Bool primitiveQueueExitFlag;
	esif_ccb_lock_t primitiveQueueLock;
} EsifArbMgr;

/*
 * Arbitration Context
 *
 * The arbitration context contains a collection of arbration entries, one
 * for each primitive/instance.
 *
 * An arbitration context is stored in each participant.  By doing this, the
 * arbitration manager does not have to track partcipant availability or track
 * active participants. (Participants call EsifArbMgr_DestroyArbitrationContext
 * when about to be destroyed to release the context.)
 */
typedef struct EsifArbCtx_s {
	esif_ccb_lock_t ctxLock;
	atomic_t arbitrationEnabled;
	size_t numEntries;
	size_t entryCapacity;
	struct EsifArbEntry_s **entriesPtr;

	esif_handle_t participantId; /* Containing participant */
	esif_string participantName; /* For tracing */
} EsifArbCtx;

/*
 * Arbitration Entry
 *
 * An arbitration entry contains association information (primitive ID and instance),
 * a collection of current requests, and the arbitration knobs.
 *
 */
typedef struct EsifArbEntry_s {
	UInt32 primitiveId; /* Association metadata*/
	UInt16 domain;
	UInt8 instance;
	esif_arbitration_type_t arbType; /* Arbitration function type */

	UInt32 upperLimit; /* Limiting information */
	UInt32 lowerLimit;

	esif_ccb_lock_t entryLock;

	atomic_t arbitrationEnabled;
	EsifLinkList arbRequests; /* EsifArbReq items*/

	esif_handle_t participantId; /* Containing participant; req for primitives */
	esif_string participantName; /* For tracing */

	/* life control */
	UInt32 refCount;
	UInt8 markedForDelete;
	esif_ccb_event_t deleteEvent;
} EsifArbEntry;


typedef struct EsifArbEntryParams_s{
	UInt32 primitiveId; /* Association metadata*/
	UInt16  domain;
	UInt8 instance;

	esif_arbitration_type_t arbType; /* Arbitration function type */

	UInt32 upperLimit; /* Limiting information */
	UInt32 lowerLimit;
} EsifArbEntryParams;


/*
 * Arbitration Request
 *
 * Arbitrated request from a given application.
 * (Data in the Arbitration Entries.)
 */

typedef struct EsifArbReq_s {
	esif_handle_t appHandle; /* Requestor identifier */
	EsifData *reqPtr; /* Request data */
} EsifArbReq;

/*
 * Primitive Request
 *
 * Asynchronous primitive request for execution by the arbitration manager.
 */
typedef struct EsifArbPrimReq_s {
	esif_handle_t participantId;
	esif_primitive_type_t primitiveId;
	UInt16 domain;
	UInt8 instance;
	EsifDataPtr reqDataPtr;
	EsifDataPtr rspDataPtr;
	esif_ccb_event_t *completionEventPtr;
	esif_error_t *completionStatusPtr;
	Bool reqDataRequiresRelease;
	Bool requiresDelay;
	esif_primitive_type_t delayedPrimitiveId;
	Bool isDummyRequest;
} EsifArbPrimReq;

/*
 * Arbitration Entry Interator
 * See EsifArbCtx_InitEntryIterator for usage.
 */
typedef struct EsifArbEntryIterator_s {
	UInt32 marker;
	size_t index;
	Bool valid;
	EsifArbEntry *entryPtr;
} EsifArbEntryIterator;


/******************************************************************************
* Function Prototypes
******************************************************************************/

/* EsifArbMgr Functions*/

/* Boilerplate lifecycle items */
esif_error_t EsifArbMgr_Init(void);
esif_error_t EsifArbMgr_Start(void);
void EsifArbMgr_Stop(void);
void EsifArbMgr_Exit(void);

/*
 * Arbitrated primitive execution interface
 * (Called by the ESIF Application Services Interface)
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

static esif_error_t EsifArbMgr_ExecuteUnarbitratedPrimitive(
	EsifUp *upPtr,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

/* The primitive queue lock is expected to be held when called */
static Bool EsifArbMgr_MustDelayPrimitive_Locked(
	esif_primitive_type_t primitiveId
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

/* Used to change the arbitration function */
esif_error_t EsifArbMgr_SetArbitrationFunction(
	const esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const esif_arbitration_type_t arbType
	);

/* FRIEND - Called by participant while being destroyed */
void EsifArbMgr_DestroyArbitrationContext(void *ctxPtr);

/* FRIEND - Called by AppMgr when an ESIF app is destroyed */
void EsifArbMgr_RemoveApp(esif_handle_t appHandle);

/* EsifArbMgr Private Functions*/

/* Returns/Creates the singleton context object for participant */
static EsifArbCtx *EsifArbMgr_CtxInst(EsifUp *upPtr);

/* Purges request for all participants */
/* Caller is expected to hold the mgrLock */
static void EsifArbMgr_PurgeRequests_Locked();

/* Queues primitive request for asynchronous execution outside locks */
/* The primitive queue lock is expected to be held when called */
static esif_error_t EsifArbMgr_QueuePrimitiveRequest_Locked(
	const esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr,
	esif_ccb_event_t *completionEventPtr,
	esif_error_t *completionStatusPtr,
	Bool requiresDelay /* Indicates if incoming requests must be delayed until completion of this request */
	);

static void *ESIF_CALLCONV EsifArbMgr_PrimitiveQueueExecutionThread(void *ctxPtr);


/* EsifArbCtx Functions*/

static EsifArbCtx *EsifArbCtx_Create(EsifUpPtr upPtr);
static void EsifArbCtx_Destroy(EsifArbCtx *self);

/* See EsifArbCtx_InitEntryIterator for usage */
static esif_error_t EsifArbCtx_InitEntryIterator(EsifArbEntryIterator *iterPtr);

static esif_error_t EsifArbCtx_GetNextEntry(
	EsifArbCtx *self,
	EsifArbEntryIterator *iterPtr,
	EsifArbEntry **entryPtr
	);

static esif_error_t EsifArbCtx_ExecutePrimitive(
	EsifArbCtx *self,
	const esif_handle_t appHandle,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const EsifDataPtr requestPtr
	);

/*
 * Gets arbitration information for a participant
 *
 * WARNING!!! Caller is expected to free returned pointer if not NULL
 *
 * Else if primitiveId is 0; gets all entries
 * Else gets information for a specific entry
 *
 */
EsifArbCtxInfo *EsifArbCtx_GetInformation(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
);

static esif_error_t EsifArbCtx_SetArbitrationState(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const Bool isEnabled
	);

static esif_error_t EsifArbCtx_StopArbitration(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
	);

/*
* Sets the limits for a given participant/primitive/instance
* Note:  Creates the entry if not present
*/
static esif_error_t EsifArbCtx_SetLimits(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	UInt32 *upperLimitPtr,
	UInt32 *lowerLimitPtr
	);

/*
* Sets the arbitration funciton for a given participant/primitive/instance
* Note:  Creates the entry if not present
*/
static esif_error_t EsifArbCtx_SetArbitrationFunction(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const esif_arbitration_type_t arbType
	);

static void EsifArbCtx_PurgeRequests(
	EsifArbCtx *self
);

static void EsifArbCtx_RemoveApp(
	EsifArbCtx *self,
	esif_handle_t appHandle
);

/* EsifArbCxt Private Functions */

/* Callers must call EsifArbEntry_PutRef on returned pointer */
static EsifArbEntry *EsifArbCtx_LookupArbEntry_Locked(
	/* Caller is expected to hold the ctxLock */
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
	);

/* Callers must call EsifArbEntry_PutRef on the returned pointer after use */
static esif_error_t EsifArbCtx_CreateAndInsertEntry_Locked(
	/* Caller is expected to hold the ctxLock */
	EsifArbCtx *self,
	EsifArbEntryParams *paramPtr,
	const esif_handle_t participantId,
	esif_string participantName,
	EsifArbEntry **entryPtr
	);

static esif_error_t EsifArbCtx_InsertArbEntry_Locked(
	/* Caller is expected to hold the ctxLock */
	EsifArbCtx *self,
	EsifArbEntry *entryPtr
	);

static esif_error_t EsifArbCtx_RemoveArbEntry_Locked(
	/* Caller is expected to hold the ctxLock */
	EsifArbCtx *self,
	EsifArbEntry *entryPtr
	);

static void EsifArbCtx_PurgeRequests_Locked(
	/* Caller is expected to hold the ctxLock */
	EsifArbCtx *self
	);

static esif_error_t EsifArbCtx_PopulateDefaultArbitrationTable(
	EsifArbCtx *ctxPtr,
	EsifArbEntryParams *entryParamsPtr
	);


/* EsifArbEntry Functions*/

static EsifArbEntry *EsifArbEntry_Create(
	EsifArbEntryParams *entryParamsPtr,
	const esif_handle_t participantId,
	esif_string participantName
	);

static void EsifArbEntry_Destroy(EsifArbEntry *self);

static esif_error_t EsifArbEntry_GetRef(EsifArbEntry *self);
static void EsifArbEntry_PutRef(EsifArbEntry *self);

static esif_error_t EsifArbEntry_GetInformation(
	EsifArbEntry *self,
	EsifArbEntryInfo *infoPtr
	);

static esif_error_t EsifArbEntry_SetArbitrationState(
	EsifArbEntry *self,
	const Bool isEnabled
	);

static esif_error_t EsifArbEntry_SetLimits(
	EsifArbEntry *self,
	UInt32 *upperLimitPtr,
	UInt32 *lowerLimitPtr
	);

static esif_error_t EsifArbEntry_SetArbitrationFunction(
	EsifArbEntry *self,
	esif_arbitration_type_t arbType
	);

static esif_error_t EsifArbEntry_ExecutePrimitive(
	EsifArbEntry *self,
	const esif_handle_t appHandle,
	EsifData *requestPtr
	);

static void EsifArbEntry_RemoveApp(
	EsifArbEntry *self,
	esif_handle_t appHandle
	);

static Bool EsifArbEntry_IsMatchingEntry(
	EsifArbEntry *self,
	UInt32 primitiveId,
	UInt16 domain,
	UInt8 instance
	);

static void EsifArbEntry_PurgeRequests(
	EsifArbEntry *self
	);

/* EsifArbEntry Private Functions*/

/*
 * Returns the current highest-priority data
 * (EntryLock is expected to be held when called)
 */
static	EsifData *EsifArbEntry_GetArbitratedRequestData_Locked(EsifArbEntry *self);

static esif_error_t EsifArbEntry_InsertRequest_Locked(
	/* EntryLock is expected to be held when called */
	EsifArbEntry *self,
	const esif_handle_t appHandle,
	const EsifData *requestPtr
	);

static esif_error_t EsifArbEntry_ArbitrateRequest_Locked(
	/* EntryLock is expected to be held when called */
	EsifArbEntry *self,
	EsifArbReq *arbReqPtr
	);

static esif_error_t EsifArbEntry_RearbitrateRequests_Locked(
	/* EntryLock is expected to be held when called */
	EsifArbEntry *self
	);

/* Limits data and then queues primitive request for asynchronous execution outside locks */
/* The entryLock is expect to be held when called */
static esif_error_t EsifArbEntry_QueueLimitedPrimitiveRequest_Locked(
	EsifArbEntry *self,
	const EsifDataPtr requestPtr,
	Bool requiresDelay
	);

/*
 * Returns a new EsifData pointer based on the original input with the value
 * limited if limiting is required.  Return NULL if orginal input is does not
 * need to be limited.
 * (EntryLock is expected to be held when called)
 *
 * WARNING!!! It is the responsibility of the caller to call EsifData_Destroy on
 * the returned data pointer.
 */
static EsifData *EsifArbEntry_CreateLimitedData_Locked(
	EsifArbEntry *self,
	EsifData *dataPtr
	);

static void EsifArbEntry_PurgeRequests_Locked(
	/* EntryLock is expected to be held when called */
	EsifArbEntry *self
);

/* EsifArbReq Functions*/

static EsifArbReq *EsifArbReq_Create(
	esif_handle_t appHandle,
	const EsifData *reqPtr
	);

static void EsifArbReq_Destroy(EsifArbReq *self);

static esif_handle_t EsifArbReq_GetAppHandle(EsifArbReq *self);
static EsifData *EsifArbReq_GetRequestData(EsifArbReq *self);


/* EsifArbPrimReq */

static EsifArbPrimReq *EsifArbPrimReq_Create(
	const esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr,
	esif_ccb_event_t *completionEventPtr,
	esif_error_t *completionStatusPtr,
	Bool requiresDelay
	);

static void EsifArbPrimReq_Destroy(EsifArbPrimReq *self);

/* Arbitration Functions*/
static EsifArbMgr_ArbitratorFunc GetArbitrationFunction(
	esif_arbitration_type_t funcType
	);

static Bool IsPrimitiveSupported(
	esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
	);

void GenericLLDestroyCallback(void *ptr);

static esif_error_t EsifArbFunction_UInt32_GreaterThan(
	EsifArbReq *req1Ptr,
	Bool *isValid1Ptr,
	EsifArbReq *req2Ptr,
	Bool *isValid2Ptr,
	int *resultPtr
	);

static esif_error_t EsifArbFunction_UInt32_LessThan(
	EsifArbReq *req1Ptr,
	Bool *isValid1Ptr,
	EsifArbReq *req2Ptr,
	Bool *isValid2Ptr,
	int *resultPtr
	);


/******************************************************************************
* Global Objects
******************************************************************************/

static EsifArbMgr g_arbMgr = { 0 };

/*
 * Arbitration Functions
 *
 * WARNING:  The order of functions in this table must match the
 * esif_arbitration_type_t declaration order.
 */
static EsifArbMgr_ArbitratorFunc g_arbitrationFunctions[] = {
	NULL, /* 0 is not considered an initialized index */
	EsifArbFunction_UInt32_GreaterThan, /*ESIF_ARBITRATION_UIN32_GREATER_THAN */
	EsifArbFunction_UInt32_LessThan,  /* ESIF_ARBITRATION_UIN32_LESS_THAN */
};

/*
 * NOTE: Whenever a Primitive is Added to or Removed from these Tables, it must also
 * be updated in the DSP database so that esif_uf_primitive_type.h can be regenerated.
 */
static EsifArbEntryParams g_cpuArbTable[] = {
	{SET_PERF_PREFERENCE_MAX, ESIF_PRIMITIVE_DOMAIN_D0, 255, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX_PERCENT, ESIF_ARB_LIMIT_MIN_PERCENT},
	{SET_PERF_PREFERENCE_MIN, ESIF_PRIMITIVE_DOMAIN_D0, 255, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX_PERCENT, ESIF_ARB_LIMIT_MIN_PERCENT},
	{SET_PLATFORM_POWER_LIMIT, ESIF_PRIMITIVE_DOMAIN_D0, 0, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_PLATFORM_POWER_LIMIT, ESIF_PRIMITIVE_DOMAIN_D0, 1, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_PLATFORM_POWER_LIMIT_ENABLE, ESIF_PRIMITIVE_DOMAIN_D0, 0, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_PLATFORM_POWER_LIMIT_ENABLE, ESIF_PRIMITIVE_DOMAIN_D0, 1, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_PLATFORM_POWER_LIMIT_TIME_WINDOW, ESIF_PRIMITIVE_DOMAIN_D0, 0, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_PROC_NUMBER_OFFLINE_CORES, ESIF_PRIMITIVE_DOMAIN_D1, 255, ESIF_ARBITRATION_UIN32_GREATER_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT, ESIF_PRIMITIVE_DOMAIN_D0, 0, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT, ESIF_PRIMITIVE_DOMAIN_D0, 1, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT, ESIF_PRIMITIVE_DOMAIN_D0, 3, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT_DUTY_CYCLE, ESIF_PRIMITIVE_DOMAIN_D0, 2, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT_ENABLE, ESIF_PRIMITIVE_DOMAIN_D0, 0, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT_ENABLE, ESIF_PRIMITIVE_DOMAIN_D0, 1, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT_TIME_WINDOW, ESIF_PRIMITIVE_DOMAIN_D0, 0, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT_TIME_WINDOW, ESIF_PRIMITIVE_DOMAIN_D0, 1, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_TCC_OFFSET, ESIF_PRIMITIVE_DOMAIN_D0, 255, ESIF_ARBITRATION_UIN32_GREATER_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{0} /* Mark end of table */
};

static EsifArbEntryParams g_wifiArbTable[] = {
	{SET_RAPL_POWER_LIMIT, ESIF_PRIMITIVE_DOMAIN_D0, 0, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT, ESIF_PRIMITIVE_DOMAIN_D0, 1, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT, ESIF_PRIMITIVE_DOMAIN_D0, 3, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT_ENABLE, ESIF_PRIMITIVE_DOMAIN_D0, 0, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{SET_RAPL_POWER_LIMIT_ENABLE, ESIF_PRIMITIVE_DOMAIN_D0, 1, ESIF_ARBITRATION_UIN32_LESS_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{0} /* Mark end of table */
};

static EsifArbEntryParams g_ietmArbTable[] = {
	{0} /* Mark end of table */
};

static EsifArbEntryParams g_fanArbTable[] = {
	{SET_FAN_LEVEL, ESIF_PRIMITIVE_DOMAIN_D0, 255, ESIF_ARBITRATION_UIN32_GREATER_THAN, ESIF_ARB_LIMIT_MAX, ESIF_ARB_LIMIT_MIN},
	{0} /* Mark end of table */
};


/******************************************************************************
* Function Definitions
******************************************************************************/

/******************************************************************************
*******************************************************************************
*
* EsifArbMgr Functions
*
*******************************************************************************
******************************************************************************/

/*
 * Arbitration Primitive Execution Filter
 *
 * Arbitrates requests based on the primitive ID and instance
 * NOTE(s):
 * 1. Only arbitrates D0
 * 2. Only arbitrates primitives specified in tables
 * 3. Tables kept as context in each participant
 * 4. Primitive is executed if arbitration code fails
 */
esif_error_t EsifArbMgr_ExecutePrimitive(
	const esif_handle_t appHandle,
	const esif_handle_t participantId,
	const UInt32 primitiveId,
	const EsifString domainStr,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifData *responsePtr
	)
{
	esif_error_t rc = ESIF_OK;
	EsifUp  *upPtr = NULL;
	UInt16 domain = 0;
	EsifArbCtx *arbCtxPtr = NULL;

	if (NULL == domainStr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	domain = domain_str_to_short(domainStr);
	
	ESIF_TRACE_PRIMITIVE_DEBUG("\n\n"
		"Primitive Request:\n"
		"  Application ID       : " ESIF_HANDLE_FMT "\n"
		"  Participant ID       : " ESIF_HANDLE_FMT "\n"
		"  Primitive            : %s(%u)\n"
		"  Domain               : %s\n"
		"  Instance             : %u\n"
		"  Request              : %p\n"
		"  Response             : %p\n",
		esif_ccb_handle2llu(appHandle),
		esif_ccb_handle2llu(participantId),
		esif_primitive_str((enum esif_primitive_type)primitiveId), primitiveId,
		domainStr,
		instance,
		requestPtr,
		responsePtr);

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (NULL == upPtr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}
	/*
	 * Only accesses to Domain 0 are arbitrated
	 */
	rc = ESIF_E_NOT_SUPPORTED;
	if (atomic_read(&g_arbMgr.arbitrationEnabled)) {
		/*
		 * Get/create the arbitration context stored in the participant.
		 * (While a reference is held on the participant, the arbitration context
		 * is valid.)
		 */
		arbCtxPtr = EsifArbMgr_CtxInst(upPtr);
		rc = EsifArbCtx_ExecutePrimitive(arbCtxPtr, appHandle, primitiveId, domain, instance, requestPtr);
	}

	/*
	 * If arbitration fails, execute the primitive unarbitrated.
	 */
	if (rc != ESIF_OK) {
		ESIF_TRACE_PRIMITIVE_DEBUG("Executing unarbitrated primitive.");
		rc = EsifArbMgr_ExecuteUnarbitratedPrimitive(
			upPtr,
			primitiveId, domain, instance,
			requestPtr, responsePtr);
	}
exit:
	ESIF_TRACE_PRIMITIVE_DEBUG("Primitive result = %s", esif_rc_str(rc));

	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


static esif_error_t EsifArbMgr_ExecuteUnarbitratedPrimitive(
	EsifUp *upPtr,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	esif_error_t rc = ESIF_OK;
	esif_error_t completionStatus = ESIF_E_UNSPECIFIED;
	Bool primitiveCompleted = ESIF_FALSE;
	EsifPrimitiveTuple tuple = { 0 };
	esif_ccb_event_t completionEvent = { 0 };

	tuple.id = (u16)primitiveId;
	tuple.domain = domain;
	tuple.instance = instance;

	if (atomic_read(&g_arbMgr.arbitrationEnabled)) {

		esif_ccb_write_lock(&g_arbMgr.primitiveQueueLock);

		if (EsifArbMgr_MustDelayPrimitive_Locked(primitiveId)) {
			esif_ccb_event_init(&completionEvent);
			esif_ccb_event_reset(&completionEvent);

			rc = EsifArbMgr_QueuePrimitiveRequest_Locked(
				EsifUp_GetInstance(upPtr),
				primitiveId, domain, instance,
				requestPtr,
				responsePtr,
				&completionEvent,
				&completionStatus,
				ESIF_FALSE);

			/* Must release lock before waiting */
			esif_ccb_write_unlock(&g_arbMgr.primitiveQueueLock);

			if (ESIF_OK == rc) {
				/* Wait for primitive execution if queueing was successful */
				esif_ccb_event_wait(&completionEvent);
				rc = completionStatus; /* Set by pointer during primitive execution */
				primitiveCompleted = ESIF_TRUE;
			}
			esif_ccb_event_uninit(&completionEvent);
		}
		else {
			esif_ccb_write_unlock(&g_arbMgr.primitiveQueueLock);
		}
	}

	/*
	* If the primitive did not execute; we will execute without queue.
	*/
	if (!primitiveCompleted) {
		rc = EsifUp_ExecutePrimitive(upPtr, &tuple, requestPtr, responsePtr);
	}

	return rc;
}

/* The primitive queue lock is expected to be held when called */
static Bool EsifArbMgr_MustDelayPrimitive_Locked(
	esif_primitive_type_t primitiveId
)
{
	Bool requiresDelay = ESIF_FALSE;
	EsifLinkListNode *curNodePtr = NULL;
	EsifArbPrimReq *curReqPtr = NULL;

	/*
	* Search the arbitrated request queue for any primitives that would require
	* the current primitive to be delayed
	*/
	if (g_arbMgr.primitiveQueuePtr && g_arbMgr.primitiveQueuePtr->queue_list_ptr) {

		curNodePtr = g_arbMgr.primitiveQueuePtr->queue_list_ptr->head_ptr;
		while (curNodePtr) {
			curReqPtr = (EsifArbPrimReq *)curNodePtr->data_ptr;

			if (curReqPtr) {
				if (curReqPtr->requiresDelay && (primitiveId == curReqPtr->delayedPrimitiveId)) {
					requiresDelay = ESIF_TRUE;
					break;
				}
			}
			curNodePtr = curNodePtr->next_ptr;
		}
	}
	return requiresDelay;
}


/*
 * Gets the arbitration context from the participant
 * If not present, creates an arbitration context and places it in the
 * participant. (Acts much as a singleton and exists during the lifetime of the
 * participant.)
 */
static EsifArbCtx *EsifArbMgr_CtxInst(
	EsifUp *upPtr
	)
{
	EsifArbCtx *arbCtxPtr = NULL;
	EsifArbCtx *arbCtxVerifyPtr = NULL;

	arbCtxPtr = (EsifArbCtx *)EsifUp_GetArbitrationContext(upPtr);
	if (NULL == arbCtxPtr) {
		ESIF_TRACE_INFO("[%s] : No arbitration context present\n", EsifUp_GetName(upPtr));

		/* Arbitration context create outside locks */

		/*
		 * Context creation is attempted once only (assuming allocation
		 * succeeds), so that we will not end up a loop re-trying
		 * creation for every primtive.
		 * The context creation should always return a non-NULL pointer,
		 * even if the internal context is empty.
		*/
		arbCtxPtr = EsifArbCtx_Create(upPtr);

		esif_ccb_write_lock(&g_arbMgr.mgrLock);

		/* Check again inside lock; if still NULL, set the context */
		arbCtxVerifyPtr = (EsifArbCtx *)EsifUp_GetArbitrationContext(upPtr);
		if (NULL == arbCtxVerifyPtr) {
			ESIF_TRACE_INFO("[%s] : Created participant arbitration context = %p\n", EsifUp_GetName(upPtr), arbCtxPtr);
			EsifUp_SetArbitrationContext(upPtr, arbCtxPtr);
		}
		else {
			/* If there was a race condition from creation to insertion */
			EsifArbCtx_Destroy(arbCtxPtr);
		}

		esif_ccb_write_unlock(&g_arbMgr.mgrLock);
	}
	return EsifUp_GetArbitrationContext(upPtr);
}


/* The primitive queue lock is expected to be held when called */
static esif_error_t EsifArbMgr_QueuePrimitiveRequest_Locked(
	const esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr,
	esif_ccb_event_t *completionEventPtr,
	esif_error_t *completionStatusPtr,
	Bool requiresDelay /* Indicates if incoming requests must be delayed until completion of this request */
	)
{
	esif_error_t rc = ESIF_E_NO_MEMORY;
	EsifArbPrimReq *primReqPtr = NULL;
	EsifArbPrimReq *copyPrimReqPtr = NULL;

	primReqPtr = EsifArbPrimReq_Create(
		participantId,
		primitiveId,
		domain,
		instance,
		requestPtr,
		responsePtr,
		completionEventPtr,
		completionStatusPtr,
		requiresDelay
	);
	/* Queue will accept NULL requests so, need to check before inserting */
	if (primReqPtr) {
		rc = esif_queue_enqueue(g_arbMgr.primitiveQueuePtr, primReqPtr);

		/*
		* We place a copy of the request in the queue so that primitives requiring
		* delay can detect the case when the request has been removed from the queue,
		* but has not been processed yet.
		*/
		if (primReqPtr->requiresDelay && (primReqPtr->delayedPrimitiveId != (esif_primitive_type_t)0)) {
			copyPrimReqPtr = esif_ccb_malloc(sizeof(*copyPrimReqPtr));
			if (copyPrimReqPtr) {
				esif_ccb_memcpy(copyPrimReqPtr, primReqPtr, sizeof(*copyPrimReqPtr));
				copyPrimReqPtr->isDummyRequest = ESIF_TRUE;
				esif_queue_enqueue(g_arbMgr.primitiveQueuePtr, copyPrimReqPtr);
			}
		}
	}
	ESIF_TRACE_DEBUG("[Prim = %u, Inst = %u, Part = " ESIF_HANDLE_FMT "] : Queued primitive request; rc = %d",
		primitiveId, instance, esif_ccb_handle2llu(participantId), rc);

	return rc;
}


static void *ESIF_CALLCONV EsifArbMgr_PrimitiveQueueExecutionThread(
	void *ctxPtr
	)
{
	esif_error_t rc = ESIF_OK;
	EsifArbPrimReq *primReqPtr = NULL;
	EsifArbPrimReq *dummyReqPtr = NULL;
	EsifUp *upPtr = NULL;
	EsifPrimitiveTuple tuple = { 0 };
	UInt32 phonyData = 0;
	EsifData phonyResponseData = { ESIF_DATA_VOID };
	EsifData *respDataPtr = &phonyResponseData;

	phonyResponseData.buf_ptr = &phonyData;

	UNREFERENCED_PARAMETER(ctxPtr);

	while (!g_arbMgr.primitiveQueueExitFlag) {
		primReqPtr = esif_queue_pull(g_arbMgr.primitiveQueuePtr);

		if (NULL == primReqPtr) {
			continue;
		}

		/* Do not process dummy requests used to determine primitive delays */
		if (primReqPtr->isDummyRequest) {
			esif_ccb_free(primReqPtr); /* Just free, do not destroy; data is a copy */
			primReqPtr = NULL;
			continue;
		}

		tuple.id = (UInt16)primReqPtr->primitiveId;
		tuple.domain = primReqPtr->domain;
		tuple.instance = primReqPtr->instance;

		respDataPtr = &phonyResponseData;
		if (primReqPtr->rspDataPtr != NULL) {
			respDataPtr = primReqPtr->rspDataPtr;
		}

		upPtr = EsifUpPm_GetAvailableParticipantByInstance(primReqPtr->participantId);
		rc = EsifUp_ExecutePrimitive(upPtr, &tuple, primReqPtr->reqDataPtr, respDataPtr);

		if (rc != ESIF_OK) {
			ESIF_TRACE_DEBUG("[%s Prim = %lu, Inst = %lu] : Executed queued primitive request, rc = %d",
				EsifUp_GetName(upPtr),
				tuple.id, tuple.instance,
				rc);
		}

		if (primReqPtr->completionStatusPtr) {
			*primReqPtr->completionStatusPtr = rc;
		}

		/*
		* Remove the dummy request used to determine if associated primitive
		* should be delayed (used to prevent race condition when primitive is
		* removed from queue, but not yet processed)
		*/
		esif_ccb_write_lock(&g_arbMgr.primitiveQueueLock);

		dummyReqPtr = esif_queue_dequeue(g_arbMgr.primitiveQueuePtr);
		if (dummyReqPtr && dummyReqPtr->isDummyRequest && (primReqPtr->primitiveId == dummyReqPtr->primitiveId)) {
			esif_ccb_free(dummyReqPtr); /* Just free, do not destroy; data is a copy */
		}
		else {
			esif_queue_requeue(g_arbMgr.primitiveQueuePtr, dummyReqPtr);
			esif_queue_signal_event(g_arbMgr.primitiveQueuePtr);
		}
		esif_ccb_write_unlock(&g_arbMgr.primitiveQueueLock);

		EsifUp_PutRef(upPtr);
		EsifArbPrimReq_Destroy(primReqPtr); /* Release waiting thread */
		primReqPtr = NULL;
		dummyReqPtr = NULL;
	}
	return 0;
}


/*
 * Friend function to remove arbitration for the specified application
 */
void EsifArbMgr_RemoveApp(
	esif_handle_t appHandle
	)
{
	esif_error_t iterRc = ESIF_E_UNSPECIFIED;
	UfPmIterator upIter = { 0 };
	EsifUp *upPtr = NULL;
	EsifArbCtx *arbCtxPtr = NULL;

	/*
	 * Iterate through all participants and remove any entries for the specified app.
	 */
	EsifUpPm_InitIterator(&upIter);
	iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	while (ESIF_OK == iterRc) {

		arbCtxPtr = (EsifArbCtx *)EsifUp_GetArbitrationContext(upPtr);
		EsifArbCtx_RemoveApp(arbCtxPtr, appHandle);

		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}

	/* We should always complete iteration here */
	if (iterRc != ESIF_E_ITERATION_DONE) {
		ESIF_ASSERT(0);
		ESIF_TRACE_ERROR("[%s] : Iteration error : upPtr = %p, iterRc = %d\n", EsifUp_GetName(upPtr), upPtr, iterRc);
	}
	EsifUp_PutRef(upPtr);
}

/*
 * Exernal friend wrapper function to allow the context to be destroyed when a
 * participant is destroyed.
 */
void EsifArbMgr_DestroyArbitrationContext(
	void *ctxPtr
	)
{
	EsifArbCtx_Destroy((EsifArbCtx *)ctxPtr);
}

/* Gets arbitration information */
/* Caller is responsible for freeing returned pointer */
EsifArbInfo *EsifArbMgr_GetInformation(
	const esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
	)
{
	esif_error_t rc = ESIF_E_NO_MEMORY;
	esif_error_t iterRc = ESIF_E_UNSPECIFIED;
	EsifUp *upPtr = NULL;
	EsifArbInfo *infoPtr = NULL;
	EsifArbCtx *ctxPtr = NULL;
	EsifArbCtxInfo *tempCtxInfoPtr = NULL;
	EsifArbCtxInfo *curCtxInfoPtr = NULL;
	UfPmIterator upIter = { 0 };
	EsifLinkList *ctxInfoListPtr = NULL;
	EsifLinkListNode *curNodePtr = NULL;
	size_t numParts = 0;
	size_t numArbitratedParts = 0;
	size_t reqSize = sizeof(*infoPtr); /* Add to this as iteration proceeds */

	ctxInfoListPtr = esif_link_list_create();
	if (ctxInfoListPtr) {

		EsifUpPm_InitIterator(&upIter);
		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		while (ESIF_OK == iterRc) {

			if ((participantId == ESIF_INVALID_HANDLE) || /* If all participants */
				(participantId == EsifUp_GetInstance(upPtr))) { /* If matching participant */

				ctxPtr = (EsifArbCtx *)EsifUp_GetArbitrationContext(upPtr);
				tempCtxInfoPtr = EsifArbCtx_GetInformation(ctxPtr, primitiveId, domain, instance);

				/* Only if the participant has been populated */
				if (tempCtxInfoPtr) {
					numArbitratedParts++;
					tempCtxInfoPtr->isArbitrated = ESIF_TRUE;
					/* Update the required size for each participant */

				}
				/* If not arbitrated, provide placeholder for each participant data */
				else {
					/* Just provide minimal information with everything disabled */
					tempCtxInfoPtr = (EsifArbCtxInfo *)esif_ccb_malloc(sizeof(*tempCtxInfoPtr));
					if (NULL == tempCtxInfoPtr) {
						ESIF_TRACE_ERROR("Allocation failure\n");
					}
					else {
						tempCtxInfoPtr->size = sizeof(*tempCtxInfoPtr);
						tempCtxInfoPtr->participantId = EsifUp_GetInstance(upPtr);
					}
				}
				if (tempCtxInfoPtr) {
					reqSize += tempCtxInfoPtr->size;
					esif_link_list_add_at_back(ctxInfoListPtr, (void *)tempCtxInfoPtr);
				}
			}
			iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		}
		if (iterRc != ESIF_E_ITERATION_DONE) {
			if (iterRc != ESIF_OK) {
				ESIF_ASSERT(0);
				ESIF_TRACE_ERROR("[%s] : Iteration error : upPtr = %p, iterRc = %d\n", EsifUp_GetName(upPtr), upPtr, iterRc);
				rc = iterRc;
			}
		}
		else {
			rc = ESIF_OK;
		}
	}

	if (ESIF_OK == rc) {
		rc = ESIF_E_NO_MEMORY;

		infoPtr = (EsifArbInfo *)esif_ccb_malloc(reqSize);
		if (NULL == infoPtr) {
			ESIF_TRACE_ERROR("Allocation error");
		}
		else {
			rc = ESIF_OK;

			/* Set global information */
			infoPtr->size = reqSize;
			infoPtr->arbitrationEnabled = (Bool)atomic_read(&g_arbMgr.arbitrationEnabled);
			numParts = esif_link_list_get_node_count(ctxInfoListPtr);
			infoPtr->count = numParts;
			infoPtr->arbitratedCount = numArbitratedParts;

			ESIF_TRACE_DEBUG("Returning arbitration information: Global state = %s, Count = %lu, Arbitrated Count = %lu, Arbitrated = %lu, Size = %lu\n",
				infoPtr->arbitrationEnabled ? "Enabled" : "Disabled",
				infoPtr->count, infoPtr->arbitratedCount,
				infoPtr->arbitrationEnabled,
				infoPtr->size);

			/* Set participant information */
			curCtxInfoPtr = infoPtr->arbCtxInfo;

			curNodePtr = ctxInfoListPtr->head_ptr;
			while (curNodePtr) {
				if (curNodePtr->data_ptr) {
					tempCtxInfoPtr = (EsifArbCtxInfo *)curNodePtr->data_ptr;
					esif_ccb_memcpy(curCtxInfoPtr, tempCtxInfoPtr, tempCtxInfoPtr->size);
				}
				curCtxInfoPtr = (EsifArbCtxInfo *)((char*)curCtxInfoPtr + curCtxInfoPtr->size);
				curNodePtr = curNodePtr->next_ptr;
			}
		}
	}

	esif_link_list_free_data_and_destroy(ctxInfoListPtr, GenericLLDestroyCallback);
	EsifUp_PutRef(upPtr);
	return infoPtr;
}


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
	)
{
	esif_error_t rc = ESIF_OK;
	EsifArbMgr *self = &g_arbMgr;
	EsifUp *upPtr = NULL;
	EsifArbCtx *arbCtxPtr = NULL;

	/* Disable arbitration on all participants */
	if (ESIF_INVALID_HANDLE == participantId) {
		esif_ccb_write_lock(&self->mgrLock);
		atomic_set(&self->arbitrationEnabled, isEnabled);
		if (!isEnabled) {
			EsifArbMgr_PurgeRequests_Locked();
		}
		ESIF_TRACE_INFO("%s arbitration for all participants\n", isEnabled ? "Enabled" : "Disabled");
		esif_ccb_write_unlock(&self->mgrLock);
	}
	/* Disable arbitration on specific participant */
	else {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;

		upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
		if (upPtr) {
			rc = ESIF_E_NO_MEMORY;

			arbCtxPtr = EsifArbMgr_CtxInst(upPtr);
			if (arbCtxPtr) {

				/* Setting at both levels is atomic */
				esif_ccb_write_lock(&self->mgrLock);

				rc = EsifArbCtx_SetArbitrationState(arbCtxPtr, primitiveId, domain, instance, isEnabled);

				/*
				 * When enabling, we will also enable at the higher levels so that the target
				 * level is enabled so it is fully arbitrated after the call
				 */
				if ((ESIF_OK == rc) && isEnabled) {
					atomic_set(&self->arbitrationEnabled, ESIF_TRUE);
					ESIF_TRACE_INFO("Enabled arbitration for all participants\n");
				}
				esif_ccb_write_unlock(&self->mgrLock);
			}
		}
	}
	EsifUp_PutRef(upPtr);
	return rc;
}


static void EsifArbMgr_PurgeRequests_Locked()
{
	esif_error_t iterRc = ESIF_OK;
	UfPmIterator upIter = { 0 };
	EsifUp *upPtr = NULL;
	EsifArbCtx *arbCtxPtr = NULL;

	/*
	 * Iterate through all participants and purge all requests
	 */
	EsifUpPm_InitIterator(&upIter);
	iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	while (ESIF_OK == iterRc) {

		arbCtxPtr = (EsifArbCtx *)EsifUp_GetArbitrationContext(upPtr);
		EsifArbCtx_PurgeRequests(arbCtxPtr);

		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}

	/* We should always complete iteration here */
	if (iterRc != ESIF_E_ITERATION_DONE) {
		ESIF_ASSERT(0);
		ESIF_TRACE_ERROR("[%s] : Iteration error : upPtr = %p, iterRc = %d\n", EsifUp_GetName(upPtr), upPtr, iterRc);
	}
	EsifUp_PutRef(upPtr);
}


/* Used to release the arbitration information for a given prim/inst */
esif_error_t EsifArbMgr_StopArbitration(
	const esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifUp *upPtr = NULL;
	EsifArbCtx *arbCtxPtr = NULL;

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (upPtr) {
		rc = ESIF_E_NO_MEMORY;
		arbCtxPtr = EsifArbMgr_CtxInst(upPtr);
		if (arbCtxPtr) {
			rc = EsifArbCtx_StopArbitration(arbCtxPtr, primitiveId, domain, instance);
		}
	}
	EsifUp_PutRef(upPtr);
	return rc;
}


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
	)
{
	esif_error_t rc = ESIF_E_PARTICIPANT_NOT_FOUND;
	EsifUp *upPtr = NULL;
	EsifArbCtx *arbCtxPtr = NULL;

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (upPtr) {
		rc = ESIF_E_NO_MEMORY;
		arbCtxPtr = EsifArbMgr_CtxInst(upPtr);
		if (arbCtxPtr) {
			rc = EsifArbCtx_SetLimits(arbCtxPtr, primitiveId, domain, instance, upperLimitPtr, lowerLimitPtr);
		}
	}

	EsifUp_PutRef(upPtr);
	return rc;
}


esif_error_t EsifArbMgr_SetArbitrationFunction(
	const esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const esif_arbitration_type_t arbType
	)
{
	esif_error_t rc = ESIF_E_PARTICIPANT_NOT_FOUND;
	EsifUp *upPtr = NULL;
	EsifArbCtx *arbCtxPtr = NULL;

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (upPtr) {
		rc = ESIF_E_NO_MEMORY;
		arbCtxPtr = EsifArbMgr_CtxInst(upPtr);
		if (arbCtxPtr) {
			rc = EsifArbCtx_SetArbitrationFunction(arbCtxPtr, primitiveId, domain, instance, arbType);
		}
	}

	EsifUp_PutRef(upPtr);
	return rc;
}


esif_error_t EsifArbMgr_Init(void)
{
	esif_error_t rc = ESIF_E_NO_MEMORY;

	esif_ccb_lock_init(&g_arbMgr.mgrLock);
	esif_ccb_lock_init(&g_arbMgr.primitiveQueueLock);

	atomic_set(&g_arbMgr.arbitrationEnabled, ESIF_TRUE);

	/*
	* Create a queue for processing arbitrated primitives so they may be
	* executed outside of locks and still have arbitration be atomic.
	*/
	g_arbMgr.primitiveQueuePtr = esif_queue_create(ESIF_UF_ARBMGR_QUEUE_SIZE, ESIF_UF_ARBMGR_QUEUE_NAME, ESIF_QUEUE_TIMEOUT_INFINITE);
	if (g_arbMgr.primitiveQueuePtr) {
		rc = esif_ccb_thread_create(&g_arbMgr.primitiveQueueThread, EsifArbMgr_PrimitiveQueueExecutionThread, NULL);
	}
	return rc;
}


void EsifArbMgr_Exit(void)
{
	esif_queue_destroy(g_arbMgr.primitiveQueuePtr, (queue_item_destroy_func)EsifArbPrimReq_Destroy);
	g_arbMgr.primitiveQueuePtr = NULL;

	esif_ccb_lock_uninit(&g_arbMgr.primitiveQueueLock);
	esif_ccb_lock_uninit(&g_arbMgr.mgrLock);
}


esif_error_t EsifArbMgr_Start(void)
{
	return ESIF_OK;
}


void EsifArbMgr_Stop(void)
{
	/*
	 * Stop the primitive queue and wait for thread to exit
	 */
	g_arbMgr.primitiveQueueExitFlag = ESIF_TRUE;
	esif_queue_signal_event(g_arbMgr.primitiveQueuePtr);
	esif_ccb_thread_join(&g_arbMgr.primitiveQueueThread);
}


/******************************************************************************
*******************************************************************************
*
* EsifArbCtx Functions
*
*******************************************************************************
******************************************************************************/

/*
 * Arbitration Primitive Execution Filter
 *
 * Creates the arbitration context
 * NOTE(s):
 * 1. Always returns at least a context with the numEntries set to 0 (as long
 * as allocation succeeds).
 * 2. Table creation rules:
 * a. Table populator functions should create as many valid entries as possible,
 * and not stop creation because a single entry fails
 * b. Table populator functions may mark an entry as invalid by simply setting
 * arbitration function to NULL
 */
static EsifArbCtx *EsifArbCtx_Create(
	EsifUpPtr upPtr
	)
{
	EsifArbCtx *self = NULL;
	EsifUpData *upDataPtr = NULL;

	self = (EsifArbCtx *)esif_ccb_malloc(sizeof(*self));
	if (NULL == self) {
		ESIF_TRACE_ERROR("Allocation failure");
		goto exit;
	}

	esif_ccb_lock_init(&self->ctxLock);

	atomic_set(&self->arbitrationEnabled, ESIF_TRUE);

	self->numEntries = 0;
	self->entriesPtr = NULL;

	/* Special trace messages should only be called after setting name */
	self->participantId = EsifUp_GetInstance(upPtr);
	self->participantName = esif_ccb_strdup(EsifUp_GetName(upPtr));

	/* Show handle here to allow association between name and handle first */
	ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_DEBUG, "Creating context for " ESIF_HANDLE_FMT "\n",
		esif_ccb_handle2llu(self->participantId));

	if (0 == esif_ccb_stricmp(ESIF_PARTICIPANT_CPU_NAME, EsifUp_GetName(upPtr))) {
			EsifArbCtx_PopulateDefaultArbitrationTable(self, g_cpuArbTable);
	}
	else {
		upDataPtr = EsifUp_GetMetadata(upPtr);
		if (upDataPtr) {
			if (EsifUpPm_IsPrimaryParticipantId(EsifUp_GetInstance(upPtr))) {
				EsifArbCtx_PopulateDefaultArbitrationTable(self, g_ietmArbTable);
			}
			else if (ESIF_DOMAIN_TYPE_WIRELESS == upDataPtr->fAcpiType) {
				EsifArbCtx_PopulateDefaultArbitrationTable(self, g_wifiArbTable);
			}
			else if (ESIF_DOMAIN_TYPE_FAN == upDataPtr->fAcpiType) {
				EsifArbCtx_PopulateDefaultArbitrationTable(self, g_fanArbTable);
			}
		}
	}
exit:
	return self;
}


static void EsifArbCtx_Destroy(
	EsifArbCtx *self
	)
{
	size_t i = 0;
	EsifArbEntry *entryPtr = NULL;

	/* Locks not required when this is called */
	if (self) {
		ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_DEBUG, "Destroying context\n");

		for (i = 0; i < self->numEntries; i++) {
			entryPtr = self->entriesPtr[i];
			self->entriesPtr[i] = NULL;
			EsifArbEntry_Destroy(entryPtr);
		}
		esif_ccb_free(self->entriesPtr);

		esif_ccb_free(self->participantName);
		esif_ccb_lock_uninit(&self->ctxLock);
	}
	esif_ccb_free(self);
}


static esif_error_t EsifArbCtx_ExecutePrimitive(
	EsifArbCtx *self,
	const esif_handle_t appHandle,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const EsifDataPtr requestPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifArbEntry *arbEntryPtr = NULL;

	if (self) {
		rc = ESIF_E_NOT_SUPPORTED;

		esif_ccb_write_lock(&self->ctxLock);

		if (atomic_read(&self->arbitrationEnabled)) {
			rc = ESIF_E_NOT_FOUND;
			arbEntryPtr = EsifArbCtx_LookupArbEntry_Locked(self, primitiveId, domain, instance);
		}
		esif_ccb_write_unlock(&self->ctxLock);

		if (arbEntryPtr) {
			rc = EsifArbEntry_ExecutePrimitive(arbEntryPtr, appHandle, requestPtr);
		}
	}
	EsifArbEntry_PutRef(arbEntryPtr);

	return rc;
}


/*
 * Gets arbitration information for a participant
 *
 * WARNING!!! Caller is expected to free returned pointer if not NULL
 *
 * Else if primitiveId is 0; gets all entries
 * Else gets information for a specific entry
 *
 */
EsifArbCtxInfo *EsifArbCtx_GetInformation(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
)
{
	EsifArbCtxInfo *infoPtr = NULL;
	EsifArbEntryInfo *curEntryInfoPtr = NULL;
	EsifArbEntry **curEntryPtr = NULL;
	size_t reqSize = 0;
	size_t numEntries = 1;  /* Assume a single entry requested */
	size_t entryIndex = 0;

	UNREFERENCED_PARAMETER(domain);

	if (self) {
		esif_ccb_write_lock(&self->ctxLock);

		/* If getting all entries */
		if (0 == primitiveId) {
			numEntries = self->numEntries;
		}

		reqSize = sizeof(*infoPtr);
		reqSize += self->numEntries > 0 ? (self->numEntries - 1) * sizeof(*infoPtr->arbEntryInfo) : 0;

		infoPtr = (EsifArbCtxInfo *)esif_ccb_malloc(reqSize);
		if (!infoPtr) {
			ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_ERROR, "Allocation failure");
		}

		if (infoPtr) {
			/* Get the context information */
			infoPtr->size = reqSize;
			infoPtr->participantId = self->participantId;
			infoPtr->count = self->numEntries;
			infoPtr->arbitrationEnabled = (Bool)atomic_read(&self->arbitrationEnabled);

			ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_DEBUG, "Getting participant arbitration information : State = %s, Num Entries = %lu, Size = %lu\n",
				infoPtr->arbitrationEnabled ? "Enabled" : "Disabled", infoPtr->count, infoPtr->size);

			/* Get the entry information */
			curEntryInfoPtr = infoPtr->arbEntryInfo;
			curEntryPtr = self->entriesPtr;
			for (entryIndex = 0; entryIndex < self->numEntries; entryIndex++, curEntryInfoPtr++, curEntryPtr++) {
				if ((0 == primitiveId) || /* Getting all entries */
					EsifArbEntry_IsMatchingEntry(*curEntryPtr, primitiveId, domain, instance)) { /* Getting specific entry */
					EsifArbEntry_GetInformation(*curEntryPtr, curEntryInfoPtr);
				}
			}
		}
		esif_ccb_write_unlock(&self->ctxLock);
	}

	return infoPtr;
}


static esif_error_t EsifArbCtx_PopulateDefaultArbitrationTable(
	EsifArbCtx *self,
	EsifArbEntryParams *entryParamsPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifArbEntryParams *curParamPtr = entryParamsPtr;
	EsifArbEntry *entryPtr = NULL;

	if (self && entryParamsPtr) {
		while (curParamPtr->primitiveId != 0) {
			if (IsPrimitiveSupported(self->participantId, curParamPtr->primitiveId, curParamPtr->domain, curParamPtr->instance)) {

				/* Best effort */
				EsifArbCtx_CreateAndInsertEntry_Locked(
					self,
					curParamPtr,
					self->participantId,
					self->participantName,
					&entryPtr);

				EsifArbEntry_PutRef(entryPtr); /* Not using, so just release reference */
				entryPtr = NULL;
			}
			curParamPtr++;
		}
		rc = ESIF_OK;
	}
	return rc;
}


/* Caller is expected to hold the ctxLock */
static esif_error_t EsifArbCtx_InsertArbEntry_Locked(
	EsifArbCtx *self,
	EsifArbEntry *entryPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifArbEntry **newEntriesPtr = NULL;

	if (self) {
		/* Check if capacity needed to add new entry */
		if (self->numEntries < self->entryCapacity) {
			self->entriesPtr[self->numEntries++] = entryPtr;
			rc = ESIF_OK;
		}
		else {
			rc = ESIF_E_NO_MEMORY;

			newEntriesPtr = esif_ccb_realloc(self->entriesPtr, (self->entryCapacity + ESIF_ARB_CTX_ENTRY_TABLE_GROWTH_RATE) * sizeof(*self->entriesPtr));
			if (newEntriesPtr) {
				self->entriesPtr = newEntriesPtr;
				self->entryCapacity += ESIF_ARB_CTX_ENTRY_TABLE_GROWTH_RATE;

				/* Try again after growing capacity */
				rc = EsifArbCtx_InsertArbEntry_Locked(self, entryPtr);
			}
			else {
				ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_ERROR, "Allocation failure\n");
			}
		}
	}
	return rc;
}


/* Caller is expected to hold the ctxLock */
static esif_error_t EsifArbCtx_CreateAndInsertEntry_Locked(
	EsifArbCtx *self,
	EsifArbEntryParams *paramPtr,
	const esif_handle_t participantId,
	esif_string participantName,
	EsifArbEntry **entryPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifArbEntry *newEntryPtr = NULL;

	if (self && paramPtr && entryPtr) {
		rc = ESIF_E_NO_MEMORY;

		newEntryPtr = EsifArbEntry_Create(paramPtr, participantId, participantName);
		if (newEntryPtr) {

			rc = EsifArbEntry_GetRef(newEntryPtr); /* Take a reference on the entry to be passed back */
			if (ESIF_OK == rc) {
				rc = EsifArbCtx_InsertArbEntry_Locked(self, newEntryPtr);
			}
			else {
				ESIF_ASSERT(0);
				ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_ERROR, "Unable to get reference on entry\n");
			}

			if (ESIF_OK == rc) {
				*entryPtr = newEntryPtr;
			}
		}
	}
	if (rc != ESIF_OK) {
		EsifArbEntry_Destroy(newEntryPtr);
	}
	return rc;
}


/* Caller is expected to hold the ctxLock */
static esif_error_t EsifArbCtx_RemoveArbEntry_Locked(
	EsifArbCtx *self,
	EsifArbEntry *entryPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifArbEntry **entryMovePtr = NULL;
	EsifArbEntry **newArbListPtr = NULL;
	size_t index = 0;
	size_t moveSize = 0;
	size_t newCapacity = 0;

	if (self && entryPtr) {
		rc = ESIF_OK;

		/* Find the entry to remove */
		for (index = 0; index < self->numEntries; index++) {
			if (entryPtr == self->entriesPtr[index]) {
				self->entriesPtr[index] = NULL;
				break;
			}
		}

		/* If found, move other entires to replace empty slot */
		if (index < self->numEntries) {
			moveSize = (self->numEntries - index - 1) * sizeof(*self->entriesPtr);

			if (moveSize > 0) {
				entryMovePtr = &self->entriesPtr[index];
				esif_ccb_memcpy(entryMovePtr, entryMovePtr + 1, moveSize);
			}
			self->numEntries--;
		}

		/* Shrink the list if needed */
		if ((self->entryCapacity - self->numEntries) > ESIF_ARB_CTX_ENTRY_TABLE_GROWTH_RATE) {
			if (0 == self->numEntries) {
				esif_ccb_free(self->entriesPtr);
				self->entriesPtr = NULL;
				self->entryCapacity = 0;
			}
			else {
				newCapacity = self->entryCapacity - ESIF_ARB_CTX_ENTRY_TABLE_GROWTH_RATE;
				newArbListPtr = esif_ccb_realloc(self->entriesPtr, newCapacity * sizeof(*self->entriesPtr));
				if (newArbListPtr) {
					self->entriesPtr = newArbListPtr;
					self->entryCapacity = newCapacity;
				}
				else {
					ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_DEBUG, "Reallocation failure\n");
				}
			}
		}
	}
	return rc;
}


/*
 * WARNING!!! Caller responsible for calling EsifArbEntry_PutRef after use
 * Caller is expected to hold the ctxLock
 */
static EsifArbEntry *EsifArbCtx_LookupArbEntry_Locked(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
	)
{
	EsifArbEntry *entryPtr = NULL;
	EsifArbEntry **curEntryPtr = NULL;
	size_t i = 0;

	/*
	 * Looking for valid entry based on the primitive and instance.
	 */
	if (self && self->numEntries && self->entriesPtr) {
		curEntryPtr = self->entriesPtr;
		for (i = 0; i < self->numEntries; i++, curEntryPtr++) {
			if (EsifArbEntry_IsMatchingEntry(*curEntryPtr, primitiveId, domain, instance)) {
				if (ESIF_OK == EsifArbEntry_GetRef(*curEntryPtr)) {
					entryPtr = *curEntryPtr;
					break;
				}
			}
		}
	}
	return entryPtr;
}


/*
 * Used to iterate through the arbitration entries.
 * First call EsifArbCtx_InitEntryIterator to initialize the iterator.
 * Next, call EsifArbCtx_GetNextEntry using the iterator.  Repeat until
 * EsifArbCtx_GetNextEntry fails. The call will release the reference of the
 * object from the previous call.  If you stop iteration part way through
 * all object, the caller is responsible for releasing the reference on
 * the last object returned.  Iteration is complete when
 * ESIF_E_ITERATOR_DONE is returned.
 */
static esif_error_t EsifArbCtx_InitEntryIterator(
	EsifArbEntryIterator *iterPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (iterPtr) {
		esif_ccb_memset(iterPtr, 0, sizeof(*iterPtr));
		iterPtr->marker = ESIF_ARB_ENTRY_ITERATOR_MARKER;
	}

	return rc;
}


/*
 * See EsifArbCtx_InitEntryIterator for usage
 */
static esif_error_t EsifArbCtx_GetNextEntry(
	EsifArbCtx *self,
	EsifArbEntryIterator *iterPtr,
	EsifArbEntry **entryPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && iterPtr && entryPtr) {

		rc = ESIF_E_INVALID_HANDLE;
		if (ESIF_ARB_ENTRY_ITERATOR_MARKER == iterPtr->marker) {
			rc = ESIF_E_ITERATION_DONE;

			if (iterPtr->valid) {
				iterPtr->index++;
				EsifArbEntry_PutRef(iterPtr->entryPtr);
				iterPtr->entryPtr = NULL;
				iterPtr->valid = ESIF_FALSE;
			}

			esif_ccb_write_lock(&self->ctxLock);

			while (iterPtr->index < self->numEntries) {
				iterPtr->entryPtr = self->entriesPtr[iterPtr->index];

				if (ESIF_OK == EsifArbEntry_GetRef(iterPtr->entryPtr)) {
					*entryPtr = iterPtr->entryPtr;
					iterPtr->valid = ESIF_TRUE;
					rc = ESIF_OK;
					break;
				}
				iterPtr->entryPtr = NULL;
				iterPtr->index++;
			}

			esif_ccb_write_unlock(&self->ctxLock);
		}
		else {
			ESIF_ASSERT(0);
			ESIF_TRACE_ERROR("Invalid iterator\n");
		}
	}
	return rc;
}


static void EsifArbCtx_RemoveApp(
	EsifArbCtx *self,
	esif_handle_t appHandle
	)
{
	esif_error_t iterRc = ESIF_OK;
	EsifArbEntry *entryPtr = NULL;
	EsifArbEntryIterator entryIter = { 0 };

	if (self) {
		EsifArbCtx_InitEntryIterator(&entryIter);

		iterRc = EsifArbCtx_GetNextEntry(self, &entryIter, &entryPtr);
		while (ESIF_OK == iterRc) {
			EsifArbEntry_RemoveApp(entryPtr, appHandle);
			iterRc = EsifArbCtx_GetNextEntry(self, &entryIter, &entryPtr);
		}

		if (iterRc != ESIF_E_ITERATION_DONE) {
			/* Should always complete iteration */
			ESIF_ASSERT(0);
			EsifArbEntry_PutRef(entryPtr);
			ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_ERROR, "Iteration error : entryPtr = %p, iterRc = %d\n", entryPtr, iterRc);
		}
	}
}


static esif_error_t EsifArbCtx_SetArbitrationFunction(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	esif_arbitration_type_t arbType
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifArbEntry *entryPtr = NULL;
	Bool isPrimSupported = ESIF_FALSE;
	EsifArbEntryParams entryParams = {
		.primitiveId = primitiveId,
		.domain = domain,
		.instance = instance,
		.arbType = arbType,
		.upperLimit = ESIF_ARB_LIMIT_MAX,
		.lowerLimit = ESIF_ARB_LIMIT_MIN
	};

	if (self) {
		esif_ccb_write_lock(&self->ctxLock);

		entryPtr = EsifArbCtx_LookupArbEntry_Locked(self, primitiveId, domain, instance);

		/* If we can't find an entry, create and insert one */
		if (!entryPtr) {
			rc = ESIF_E_NOT_SUPPORTED;

			isPrimSupported = IsPrimitiveSupported(self->participantId, primitiveId, domain, instance);
			if (isPrimSupported) {
				rc = EsifArbCtx_CreateAndInsertEntry_Locked(self, &entryParams, self->participantId, self->participantName, &entryPtr);
			}
			ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_DEBUG, "[Prim = %lu, Inst = %lu] : Inserting arbitration entry : isPrimSupported = %d, entryPtr = %p; rc = %d",
				primitiveId, instance, isPrimSupported, entryPtr, rc);
		}

		if (entryPtr) {
			rc = EsifArbEntry_SetArbitrationFunction(entryPtr, arbType);
		}

		EsifArbEntry_PutRef(entryPtr);
		esif_ccb_write_unlock(&self->ctxLock);
	}
	return rc;
}


static void EsifArbCtx_PurgeRequests(
	EsifArbCtx *self
)
{
	if (self) {
		esif_ccb_write_lock(&self->ctxLock);
		EsifArbCtx_PurgeRequests_Locked(self);
		esif_ccb_write_unlock(&self->ctxLock);
	}
}


static void EsifArbCtx_PurgeRequests_Locked(
	EsifArbCtx *self
	)
{
	size_t i = 0;
	EsifArbEntry **curEntryPtr = NULL;

	if (self && self->entriesPtr) {
		ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_DEBUG, "Removing arbitration requests for all entries");

		curEntryPtr = self->entriesPtr;
		for (i = 0; i < self->numEntries; i++) {
			EsifArbEntry_PurgeRequests(*curEntryPtr++);
		}
	}
}


static esif_error_t EsifArbCtx_SetArbitrationState(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const Bool isEnabled
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifArbEntry *entryPtr = NULL;

	if (self) {

		/* If applies to all entries... */
		if (0 == primitiveId) {
			rc = ESIF_OK;
			esif_ccb_write_lock(&self->ctxLock);
			atomic_set(&self->arbitrationEnabled, isEnabled);
			if (!isEnabled) {
				EsifArbCtx_PurgeRequests_Locked(self);
			}
			ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_DEBUG, "%s arbitration for participant", isEnabled ? "Enabled" : "Disabled");
			esif_ccb_write_unlock(&self->ctxLock);
		}
		/* Else applies to specific entry */
		else {
			rc = ESIF_E_NOT_FOUND;

			esif_ccb_write_lock(&self->ctxLock);

			entryPtr = EsifArbCtx_LookupArbEntry_Locked(self, primitiveId, domain, instance);
			if (entryPtr) {

				rc = EsifArbEntry_SetArbitrationState(entryPtr, isEnabled);
				if ((ESIF_OK == rc) && isEnabled) {

					ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_DEBUG, "Enabled arbitration for participant");
					atomic_set(&self->arbitrationEnabled, isEnabled);
				}
			}
			esif_ccb_write_unlock(&self->ctxLock);
		}
	}
	EsifArbEntry_PutRef(entryPtr);
	return rc;
}


static esif_error_t EsifArbCtx_StopArbitration(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	esif_error_t iterRc = ESIF_E_UNSPECIFIED;
	EsifArbEntryIterator entryIter = { 0 };
	EsifArbEntry *curEntryPtr = NULL;

	if (self) {
		rc = ESIF_E_NOT_FOUND;

		EsifArbCtx_InitEntryIterator(&entryIter);

		iterRc = EsifArbCtx_GetNextEntry(self, &entryIter, &curEntryPtr);
		while (ESIF_OK == iterRc) {
			rc = ESIF_E_NOT_FOUND;

			if (EsifArbEntry_IsMatchingEntry(curEntryPtr, primitiveId, domain, instance)) {

				esif_ccb_write_lock(&self->ctxLock);

				rc = EsifArbCtx_RemoveArbEntry_Locked(self, curEntryPtr);

				ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_DEBUG, "[Prim = %lu, Inst = %lu] Stopped arbitration; rc = %d",
					primitiveId, instance, rc);

				esif_ccb_write_unlock(&self->ctxLock);

				/*
				* Only destroy the entry if able to successfully remove from table.
				*/
				if (ESIF_OK == rc) {
					EsifArbEntry_PutRef(curEntryPtr); /* Release iteration reference before destroying */
					EsifArbEntry_Destroy(curEntryPtr);
					curEntryPtr = NULL;
				}
				break;
			}
			iterRc = EsifArbCtx_GetNextEntry(self, &entryIter, &curEntryPtr);
		}

		if (iterRc != ESIF_E_ITERATION_DONE) {
			EsifArbEntry_PutRef(curEntryPtr);

			if (iterRc != ESIF_OK) {
				ESIF_ASSERT(0);
				ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_ERROR, "[Prim = %lu, Inst = %lu] : Iteration error : curEntryPtr = %p, iterRc = %d, rc = %d\n",
					primitiveId, instance,
					curEntryPtr, iterRc, rc);
				rc = iterRc;
			}
		}
	}
	return rc;
}


/*
* Sets the limits for a given participant/primitive/instance
* Note:  Creates the entry if not present
*/
static esif_error_t EsifArbCtx_SetLimits(
	EsifArbCtx *self,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	UInt32 *upperLimitPtr,
	UInt32 *lowerLimitPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifArbEntry *entryPtr = NULL;
	EsifArbEntryParams entryParams = {
		.primitiveId = primitiveId,
		.domain = domain,
		.instance = instance,
		.arbType = ESIF_ARBITRATION_INVALID,
		.upperLimit = ESIF_ARB_LIMIT_MAX,
		.lowerLimit = ESIF_ARB_LIMIT_MIN
	};

	if (self) {
		esif_ccb_write_lock(&self->ctxLock);

		entryPtr = EsifArbCtx_LookupArbEntry_Locked(self, primitiveId, domain, instance);

		/* If we can't find an entry, create and insert one */
		if (!entryPtr) {
			rc = ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;

			if (IsPrimitiveSupported(self->participantId, primitiveId, domain, instance)) {
				rc = EsifArbCtx_CreateAndInsertEntry_Locked(self, &entryParams, self->participantId, self->participantName, &entryPtr);
				if (rc != ESIF_OK) {
					ESIF_ASSERT(0);
					ESIF_TRACE_ARB_CTX(ESIF_TRACELEVEL_ERROR, "[Prim = %lu, Inst = %lu] Failed to add entry : rc = %d\n",
						primitiveId, instance, rc);
				}
			}
		}

		if (entryPtr) {
			rc = EsifArbEntry_SetLimits(entryPtr, upperLimitPtr, lowerLimitPtr);
		}

		EsifArbEntry_PutRef(entryPtr);
		esif_ccb_write_unlock(&self->ctxLock);
	}
	return rc;
}



/******************************************************************************
*******************************************************************************
*
* EsifArbEntry Functions
*
*******************************************************************************
******************************************************************************/

static EsifArbEntry *EsifArbEntry_Create(
	EsifArbEntryParams *entryParamsPtr,
	const esif_handle_t participantId,
	esif_string participantName
	)
{
	EsifArbEntry *self = NULL;

	self = (EsifArbEntry *)esif_ccb_malloc(sizeof(*self));
	if (NULL == self) {
		ESIF_TRACE_ERROR("Allocation failure\n");
		goto exit;
	}

	if (entryParamsPtr) {
		/* Set parameters */
		self->arbType = entryParamsPtr->arbType;
		self->instance = entryParamsPtr->instance;
		self->domain = entryParamsPtr->domain;
		self->lowerLimit = entryParamsPtr->lowerLimit;
		self->upperLimit = entryParamsPtr->upperLimit;
		self->primitiveId = entryParamsPtr->primitiveId;

		atomic_set(&self->arbitrationEnabled, ESIF_TRUE);

		self->refCount = 1;
		self->markedForDelete = ESIF_FALSE;
		self->participantId = participantId;
		self->participantName = esif_ccb_strdup(participantName);

		esif_ccb_lock_init(&self->entryLock);
		esif_ccb_event_init(&self->deleteEvent);
		esif_ccb_event_reset(&self->deleteEvent);

		ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Initialized: Enabled = %lu, arbType = %lu, upper = 0x%08X, lower = 0x%08X",
			atomic_read(&self->arbitrationEnabled),
			self->arbType,
			self->upperLimit,
			self->lowerLimit);

	}
exit:
	return self;
}


static void EsifArbEntry_Destroy(
	EsifArbEntry *self
	)
{
	if (self) {

		self->markedForDelete = ESIF_TRUE;

		EsifArbEntry_PutRef(self); /* Must release reference we own */

		ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Destroying arbitration entry: waiting for delete event...\n");
		esif_ccb_event_wait(&self->deleteEvent);

		esif_link_list_free_data(&self->arbRequests, (queue_item_destroy_func)EsifArbReq_Destroy);
		esif_ccb_free(self->participantName);

		esif_ccb_event_uninit(&self->deleteEvent);
		esif_ccb_lock_uninit(&self->entryLock);

		/* Can't use special macro here as self is no lonter valid */
		ESIF_TRACE_DEBUG("Arbitration entry destroyed\n");
	}
	esif_ccb_free(self);
}


/*
 * Takes a reference on the object (prevents destruction until all references are released.)
 * WARNING!!! Callers are responible for calling EsifArbEntry_PutRef on the
 * object when done using it.
 */
static esif_error_t EsifArbEntry_GetRef(
	EsifArbEntry *self
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		rc = ESIF_E_UNSPECIFIED;

		esif_ccb_write_lock(&self->entryLock);
		if (!self->markedForDelete) {
			self->refCount++;
			rc = ESIF_OK;
		}
		else {
			ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_ERROR, "Unabled to get reference; marked-for-delete\n");
		}
		esif_ccb_write_unlock(&self->entryLock);
	}
	return rc;
}


/*
 * Releases a reference on the object (allows destruction when all references are
 * released.)
 */
static void EsifArbEntry_PutRef(
	EsifArbEntry *self
	)
{
	Bool needsRelease = ESIF_FALSE;

	if (self) {
		esif_ccb_write_lock(&self->entryLock);

		self->refCount--;

		if ((0 == self->refCount) && (self->markedForDelete)) {
			needsRelease = ESIF_TRUE;
		}

		esif_ccb_write_unlock(&self->entryLock);

		if (needsRelease == ESIF_TRUE) {
			ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Signal delete event\n");
			esif_ccb_event_set(&self->deleteEvent);
		}
	}
}


static esif_error_t EsifArbEntry_ExecutePrimitive(
	EsifArbEntry *self,
	const esif_handle_t appHandle,
	EsifData *requestPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifData *prevArbDataPtr = NULL;
	EsifData *curArbDataPtr = NULL;

	if (self) {
		rc = ESIF_E_NOT_SUPPORTED;
		esif_ccb_write_lock(&self->entryLock);
		if (atomic_read(&self->arbitrationEnabled)) {
			prevArbDataPtr = EsifArbEntry_GetArbitratedRequestData_Locked(self);

			rc = EsifArbEntry_InsertRequest_Locked(self, appHandle, requestPtr);

			ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Inserted arbitration request for " ESIF_HANDLE_FMT "; rc = %d",
				esif_ccb_handle2llu(appHandle),
				rc);

			/*
			 * If successful, check to see if there is a change in the arbitration
			 * winner.  If so, queue the primitive for execution.
			 */
			if (ESIF_OK == rc) {

				curArbDataPtr = EsifArbEntry_GetArbitratedRequestData_Locked(self);

				if (curArbDataPtr && (curArbDataPtr != prevArbDataPtr)) {
					ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Arbitrated value changed");
					/*
					 * We don't want to execute the primitive while holding a lock,
					 * so queue the request for later processing.  Execution order is
					 * guaranteed, but is asynchronous.
					 */
					rc = EsifArbEntry_QueueLimitedPrimitiveRequest_Locked(self, curArbDataPtr, ESIF_TRUE);
				}
			}
		}
		esif_ccb_write_unlock(&self->entryLock);
	}
	return rc;
}


static esif_error_t EsifArbEntry_InsertRequest_Locked(
	EsifArbEntry *self,
	const esif_handle_t appHandle,
	const EsifData *requestPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifArbReq *arbReqPtr = NULL;

	if (self && requestPtr) {
		arbReqPtr = EsifArbReq_Create(appHandle, requestPtr);
		rc = EsifArbEntry_ArbitrateRequest_Locked(self, arbReqPtr);
	}

	if (rc != ESIF_OK) {
		EsifArbReq_Destroy(arbReqPtr);
	}
	return rc;
}


static esif_error_t EsifArbEntry_ArbitrateRequest_Locked(
	EsifArbEntry *self,
	EsifArbReq *arbReqPtr
	)
{
	esif_error_t rc = ESIF_OK;
	Bool isInserted = ESIF_FALSE;
	Bool isOldRemoved = ESIF_FALSE;
	Bool isArbitable = ESIF_FALSE;
	EsifLinkListNode *curNodePtr = NULL;
	EsifLinkListNode *nextNodePtr = NULL;
	EsifArbReq *curArbReqPtr = NULL;
	int arbResult = 0;
	EsifArbMgr_ArbitratorFunc arbFunc = NULL;


	/* If nothing to arbitrate, exit */
	if ((NULL == self) || (NULL == arbReqPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Get the arbitration function */
	arbFunc = GetArbitrationFunction(self->arbType);

	/*
	 * Go through the arbitrated requests and remove any for the specified
	 * application.
	 * If the request is better; insert it there, but continue to remove all
	 * other requests for the application.
	 * If there is a failure during arbitration.
	 */
	curNodePtr = self->arbRequests.head_ptr;

	while (curNodePtr && !(isInserted && isOldRemoved)) {
		nextNodePtr = curNodePtr->next_ptr;

		curArbReqPtr = (EsifArbReq *)curNodePtr->data_ptr;

		if (curArbReqPtr) {
			/*
			 * Remove any other entries for the same application
			 */
			if (EsifArbReq_GetAppHandle(curArbReqPtr) == EsifArbReq_GetAppHandle(arbReqPtr)) {

				ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Removed arbitration requested for " ESIF_HANDLE_FMT,
					esif_ccb_handle2llu(EsifArbReq_GetAppHandle(arbReqPtr)));

				EsifArbReq_Destroy(curArbReqPtr);
				esif_link_list_node_remove(&self->arbRequests, curNodePtr);
				isOldRemoved = ESIF_TRUE;
			}
			/*
			 * Continue iteration after insertion to remove all other requests for current application
			 */
			else if (arbFunc && !isInserted) {
				/*
				* Arbitrate the request with the current request;
				* returns > 0 if 1 better than 2
				* returns < 0 2 better than 1
				* returns 0 if equal
				* (Also checks to see if data is valid for arbitration.)
				*/
				arbResult = 0;
				rc = arbFunc(arbReqPtr, &isArbitable, curArbReqPtr, NULL, &arbResult);

				/*
					* If result is better, place request before current node.
					* (Done by copying current request to after current entry;
					* and then replace data in current node with new data.)
					*/
				if (((ESIF_OK == rc) && (arbResult > 0)) ||
					((rc != ESIF_OK) && isArbitable)) { /* Inarbitable entry/empty current request */

					ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Inserted arbitration request (arb) for " ESIF_HANDLE_FMT,
						esif_ccb_handle2llu(EsifArbReq_GetAppHandle(arbReqPtr)));

					esif_link_list_add_after(&self->arbRequests, curNodePtr, curArbReqPtr);
					curNodePtr->data_ptr = arbReqPtr;
					isInserted = ESIF_TRUE;
				}
			}
		}
		curNodePtr = nextNodePtr;
	}

	if (!isInserted) {
		ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Inserted arbitration request (unarb) for " ESIF_HANDLE_FMT "\n",
			esif_ccb_handle2llu(EsifArbReq_GetAppHandle(arbReqPtr)));

		esif_link_list_add_at_back(&self->arbRequests, arbReqPtr);
	}
	rc = ESIF_OK;
exit:
	return rc;
}


static esif_error_t EsifArbEntry_GetInformation(
	EsifArbEntry *self,
	EsifArbEntryInfo *infoPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && infoPtr) {
		rc = ESIF_OK;

		esif_ccb_write_lock(&self->entryLock);

		infoPtr->participantId = self->participantId;
		infoPtr->domain = self->domain;
		infoPtr->primitiveId = self->primitiveId;
		infoPtr->instance = self->instance;

		infoPtr->arbType = self->arbType;
		infoPtr->upperLimit = self->upperLimit;
		infoPtr->lowerLimit = self->lowerLimit;

		atomic_set(&infoPtr->arbitrationEnabled, atomic_read(&self->arbitrationEnabled));

		ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Arbitration Information: "
			"Arbitration Type = %lu,"
			"Upper Limit = 0x%08X,"
			"Lower Limit = 0x%08X,"
			"Arbitration is %s,",
			infoPtr->arbType,
			infoPtr->upperLimit, infoPtr->lowerLimit,
			infoPtr->arbitrationEnabled ? "Enabled" : "Disabled");

		esif_ccb_write_unlock(&self->entryLock);
	}
	return rc;
}


/*
* Sets the limits for a given participant/primitive/instance
*/
static esif_error_t EsifArbEntry_SetLimits(
	EsifArbEntry *self,
	UInt32 *upperLimitPtr,
	UInt32 *lowerLimitPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	UInt32 reqUpperLimit = upperLimitPtr ? *upperLimitPtr : ESIF_ARB_LIMIT_MAX;
	UInt32 reqLowerLimit = lowerLimitPtr ? *lowerLimitPtr : ESIF_ARB_LIMIT_MIN;
	UInt32 newUpperLimit = ESIF_ARB_LIMIT_MAX;
	UInt32 newLowerLimit = ESIF_ARB_LIMIT_MIN;
	EsifData *arbDataPtr = NULL;

	if (self) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;

		esif_ccb_write_lock(&self->entryLock);

		newUpperLimit = upperLimitPtr ? reqUpperLimit : self->upperLimit;
		newLowerLimit = lowerLimitPtr ? reqLowerLimit : self->lowerLimit;

		if (newUpperLimit >= newLowerLimit) {
			rc = ESIF_OK;

			/* Only process if there is a change in limits */
			if ((self->upperLimit != newUpperLimit) ||
				(self->lowerLimit != newLowerLimit)) {

				self->upperLimit = newUpperLimit;
				self->lowerLimit = newLowerLimit;

				arbDataPtr = EsifArbEntry_GetArbitratedRequestData_Locked(self);
				EsifArbEntry_QueueLimitedPrimitiveRequest_Locked(self, arbDataPtr, ESIF_FALSE);
			}
		}

		ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Set upper = 0x%08X(%lu)[Req=0x%08X, %s], lower = 0x%08X(%lu)[Req=0x%08X, %s]; rc = %d",
			self->upperLimit, self->upperLimit, reqUpperLimit, upperLimitPtr ? "valid" : "invalid",
			self->lowerLimit, self->lowerLimit, reqLowerLimit, lowerLimitPtr ? "valid" : "invalid",
			rc);
		esif_ccb_write_unlock(&self->entryLock);
	}
	return rc;
}


static esif_error_t EsifArbEntry_SetArbitrationState(
	EsifArbEntry *self,
	const Bool isEnabled
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		rc = ESIF_OK;

		esif_ccb_write_lock(&self->entryLock);

		atomic_set(&self->arbitrationEnabled, isEnabled);

		/* If disabling, clear the request list*/
		if (!isEnabled) {
			EsifArbEntry_PurgeRequests_Locked(self);
		}

		ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "%s arbitration", isEnabled ? "Enabled" : "Disabled");

		esif_ccb_write_unlock(&self->entryLock);
	}
	return rc;
}


static esif_error_t EsifArbEntry_SetArbitrationFunction(
	EsifArbEntry *self,
	esif_arbitration_type_t arbType
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifData *prevArbDataPtr = NULL;
	EsifData *curArbDataPtr = NULL;

	if (self) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;

		esif_ccb_write_lock(&self->entryLock);

		/*
		 * If the arbitration type is valid, change it and then re-arbitrate
		 * requests.
		 */
		if (GetArbitrationFunction(arbType)) {
			self->arbType = arbType;

			prevArbDataPtr = EsifArbEntry_GetArbitratedRequestData_Locked(self);

			ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Changing arbitration function to %u", arbType);

			rc = EsifArbEntry_RearbitrateRequests_Locked(self);
			/*
			 * If successful, check to see if there is a change in the arbitration
			 * winner.  If so, queue the primitive for execution.
			 */
			if (ESIF_OK == rc) {

				curArbDataPtr = EsifArbEntry_GetArbitratedRequestData_Locked(self);

				if (curArbDataPtr && (curArbDataPtr != prevArbDataPtr)) {
					ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, " Arbitrated value changed");

					/*
					 * We don't want to execute the primitive while holding a lock,
					 * so queue the request for later processing.  Execution order is
					 * guaranteed, but is asynchronous.
					 */
					rc = EsifArbEntry_QueueLimitedPrimitiveRequest_Locked(self, curArbDataPtr, ESIF_FALSE);
				}
			}
		}
		else {
			ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_WARN, "Invalid function requested = %lu", arbType);
		}
		esif_ccb_write_unlock(&self->entryLock);
	}
	return rc;
}

/* The entryLock is expect to be held when called */
static esif_error_t EsifArbEntry_QueueLimitedPrimitiveRequest_Locked(
	EsifArbEntry *self,
	const EsifDataPtr requestPtr,
	Bool requiresDelay /* Indicates if incoming associated requests must be delayed until completion of this request */
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifData *limitedClonedDataPtr = NULL;
	EsifData *dataPtr = requestPtr;

	if (self && requestPtr) {

		limitedClonedDataPtr = EsifArbEntry_CreateLimitedData_Locked(self, requestPtr);
		if (limitedClonedDataPtr) {
			dataPtr = limitedClonedDataPtr;
		}

		esif_ccb_write_lock(&g_arbMgr.primitiveQueueLock);

		rc = EsifArbMgr_QueuePrimitiveRequest_Locked(
			self->participantId,
			self->primitiveId, self->domain, self->instance,
			dataPtr,
			NULL,
			NULL,
			NULL,
			requiresDelay);

		esif_ccb_write_unlock(&g_arbMgr.primitiveQueueLock);

	}

	EsifData_Destroy(limitedClonedDataPtr);
	return rc;
}


/*
 * Returns a new EsifData structure based on the original input with the value
 * limited if limiting is required.  Return NULL if orginal input is does not
 * need to be limited.
 *
 * WARNING!!! It is the responsibility of the caller to call EsifData_Destroy on
 * the returned data pointer.
 */
static EsifData *EsifArbEntry_CreateLimitedData_Locked(
	EsifArbEntry *self,
	EsifData *dataPtr
)
{
	EsifData *limitedDataPtr = NULL;
	UInt32 unlimitedValue = 0;
	UInt32 limitedValue = 0;

	if (self && dataPtr && dataPtr->buf_ptr && (dataPtr->buf_len >= sizeof(limitedValue))) {

		/* Determine the limited value */
		unlimitedValue = *((UInt32 *)dataPtr->buf_ptr);
		limitedValue = esif_ccb_max(unlimitedValue, self->lowerLimit);
		limitedValue = esif_ccb_min(limitedValue, self->upperLimit);

		/*
		* If the limited value is not the same as the unlimited value, we create a
		* clone of the request with the limited value
		*/
		if (unlimitedValue != limitedValue) {

			ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Limiting request; Req value = %lu, limited value = %lu",
				unlimitedValue,
				limitedValue
			);

			limitedDataPtr = EsifData_Clone(dataPtr);
			if (limitedDataPtr) {
				*((UInt32 *)limitedDataPtr->buf_ptr) = limitedValue;
			}
		}
	}
	return limitedDataPtr;
}


/* EntryLock is expected to be held when called */
static esif_error_t EsifArbEntry_RearbitrateRequests_Locked(
	EsifArbEntry *self
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;;
	EsifLinkList tempList = { 0 };
	EsifLinkListNode *curNodePtr = NULL;
	EsifArbReq *curReqPtr = NULL;

	if (self) {
		rc = ESIF_OK;
		/*
		* Copy the current request list and clear it in the entry.
		* Then rearbitrate each item in the list.
		*/
		tempList = self->arbRequests;

		/*
		 * Clear list in self...
		 */
		self->arbRequests.nodes = 0;
		self->arbRequests.head_ptr = NULL;
		self->arbRequests.tail_ptr = NULL;

		curNodePtr = tempList.head_ptr;
		while (curNodePtr) {
			curReqPtr = (EsifArbReq *)curNodePtr->data_ptr;

			EsifArbEntry_ArbitrateRequest_Locked(self, curReqPtr);

			esif_link_list_node_remove(&tempList, curNodePtr);
			curNodePtr = tempList.head_ptr;
		}
	}

	return rc;
}


static	EsifData *EsifArbEntry_GetArbitratedRequestData_Locked(
	EsifArbEntry *self
	)
{
	EsifArbReq *arbReqPtr = NULL;
	EsifData *arbReqDataPtr = NULL;

	/*
	 * Head of list is considered the arbitration winner
	 */
	if (self && self->arbRequests.head_ptr) {
		arbReqPtr = (EsifArbReq *)self->arbRequests.head_ptr->data_ptr;
		arbReqDataPtr = EsifArbReq_GetRequestData(arbReqPtr);
	}

	return arbReqDataPtr;
}


static void EsifArbEntry_RemoveApp(
	EsifArbEntry *self,
	esif_handle_t appHandle
	)
{
	EsifData *prevArbDataPtr = NULL;
	EsifData *curArbDataPtr = NULL;
	EsifLinkListNode *curNodePtr = NULL;
	EsifLinkListNode *nextNodePtr = NULL;
	EsifArbReq *curReqPtr = NULL;

	if (self) {
		esif_ccb_write_lock(&self->entryLock);

		prevArbDataPtr = EsifArbEntry_GetArbitratedRequestData_Locked(self);

		/*
		 * Remove requests for the specified app
		 */
		curNodePtr = self->arbRequests.head_ptr;
		while (curNodePtr) {
			nextNodePtr = curNodePtr->next_ptr; /* Save next node as the curren node may be removed */

			curReqPtr = (EsifArbReq *)curNodePtr->data_ptr;

			if (appHandle == EsifArbReq_GetAppHandle(curReqPtr)) {
				ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Removing arbitration request for " ESIF_HANDLE_FMT,
					esif_ccb_handle2llu(appHandle));

				EsifArbReq_Destroy(curReqPtr);
				esif_link_list_node_remove(&self->arbRequests, curNodePtr);
			}
			curNodePtr = nextNodePtr;
		}

		curArbDataPtr = EsifArbEntry_GetArbitratedRequestData_Locked(self);

		/*
		 * If the arbritrated value has change, execute the primtive with the request data
		 *
		 * WARNING: Must use clone of data buffer as pointer
		 * may be invalid as soon as lock released.
		 */
		if (curArbDataPtr && (curArbDataPtr != prevArbDataPtr)) {

			ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Updating arbitrated value after removal of " ESIF_HANDLE_FMT,
				esif_ccb_handle2llu(appHandle));

			EsifArbEntry_QueueLimitedPrimitiveRequest_Locked(self, curArbDataPtr, ESIF_FALSE);
		}

		esif_ccb_write_unlock(&self->entryLock);
	}
}


static void EsifArbEntry_PurgeRequests(
	EsifArbEntry *self
	)
{
	if (self) {
		esif_ccb_write_lock(&self->entryLock);
		EsifArbEntry_PurgeRequests_Locked(self);
		esif_ccb_write_unlock(&self->entryLock);
	}
}


static void EsifArbEntry_PurgeRequests_Locked(
	EsifArbEntry *self
	)
{
	if (self) {
		ESIF_TRACE_ARB_ENTRY(ESIF_TRACELEVEL_DEBUG, "Removing arbitration requests");
		esif_link_list_free_data(&self->arbRequests, (queue_item_destroy_func)EsifArbReq_Destroy);
	}
}


static Bool EsifArbEntry_IsMatchingEntry(
	EsifArbEntry *self,
	UInt32 primitiveId,
	UInt16 domain,
	UInt8 instance
	)
{
	Bool bRet = ESIF_FALSE;

	ESIF_ASSERT(self);

	if (self &&
		(primitiveId == self->primitiveId) &&
		(domain == self->domain) &&
		(instance == self->instance)) {
		bRet = ESIF_TRUE;
	}
	return bRet;
}


/******************************************************************************
*******************************************************************************
*
* EsifArbReq Functions
*
*******************************************************************************
******************************************************************************/

static EsifArbReq *EsifArbReq_Create(
	esif_handle_t appHandle,
	const EsifData *reqPtr
	)
{
	EsifArbReq *self = NULL;

	self = (EsifArbReq *)esif_ccb_malloc(sizeof(*self));
	if (self) {
		self->appHandle = appHandle;
		self->reqPtr = EsifData_Clone((EsifData *)reqPtr);
		if (NULL == self->reqPtr) {
			EsifArbReq_Destroy(self);
			self = NULL;
		}
	}
	else {
		ESIF_TRACE_ERROR("Allocation failure\n");
	}
	return self;
}


static void EsifArbReq_Destroy(
	EsifArbReq *self
	)
{
	if (self) {
		/* Release data clone */
		EsifData_Destroy(self->reqPtr);
	}
	esif_ccb_free(self);
}


static esif_handle_t EsifArbReq_GetAppHandle(
	EsifArbReq *self
	)
{
	return (self) ? self->appHandle : ESIF_INVALID_HANDLE;
}


static EsifData *EsifArbReq_GetRequestData(EsifArbReq *self)
{
	return (self) ? self->reqPtr : NULL;
}



/******************************************************************************
*******************************************************************************
*
* EsifArbPrimReq Functions
*
*******************************************************************************
******************************************************************************/

static EsifArbPrimReq *EsifArbPrimReq_Create(
	const esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr,
	esif_ccb_event_t *completionEventPtr,
	esif_error_t *completionStatusPtr,
	Bool requiresDelay
	)
{
	EsifArbPrimReq *self = NULL;

	self = (EsifArbPrimReq *)esif_ccb_malloc(sizeof(*self));
	if (self) {
		self->participantId = participantId;
		self->primitiveId = primitiveId;
		self->domain = domain;
		self->instance = instance;
		self->rspDataPtr = responsePtr;
		self->completionStatusPtr = completionStatusPtr;
		self->completionEventPtr = completionEventPtr;
		self->requiresDelay = requiresDelay;

		if (requestPtr) {
			self->reqDataPtr = EsifData_Clone(requestPtr);

			if (requiresDelay) {
				self->delayedPrimitiveId = esif_primitive_type_set2get(primitiveId);
			}
			if (NULL == self->reqDataPtr) {
				EsifArbPrimReq_Destroy(self);
				self = NULL;
			}
			else {
				self->reqDataRequiresRelease = ESIF_TRUE;
			}
		}
	}
	else {
		ESIF_TRACE_ERROR("Allocation failure\n");
	}
	return self;
}


static void EsifArbPrimReq_Destroy(EsifArbPrimReq *self)
{
	if (self) {
		if (!self->isDummyRequest) {
			if (self->reqDataRequiresRelease) {
				EsifData_Destroy(self->reqDataPtr);
			}
			if (self->completionEventPtr) {
				esif_ccb_event_set(self->completionEventPtr);
			}
		}
		esif_ccb_free(self);
	}
}


/******************************************************************************
*******************************************************************************
*
* Arbitration Functions
*
*******************************************************************************
******************************************************************************/

static esif_error_t EsifArbFunction_UInt32_GreaterThan(
	EsifArbReq *req1Ptr,
	Bool *isValid1Ptr,
	EsifArbReq *req2Ptr,
	Bool *isValid2Ptr,
	int *resultPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	esif_error_t req1Rc = ESIF_E_PARAMETER_IS_NULL;
	esif_error_t req2Rc = ESIF_E_PARAMETER_IS_NULL;
	EsifData *data1Ptr = 0;
	EsifData *data2Ptr = 0;
	Bool isValid1 = ESIF_FALSE;
	Bool isValid2 = ESIF_FALSE;
	UInt32 data1 = 0;
	UInt32 data2 = 0;
	int arbResult = 0;

	if (req1Ptr && req1Ptr->reqPtr && req1Ptr->reqPtr->buf_ptr) {
		data1Ptr = req1Ptr->reqPtr;
		req1Rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;

		if (data1Ptr->buf_len >= sizeof(data1)) {
			data1 = *((UInt32 *)(data1Ptr->buf_ptr));
			isValid1 = ESIF_TRUE;
			req1Rc = ESIF_OK;
		}
	}

	if (req2Ptr && req2Ptr->reqPtr && req2Ptr->reqPtr->buf_ptr) {
		data2Ptr = req2Ptr->reqPtr;
		req1Rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;

		if (data2Ptr->buf_len >= sizeof(data2)) {
			data2 = *((UInt32 *)(data2Ptr->buf_ptr));
			isValid2 = ESIF_TRUE;
			req2Rc = ESIF_OK;
		}
	}

	rc = req1Rc | req2Rc;

	if (isValid1 && isValid2) {
		if (data1 > data2) {
			arbResult = 1;
		}
		else if (data1 < data2) {
			arbResult = -1;
		}
		rc = ESIF_OK;
	}

	if (resultPtr) {
		*resultPtr = arbResult;
	}
	if (isValid1Ptr) {
		*isValid1Ptr = isValid1;
	}
	if (isValid2Ptr) {
		*isValid2Ptr = isValid2;
	}
	ESIF_TRACE_DEBUG("Arbitration result = %d, Data1 = %u, isValid1 = %d, Data2 = %u, isValid2 = %d, rc = %d",
		arbResult, data1, isValid1, data2, isValid2, rc);
	return rc;
}


static esif_error_t EsifArbFunction_UInt32_LessThan(
	EsifArbReq *req1Ptr,
	Bool *isValid1Ptr,
	EsifArbReq *req2Ptr,
	Bool *isValid2Ptr,
	int *resultPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	esif_error_t req1Rc = ESIF_E_PARAMETER_IS_NULL;
	esif_error_t req2Rc = ESIF_E_PARAMETER_IS_NULL;
	EsifData *data1Ptr = 0;
	EsifData *data2Ptr = 0;
	Bool isValid1 = ESIF_FALSE;
	Bool isValid2 = ESIF_FALSE;
	UInt32 data1 = 0;
	UInt32 data2 = 0;
	int arbResult = 0;

	if (req1Ptr && req1Ptr->reqPtr && req1Ptr->reqPtr->buf_ptr) {
		data1Ptr = req1Ptr->reqPtr;
		req1Rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;

		if (data1Ptr->buf_len >= sizeof(data1)) {
			data1 = *((UInt32 *)(data1Ptr->buf_ptr));
			isValid1 = ESIF_TRUE;
			req1Rc = ESIF_OK;
		}
	}

	if (req2Ptr && req2Ptr->reqPtr && req2Ptr->reqPtr->buf_ptr) {
		data2Ptr = req2Ptr->reqPtr;
		req1Rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;

		if (data2Ptr->buf_len >= sizeof(data2)) {
			data2 = *((UInt32 *)(data2Ptr->buf_ptr));
			isValid2 = ESIF_TRUE;
			req2Rc = ESIF_OK;
		}
	}

	rc = req1Rc | req2Rc;

	if (isValid1 && isValid2) {
		if (data1 < data2) {
			arbResult = 1;
		}
		else if (data1 > data2) {
			arbResult = -1;
		}
		rc = ESIF_OK;
	}

	if (resultPtr) {
		*resultPtr = arbResult;
	}
	if (isValid1Ptr) {
		*isValid1Ptr = isValid1;
	}
	if (isValid2Ptr) {
		*isValid2Ptr = isValid2;
	}
	ESIF_TRACE_DEBUG("Arbitration result = %d, Data1 = %u, isValid1 = %d, Data2 = %u, isValid2 = %d, rc = %d",
		arbResult, data1, isValid1, data2, isValid2, rc);
	return rc;
}


static EsifArbMgr_ArbitratorFunc GetArbitrationFunction(
	esif_arbitration_type_t funcType
)
{
	EsifArbMgr_ArbitratorFunc arbFunc = NULL;

	if (funcType < (sizeof(g_arbitrationFunctions) / sizeof(*g_arbitrationFunctions))) {
		arbFunc = g_arbitrationFunctions[funcType];
	}
	return arbFunc;
}


static Bool IsPrimitiveSupported(
	esif_handle_t participantId,
	const UInt32 primitiveId,
	const UInt16 domain,
	const UInt8 instance
	)
{
	Bool bRet = ESIF_FALSE;
	EsifUp *upPtr = NULL;
	EsifDsp *dspPtr = NULL;
	EsifFpcPrimitive *primitivePtr = NULL;
	EsifPrimitiveTuple tuple = {
		.id = (u16)primitiveId,
		.domain = domain,
		.instance = instance
	};

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);

	if (upPtr) {
		dspPtr = EsifUp_GetDsp(upPtr);
		if (dspPtr) {
			primitivePtr = dspPtr->get_primitive(dspPtr, &tuple);
			if (primitivePtr) {
				if ((ESIF_PRIMITIVE_OP_SET == (enum esif_primitive_opcode)primitivePtr->operation) &&
					(ESIF_ARB_DATA_SIZE == esif_data_type_sizeof(primitivePtr->request_type))) {
					bRet = ESIF_TRUE;
				}
				else {
					ESIF_TRACE_DEBUG("[%s] Primitive unsupported: opcode = %s, request_type = %s\n",
						EsifUp_GetName(upPtr),
						esif_primitive_opcode_str(primitivePtr->operation),
						esif_data_type_str(primitivePtr->request_type));
				}
			}
			else {
				ESIF_TRACE_DEBUG("[%s] Primitive not in DSP\n", EsifUp_GetName(upPtr));
			}
		}
		else {
			ESIF_TRACE_DEBUG("[%s] No DSP", EsifUp_GetName(upPtr));
		}
	}

	EsifUp_PutRef(upPtr);
	return bRet;
}

void GenericLLDestroyCallback(void *ptr) { esif_ccb_free(ptr); }


#endif /* ESIF_FEAT_OPT_ARBITRATOR_ENABLED */
