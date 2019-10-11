/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "esif_sdk.h"
#include "esif_uf_fpc.h"
#include "esif_event.h"
#include "esif_domain.h"

#define ESIF_DOMAIN_STATE_INVALID 0xffffffff
#define ESIF_DOMAIN_TEMP_INVALID_VALUE 0xFF /* Celsius */
#define ESIF_DOMAIN_TEMP_INVALID_POLL_PERIOD 5000 /* ms */


#define UP_DOMAIN_ITERATOR_MARKER 'UPDM'

typedef enum EsifDomainAuxId_e {
	ESIF_DOMAIN_AUX0 = 0,
	ESIF_DOMAIN_AUX1,
	ESIF_DOMAIN_AUX_MAX
} EsifDomainAuxId, *EsifDomainAuxIdPtr;

typedef enum EsifDomainPollType_e {
	ESIF_POLL_NONE = 0,
	ESIF_POLL_UNSUPPORTED,
	ESIF_POLL_DOMAIN,
	ESIF_POLL_ECONO
} EsifDomainPollTypeId;


struct _t_EsifUp;

typedef struct EsifUpDomain_s {
	UInt16 domain;						/* Domain ID */
	UInt32 domainPriority;				
	char domainGuid[ESIF_GUID_LEN];
	char domainStr[3];
	EsifDomainCapability capability_for_domain;		/* Capabilities */
	enum esif_domain_type domainType;	/* Domain Type */
	char domainName[ESIF_NAME_LEN];
	struct _t_EsifUp *upPtr;			/* Back pointer to the UP*/
	esif_handle_t participantId;
	char participantName[MAX_NAME_STRING_LENGTH];
	esif_ccb_lock_t capsLock;

	/* Temperature detection */
	esif_ccb_lock_t tempLock;

	esif_ccb_timer_t tempPollTimer;		/* Temperature Polling Timer  */
	UInt32 tempPollPeriod;				/* Temperature polling interval: 0-disabled */

	esif_temp_t virtTemp;				/* Virtual Temperature */
	esif_temp_t tempAux0;				/* Lower temperature threshold */
	esif_temp_t tempAux1;				/* Upper temperature threshold */
	esif_temp_t tempAux0WHyst;			/* Lower threshold including hysteresis */
	esif_temp_t	tempHysteresis;			/* Lower hysteresis */
	UInt8 tempNotifySent;				/*
										 * Indicates a threshold crossing event has been sent and another will
										 * not be sent until the temperature is read or the thresholds changed.
										*/
	EsifDomainPollTypeId tempPollType;	/* Single threaded, multi threaded, or none */
	UInt8 tempPollInitialized;			/*
										 * Indicates that the temperature polling timer has been initialized,
										 * since our current timer implementation does not support kill > restart.
										 * This should remain set until the timer object is killed, even if polling 
										 * is suspended.
										 */
	UInt8 tempLastTempValid;			/*
										* Used to determine when threshold crossed events should be sent if
										* the device is no longer providing valid temperatures (so DPTF can
										* unthrottle for example if unable to read device temp)
										*/
	UInt8 tempInvalidValueDetected;		/*
										* Indicates a value of 0xFF was received and we should adjust polling to
										* a longer interval
										*/
	UInt64 lastPower;					/* rapl energy (prior to conversion) */
	esif_ccb_time_t lastPowerTime;		/* time of last power sample in microseconds */
	EsifDomainPollTypeId powerPollType;	/* Single threaded, multi threaded, or none */
	UInt32 lastState;					/* check perf participants for state change */
	/* Perf state detection */
	esif_ccb_lock_t stateLock;
	esif_ccb_timer_t statePollTimer;	/* Timer for polling participant for state change  */
	UInt32 statePollPeriod;				/* Perf state polling interval: 0-disabled */
	EsifDomainPollTypeId statePollType;	/* Single threaded, multi threaded, or none */
	UInt8 statePollInitialized;
} EsifUpDomain, *EsifUpDomainPtr;

