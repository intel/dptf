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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_DOMAIN

#include "esif_uf.h"	/* Upper Framework */
#include "esif_uf_domain.h"
#include "esif_dsp.h"		/* Device Support Package    */
#include "esif_participant.h"
#include "esif_event.h"
#include "esif_uf_eventmgr.h"
#include "esif_link_list.h"
#include "esif_uf_primitive.h"
#include "esif_temp.h"
#include "esif_pm.h"		/* Upper Participant Manager */


#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

static Bool EsifUpDomain_IsTempOutOfThresholds(
	EsifUpDomainPtr self,
	UInt32 temp
	);

static Bool EsifUpDomain_AnyTempThresholdValid(
	EsifUpDomainPtr self
	);

static eEsifError EsifUpDomain_TempDetectInit(
	EsifUpDomainPtr self
	);

static eEsifError EsifUpDomain_PowerDetectInit(
	EsifUpDomainPtr self
	);

static eEsifError EsifUpDomain_StateDetectInit(
	EsifUpDomainPtr self
	);

static eEsifError EsifUpDomain_SetTempThreshWLock(
	EsifUpDomainPtr self,
	EsifDomainAuxId auxId,
	UInt32 threshold
	);

static esif_temp_t EsifUpDomain_CalcAux0WHyst(
	EsifUpDomainPtr self,
	esif_temp_t aux0
	);

static eEsifError EsifUpDomain_StartTempPollPriv(
	EsifUpDomainPtr self
	);

static void EsifUpDomain_PollTemp(
	const void *ctx
	);