typedef struct UpDomainIterator_s {
	u32 marker;
	UInt8 handle;
	struct _t_EsifUp *upPtr;
} UpDomainIterator, *UpDomainIteratorPtr;

#ifdef __cplusplus
extern "C" {
#endif


eEsifError EsifUpDomain_InitDomain(
	EsifUpDomainPtr self,
	struct _t_EsifUp *upPtr,
	struct esif_fpc_domain *fpcDomainPtr
	);

eEsifError EsifUpDomain_InitTempPoll(
	EsifUpDomainPtr self
	);

eEsifError EsifUpDomain_InitPowerPoll(
	EsifUpDomainPtr self
	);


eEsifError EsifUpDomain_DspReadyInit(
	EsifUpDomainPtr self
	);
	
eEsifError EsifDomainIdToIndex(
	UInt16 domain,
	UInt8 *indexPtr
	);

eEsifError EsifUpDomain_EnableCaps(
	EsifUpDomainPtr self,
	unsigned int capabilityFlags,
	unsigned char *capabilityMask
	);

void EsifUpDomain_StopTempPoll(
	EsifUpDomainPtr self
	);

eEsifError EsifUpDomain_Poll(EsifUpDomainPtr self);

eEsifError EsifUpDomain_CheckTemp(EsifUpDomainPtr self);

eEsifError EsifUpDomain_CheckState(EsifUpDomainPtr self);

void EsifUpDomain_RegisterForTempPoll(EsifUpDomainPtr self, EsifDomainPollTypeId pollType);

void EsifUpDomain_UnRegisterForTempPoll(EsifUpDomainPtr self);

void EsifUpDomain_RegisterForStatePoll(EsifUpDomainPtr self, EsifDomainPollTypeId pollType);

void EsifUpDomain_UnRegisterForStatePoll(EsifUpDomainPtr self);

void EsifUpDomain_UnInitDomain(EsifUpDomainPtr self);

eEsifError EsifUpDomain_SetTempThresh(
	EsifUpDomainPtr self,
enum EsifDomainAuxId_e threshId,
	u32 threshold
	);

eEsifError EsifUpDomain_SetTempPollPeriod(
	EsifUpDomainPtr self,
	UInt32 sampleTime
	);

eEsifError EsifUpDomain_SetStatePollPeriod(
	EsifUpDomainPtr self,
	UInt32 sampleTime
	);

void EsifUpDomain_SetVirtualTemperature(
	EsifUpDomainPtr self,
	UInt32 virtTemp
	);

eEsifError EsifUpDomain_SetTempHysteresis(
	EsifUpDomainPtr self,
	esif_temp_t tempHysteresis
	);

eEsifError EsifUpDomain_SignalOSEvent(
	EsifUpDomainPtr self,
	UInt32 updatedValue,
	eEsifEventType eventType
	);

eEsifError EsifUpDomain_SignalForegroundAppChanged(
	EsifUpDomainPtr self,
	EsifString appName
	);

/*
* Used to iterate through the available domains.
* First call EsifUpDomain_InitIterator to initialize the iterator.
* Next, call EsifUpDomain_GetNextUd using the iterator.  Repeat until
* EsifUpDomain_GetNextUd fails. The call will release the reference of the
* associated upper participant.  If you stop iteration part way through
* all domains of a particular participant, the caller is responsible for
* releasing the reference on the associated upper participant.  Iteration
* is complete when ESIF_E_ITERATOR_DONE is returned.
*/
eEsifError EsifUpDomain_InitIterator(
	UpDomainIteratorPtr iteratorPtr,
	struct _t_EsifUp *upPtr
	);

/* See EsifUpDomain_InitIterator for usage */
eEsifError EsifUpDomain_GetNextUd(
	UpDomainIteratorPtr iteratorPtr,
	EsifUpDomainPtr *upDomainPtr
	);

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