eEsifError EsifUpDomain_InitDomain(
	EsifUpDomainPtr self,
	EsifUpPtr upPtr,
	struct esif_fpc_domain *fpcDomainPtr
	)
{
	eEsifError rc = ESIF_OK;
	char domainBuf[3];

	if ((NULL == self) ||
		(NULL == upPtr) ||
		(NULL == fpcDomainPtr)) {
		ESIF_TRACE_DEBUG("Parameter is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	self->domain = fpcDomainPtr->descriptor.domain;

	esif_ccb_strcpy(self->domainStr,
		esif_primitive_domain_str(self->domain, domainBuf, sizeof(domainBuf)),
		sizeof(self->domainStr));

	self->domainType = fpcDomainPtr->descriptor.domainType;
	self->capabilities = fpcDomainPtr->capability_for_domain.capability_flags;
	esif_ccb_strcpy(self->domainName, fpcDomainPtr->descriptor.name, sizeof(self->domainName));	

	self->upPtr = upPtr;
	self->participantId = upPtr->fInstance;
	esif_ccb_strcpy(self->participantName, EsifUp_GetName(upPtr), sizeof(self->participantName));

	esif_ccb_lock_init(&self->tempLock);

exit:
	return rc;
}


eEsifError EsifUpDomain_DspReadyInit(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == self) {
		ESIF_TRACE_DEBUG("Self is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	
	rc = EsifUpDomain_TempDetectInit(self);
	if ((rc != ESIF_OK)  && (rc != ESIF_E_NOT_SUPPORTED)) {
		goto exit;
	}
	
	rc = EsifUpDomain_PowerDetectInit(self);
	if ((rc != ESIF_OK) && (rc != ESIF_E_NOT_SUPPORTED)) {
		goto exit;
	}
	
	rc = EsifUpDomain_StateDetectInit(self);
	if ((rc != ESIF_OK) && (rc != ESIF_E_NOT_SUPPORTED)) {
		goto exit;
	}
exit:
	return rc;
}


static eEsifError EsifUpDomain_TempDetectInit(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 temperatureValue = ESIF_DOMAIN_TEMP_INVALID;
	EsifPrimitiveTuple hystTuple = {GET_TEMPERATURE_THRESHOLD_HYSTERESIS, 0, 255};
	EsifData hystData = {ESIF_DATA_TEMPERATURE, NULL, sizeof(self->tempHysteresis), 0};
	EsifPrimitiveTuple tempTuple = {GET_TEMPERATURE, 0, 255};
	EsifData tempData = { ESIF_DATA_TEMPERATURE, &temperatureValue, sizeof(temperatureValue), 0 };

	ESIF_ASSERT(self != NULL);

	if (!(self->capabilities & ESIF_CAPABILITY_TEMP_THRESHOLD)) {
		ESIF_TRACE_DEBUG("Capability is not supported\n");
		rc = ESIF_E_NOT_SUPPORTED;
		self->tempPollType = ESIF_POLL_UNSUPPORTED;
		goto exit;
	}

	/* Validate the interface */
	tempTuple.domain = self->domain;
	rc = EsifUp_ExecutePrimitive(self->upPtr, &tempTuple, NULL, &tempData);
	if ((rc != ESIF_OK) && (rc != ESIF_I_AGAIN)) {
		ESIF_TRACE_DEBUG("%s %s: Error getting temperature, resetting capabilities to reflect this. \n",
			self->participantName,
			self->domainStr);
		self->capabilities = self->capabilities & ~ESIF_CAPABILITY_TEMP_THRESHOLD;
		self->tempPollType = ESIF_POLL_UNSUPPORTED;
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	hystTuple.domain = self->domain;
	hystData.buf_ptr = &self->tempHysteresis;
	rc = EsifUp_ExecutePrimitive(self->upPtr, &hystTuple, NULL, &hystData);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("%s %s: Error getting hysteresis\n",
			self->participantName,
			self->domainStr);
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}
	ESIF_TRACE_DEBUG("%s %s: Setting hysteresis to %d\n",
		self->participantName,
		self->domainStr,
		self->tempHysteresis);

	self->tempAux0 = ESIF_DOMAIN_TEMP_INVALID;
	self->tempAux1 = ESIF_DOMAIN_TEMP_INVALID;
exit:
	return rc;
}

static eEsifError EsifUpDomain_PowerDetectInit(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 powerValue = 0;
	EsifPrimitiveTuple powerTuple = {GET_RAPL_POWER, 0, 255};
	EsifData powerData = { ESIF_DATA_POWER, &powerValue, sizeof(powerValue), 0 };

	ESIF_ASSERT(self != NULL);

	if (!(self->capabilities & ESIF_CAPABILITY_POWER_STATUS)) {
		ESIF_TRACE_DEBUG("Capability is not supported\n");
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	powerTuple.domain = self->domain;

	rc = EsifUp_ExecutePrimitive(self->upPtr, &powerTuple, NULL, &powerData);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("%s %s: Error getting power. \n",
			self->participantName,
			self->domainStr);
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

exit:
	return rc;
}

static eEsifError EsifUpDomain_StateDetectInit(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 stateValue = ESIF_DOMAIN_STATE_INVALID;
	EsifPrimitiveTuple stateTuple = {GET_PARTICIPANT_PERF_PRESENT_CAPABILITY, 0, 255};
	EsifData stateData = { ESIF_DATA_TEMPERATURE, &stateValue, sizeof(stateValue), 0 };

	ESIF_ASSERT(self != NULL);

	self->lastState = ESIF_DOMAIN_STATE_INVALID;
	
	if (!(self->capabilities & ESIF_CAPABILITY_PERF_CONTROL)) {
		ESIF_TRACE_DEBUG("Capability is not supported\n");
		rc = ESIF_E_NOT_SUPPORTED;
		self->statePollType = ESIF_POLL_UNSUPPORTED;
		goto exit;
	}

	stateTuple.domain = self->domain;
	rc = EsifUp_ExecutePrimitive(self->upPtr, &stateTuple, NULL, &stateData);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("%s %s: Error getting current performance state. \n",
			self->participantName,
			self->domainStr);
		self->statePollType = ESIF_POLL_UNSUPPORTED;
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	self->lastState = stateValue;
exit:
	return rc;
}


static eEsifError EsifUpDomain_SetTempThreshWLock(
	EsifUpDomainPtr self,
	EsifDomainAuxId auxId,
	UInt32 threshold
	)
{
	eEsifError rc = ESIF_OK;
	esif_temp_t tempMin = ESIF_DOMAIN_TEMP_MIN;
	esif_temp_t tempMax = ESIF_DOMAIN_TEMP_MAX;
	esif_temp_t tempAux0Def = ESIF_DOMAIN_TEMP_AUX0_DEF;
	esif_temp_t tempAux1Def;
	esif_temp_t temp;
	EsifPrimitiveTuple auxTuple = {SET_TEMPERATURE_THRESHOLDS_SUR, 0, auxId};
	EsifData auxData = {ESIF_DATA_TEMPERATURE, &temp, sizeof(UInt32), 0};

	ESIF_ASSERT(self != NULL);

	/* Normalize default temperatures */
	esif_convert_temp(ESIF_TEMP_C, NORMALIZE_TEMP_TYPE, &tempAux0Def);
	esif_convert_temp(ESIF_TEMP_C, NORMALIZE_TEMP_TYPE, &tempMin);
	esif_convert_temp(ESIF_TEMP_C, NORMALIZE_TEMP_TYPE, &tempMax);
	tempAux1Def = tempMax;
	temp = threshold;

	switch (auxId) {
	case ESIF_DOMAIN_AUX0:

		/* If threshold is not set, use default */
		if (threshold == ESIF_DOMAIN_TEMP_INVALID)
			temp = tempAux0Def;

		/* Else clamp if needed */
		else if (threshold > tempMax)
			temp = tempMax;
		else if (threshold < tempMin)
			temp = tempMin;

		self->tempAux0 = temp;
		if (threshold == ESIF_DOMAIN_TEMP_INVALID)
			self->tempAux0 = ESIF_DOMAIN_TEMP_INVALID;

		self->tempAux0WHyst = EsifUpDomain_CalcAux0WHyst(
			self,
			self->tempAux0);

		/*
		 * If not polling, we must apply hysteresis.  If not, then we are using ACPI/EC and BIOS
		 * applies hysteresis; so we do not apply it in that case.
		 */
		if (self->tempPollPeriod == 0) {
			temp = self->tempAux0WHyst;
		}
		break;

	case ESIF_DOMAIN_AUX1:

		/* If threshold is not set, use default */
		if (threshold == ESIF_DOMAIN_TEMP_INVALID)
			temp = tempAux1Def;

		/* Else clamp if needed */
		else if (threshold > tempMax)
			temp = tempMax;
		else if (threshold < tempMin)
			temp = tempMin;

		self->tempAux1 = temp;
		if (threshold == ESIF_DOMAIN_TEMP_INVALID)
			self->tempAux1 = ESIF_DOMAIN_TEMP_INVALID;
		break;
	default:
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
		break;
	}

	ESIF_TRACE_DEBUG(
		"%s %s: Set temp Thresh %d to %d (actual = %d); Aux1 = %d, Aux0 = %d, Aux0WHyst = %d %s(%d)\n",
		self->participantName,
		self->domainStr,
		auxId,
		threshold,
		temp,
		self->tempAux1,
		self->tempAux0,
		self->tempAux0WHyst,
		esif_temperature_type_desc(NORMALIZE_TEMP_TYPE),
		NORMALIZE_TEMP_TYPE);

	/*
	 * If not polling, set the AUXs - This may fail, as not
	 * all participants will have surrogates.
	 */

	if (self->tempPollPeriod == 0) {
		auxTuple.domain = self->domain;
		rc = EsifUp_ExecutePrimitive(self->upPtr, &auxTuple, &auxData, NULL);
	}

exit:
	return rc;
}

	
static esif_temp_t EsifUpDomain_CalcAux0WHyst(
	EsifUpDomainPtr self,
	esif_temp_t aux0
	)
{
	esif_temp_t aux0WHyst = ESIF_DOMAIN_TEMP_MIN;
	esif_temp_t tempAux0Def = ESIF_DOMAIN_TEMP_AUX0_DEF;

	esif_convert_temp(ESIF_TEMP_C, NORMALIZE_TEMP_TYPE, &aux0WHyst);

	if (ESIF_DOMAIN_TEMP_INVALID == aux0) {
		esif_convert_temp(ESIF_TEMP_C, NORMALIZE_TEMP_TYPE, &tempAux0Def);
		aux0 = tempAux0Def;
	}

	if (self->tempHysteresis < (aux0 - 1))
		aux0WHyst = aux0 - self->tempHysteresis - 1;

	return aux0WHyst;
}


eEsifError EsifDomainIdToIndex(
	UInt16 domain,
	UInt8 *indexPtr
	)
{
	eEsifError rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	u8 indexVal;

	if ((domain & 0xFF) != 'D')
		goto exit;

	indexVal = (UInt8)(domain >> 8);
	if ((indexVal < '0') || ('9' < indexVal))
		goto exit;

	indexVal &= 0x0F;

	*indexPtr = indexVal;
	rc = ESIF_OK;
exit:
	return rc;
}

eEsifError EsifUpDomain_SetTempThresh(
	EsifUpDomainPtr self,
	enum EsifDomainAuxId_e threshId,
	u32 threshold
	)
{
	enum esif_rc rc = ESIF_OK;

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (!(self->capabilities & ESIF_CAPABILITY_TEMP_THRESHOLD)) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	esif_ccb_write_lock(&self->tempLock);
	rc = EsifUpDomain_SetTempThreshWLock(self, threshId, threshold);
	if (ESIF_OK != rc)
		goto lockExit;

	/*
	* Reset the threshold event sent flag if temp  is read or the
	* thresholds are changed.
	*/
	self->tempNotifySent = ESIF_FALSE;

	/*
	* If period is valid, enable/continue polling.
	*/
	if (self->tempPollPeriod != 0) {
		if (self->tempPollInitialized == ESIF_TRUE) {
			rc = esif_ccb_timer_set_msec(&self->tempPollTimer,
				self->tempPollPeriod);
		}
		else {
			rc = EsifUpDomain_StartTempPollPriv(self);
		}
		
		goto lockExit;
	}


lockExit:
	esif_ccb_write_unlock(&self->tempLock);
exit:
	return rc;
}

eEsifError EsifUpDomain_Poll(EsifUpDomainPtr self)
{
	eEsifError rc = ESIF_OK;
	/* 
	 * Skip participants that don't support econo type, or are being monitored in another way.
	 * This is a single threaded poll method - all polling happens in the same process.
	 * To come: poll power when applicable 
	 */
	if (self->tempPollType == ESIF_POLL_ECONO) {
		rc = EsifUpDomain_CheckTemp(self);	
	}
	
	if (self->statePollType == ESIF_POLL_ECONO) { 
		rc = EsifUpDomain_CheckState(self);
	}

	return rc;
}

eEsifError EsifUpDomain_CheckTemp(EsifUpDomainPtr self)
{
	eEsifError rc = ESIF_OK;
	UInt32 temp = ESIF_DOMAIN_TEMP_INVALID;
	EsifPrimitiveTuple tempTuple = {GET_TEMPERATURE, 0, 255};
	struct esif_data tempResponse = { ESIF_DATA_TEMPERATURE, &temp, sizeof(temp), 0 };

	tempTuple.domain = self->domain;
	rc = EsifUp_ExecutePrimitive(self->upPtr, &tempTuple, NULL, &tempResponse);
	if (rc != ESIF_OK) {
		if (rc == ESIF_E_STOP_POLL) {
			self->tempPollType = ESIF_POLL_UNSUPPORTED;
		}
		goto exit;
	}
	
	if (EsifUpDomain_IsTempOutOfThresholds(self, temp)) {
		EsifEventMgr_SignalEvent(self->participantId, self->domain, ESIF_EVENT_DOMAIN_TEMP_THRESHOLD_CROSSED, NULL);
		ESIF_TRACE_DEBUG("THRESHOLD CROSSED EVENT!!! Participant: %s, Domain: %s, Temperature: %d \n", self->participantName, self->domainName, temp);
	}

exit:
	return rc;
}

eEsifError EsifUpDomain_CheckState(EsifUpDomainPtr self)
{
	eEsifError rc = ESIF_OK;
	UInt32 state = ESIF_DOMAIN_STATE_INVALID;
	EsifPrimitiveTuple stateTuple = {GET_PARTICIPANT_PERF_PRESENT_CAPABILITY, 0, 255};
	struct esif_data stateResponse = { ESIF_DATA_UINT32, &state, sizeof(state), 0 };

	stateTuple.domain = self->domain;
	rc = EsifUp_ExecutePrimitive(self->upPtr, &stateTuple, NULL, &stateResponse);
	if (rc != ESIF_OK) {
		goto exit;
	}
	
	if (self->lastState != state && state != ESIF_DOMAIN_STATE_INVALID) {
		EsifEventMgr_SignalEvent(self->participantId, self->domain, ESIF_EVENT_DOMAIN_PERF_CAPABILITY_CHANGED, NULL);
		ESIF_TRACE_DEBUG("PERF STATE CHANGED! Participant: %s, Domain: %s, State: %d \n", self->participantName, self->domainName, state);
		self->lastState = state;
	}
exit:
	return rc;
}

static void EsifUpDomain_PollTemp(
	const void *ctx
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpDomainPtr self = (EsifUpDomainPtr) ctx;
	EsifDspPtr dspPtr = NULL;
	EsifUpPtr upPtr = NULL;
	
	if (self == NULL) {
		goto exit;
	}

	
	/* check to see if anyone has killed the action manager or dsp manager
	in between polls */
	upPtr = EsifUpPm_GetAvailableParticipantByInstance(self->participantId);
	if (upPtr == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	if (upPtr->fDspPtr == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	dspPtr = upPtr->fDspPtr;
	if (dspPtr == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	if (dspPtr->type == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}
	
	rc = EsifUpDomain_CheckTemp(self);
	
	/* If another type of polling was started, skip the timer reset */
	if (self->tempPollType != ESIF_POLL_DOMAIN) {
		goto exit;
	}


	if (self->tempPollPeriod > 0 && EsifUpDomain_AnyTempThresholdValid(self)) {
		if (self->tempPollInitialized == ESIF_TRUE) {
			rc = esif_ccb_timer_set_msec(&self->tempPollTimer,
				self->tempPollPeriod);
		}
		else {
			rc = EsifUpDomain_StartTempPollPriv(self);
		}
	}
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}

	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Unable to set poll timer on %s %s : %s(%d)\n", self->participantName, self->domainName, esif_rc_str(rc), rc);
	}
}

static eEsifError EsifUpDomain_StartTempPollPriv(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	/* Skip if another type of polling is started */
	if (self->tempPollType != ESIF_POLL_NONE) {
		goto exit;
	}

	ESIF_TRACE_DEBUG(
		"%s %s: Starting temp polling\n",
		self->participantName,
		self->domainName);

	rc = esif_ccb_timer_init(&self->tempPollTimer, (esif_ccb_timer_cb)EsifUpDomain_PollTemp, self);
	if (ESIF_OK != rc)
		goto exit;
		
	self->tempPollInitialized = ESIF_TRUE;
	self->tempPollType = ESIF_POLL_DOMAIN;
	rc = esif_ccb_timer_set_msec(&self->tempPollTimer,
		self->tempPollPeriod);

exit:
	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("Error with setting uf poll timer: %s(%d)\n", esif_rc_str(rc), rc);
	}
	return rc;
}

static Bool EsifUpDomain_IsTempOutOfThresholds(
	EsifUpDomainPtr self,
	UInt32 temp
	)
{
	ESIF_ASSERT(self != NULL);

	return (((self->tempAux0 != ESIF_DOMAIN_TEMP_INVALID) &&
		(temp <= self->tempAux0WHyst)) ||
		((self->tempAux1 != ESIF_DOMAIN_TEMP_INVALID) &&
		(temp >= self->tempAux1)) &&
		(temp != ESIF_DOMAIN_TEMP_INVALID));
}

static Bool EsifUpDomain_AnyTempThresholdValid(
	EsifUpDomainPtr self
	)
{
	Bool isValid = ESIF_FALSE;

	ESIF_ASSERT(self != NULL);

	if ((self->tempAux0 != ESIF_DOMAIN_TEMP_INVALID) ||
		(self->tempAux1 != ESIF_DOMAIN_TEMP_INVALID)) {
		isValid = ESIF_TRUE;
	}

	return isValid;
}

void EsifUpDomain_RegisterForTempPoll(EsifUpDomainPtr self, EsifDomainPollTypeId pollType)
{
	if (self->tempPollType != ESIF_POLL_UNSUPPORTED) {
		self->tempPollType = pollType;
	}
}

void EsifUpDomain_UnRegisterForTempPoll(EsifUpDomainPtr self)
{
	self->tempPollType = ESIF_POLL_NONE;
}

void EsifUpDomain_RegisterForStatePoll(EsifUpDomainPtr self, EsifDomainPollTypeId pollType)
{
	if (self->statePollType != ESIF_POLL_UNSUPPORTED) {
		self->statePollType = pollType;
	}
}

void EsifUpDomain_UnRegisterForStatePoll(EsifUpDomainPtr self)
{
	self->statePollType = ESIF_POLL_NONE;
}

void EsifUpDomain_StopTempPoll(
	EsifUpDomainPtr self
	)
{
	esif_ccb_timer_kill_w_wait(&self->tempPollTimer);

	self->tempPollInitialized = ESIF_FALSE;
	self->tempPollType = ESIF_POLL_NONE;
}

eEsifError EsifUpDomain_SetTempPollPeriod(
	EsifUpDomainPtr self,
	UInt32 sampleTime
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);
	
	self->tempPollPeriod = sampleTime;
	if (sampleTime > 0) {
		if (self->tempPollInitialized == ESIF_TRUE) {
			rc = esif_ccb_timer_set_msec(&self->tempPollTimer,
				self->tempPollPeriod);
		}
		else {
			rc = EsifUpDomain_StartTempPollPriv(self);
		}
	}

	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("Error with setting uf poll timer: %s(%d)\n", esif_rc_str(rc), rc);
	}
	return rc;
}

void EsifUpDomain_SetVirtualTemperature(
	EsifUpDomainPtr self,
	UInt32 virtTemp
	)
{
	ESIF_ASSERT(self != NULL);

	self->virtTemp = virtTemp;
	if (EsifUpDomain_IsTempOutOfThresholds(self, virtTemp)) {
		EsifEventMgr_SignalEvent(self->participantId, self->domain, ESIF_EVENT_DOMAIN_TEMP_THRESHOLD_CROSSED, NULL);
		ESIF_TRACE_DEBUG("THRESHOLD CROSSED EVENT!!! Participant: %s, Domain: %s, Temperature: %d \n", self->participantName, self->domainName, virtTemp);
	}

	return;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
