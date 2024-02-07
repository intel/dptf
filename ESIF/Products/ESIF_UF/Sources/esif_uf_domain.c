/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "esif_uf_eventmgr.h"
#include "esif_link_list.h"
#include "esif_uf_primitive.h"
#include "esif_temp.h"
#include "esif_pm.h"		/* Upper Participant Manager */
#include "esif_lib_esifdata.h"



#define PERF_STATE_POLL_PERIOD 3000  /* msec to poll perf state (will detect AC/DC change) */

static Bool EsifUpDomain_IsTempOutOfThresholds(
	EsifUpDomainPtr self,
	UInt32 temp
	);

static Bool EsifUpDomain_TempIsInThresholds(
	EsifUpDomainPtr self,
	UInt32 temp
	);

static Bool EsifUpDomain_AnyTempThresholdValid(
	EsifUpDomainPtr self
	);

static eEsifError EsifUpDomain_CapDetect(
	EsifUpDomainPtr self,
	enum esif_capability_type cap,
	EsifPrimitiveTuplePtr tuplePtr,
	EsifDataPtr dataPtr
	);

static void EsifUpDomain_DisableCap(
	EsifUpDomainPtr self,
	enum esif_capability_type cap
	);

static eEsifError EsifUpDomain_TempDetectInit(
	EsifUpDomainPtr self
	);

static eEsifError EsifUpDomain_PowerStatusDetectInit(
	EsifUpDomainPtr self
	);

static eEsifError EsifUpDomain_PowerControlDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_ActiveControlDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_DisplayControlDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_PeakPowerControlDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_ProcessorControlDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_PlatformPowerStatusDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_RfProfileStatusDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_RfProfileControlDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_EnergyControlDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_ActivityStatusDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_BatteryStatusDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_WorkloadClassificationDetectInit(
	EsifUpDomainPtr self
);

static eEsifError EsifUpDomain_PsysDetectInit(
	EsifUpDomainPtr self
	);

static eEsifError EsifUpDomain_PerfDetectInit(
	EsifUpDomainPtr self
	);

static eEsifError EsifUpDomain_ProcPerfDetect(
	EsifUpDomainPtr self
	);

static Bool EsifUpDomain_IsHwpCapable(
	EsifUpDomainPtr self
	);

static Bool EsifUpDomain_IsHwpSupported(
	EsifUpDomainPtr self
	);

static Bool EsifUpDomain_IsHwpEnabled(
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

static eEsifError EsifUpDomain_StartStatePollPriv(
	EsifUpDomainPtr self
	);

static void EsifUpDomain_PollTemp(
	const void *ctx
	);

static void EsifUpDomain_PollState(
	const void *ctx
	);

//
// Friend functions
//
void EsifUpDomain_SetUpId(
	EsifUpDomainPtr self,
	esif_handle_t participantId
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
	self->domainPriority = fpcDomainPtr->descriptor.priority;
	esif_ccb_memcpy(self->domainGuid,fpcDomainPtr->descriptor.guid,sizeof(self->domainGuid));
	esif_ccb_strcpy(self->domainStr,
		esif_primitive_domain_str(self->domain, domainBuf, sizeof(domainBuf)),
		sizeof(self->domainStr));

	self->domainType = fpcDomainPtr->descriptor.domainType;
	esif_ccb_memcpy(&self->capability_for_domain, &fpcDomainPtr->capability_for_domain, sizeof(self->capability_for_domain));
	esif_ccb_strcpy(self->domainName, fpcDomainPtr->descriptor.name, sizeof(self->domainName));	

	self->upPtr = upPtr;
	self->participantId = ESIF_INVALID_HANDLE;
	esif_ccb_strcpy(self->participantName, EsifUp_GetName(upPtr), sizeof(self->participantName));

	esif_ccb_lock_init(&self->tempLock);
	esif_ccb_lock_init(&self->capsLock);
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
	}
	
	//multiple rc codes are acceptable for capability detection
	if (rc == ESIF_OK) {
		EsifUpDomain_InitTempPoll(self);
		EsifUpDomain_InitPowerPoll(self);
		rc = EsifUpDomain_TempDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		rc = EsifUpDomain_PowerStatusDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		rc = EsifUpDomain_PowerControlDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		rc = EsifUpDomain_PsysDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		rc = EsifUpDomain_PerfDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		rc = EsifUpDomain_BatteryStatusDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_ActiveControlDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_DisplayControlDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_PeakPowerControlDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_ProcessorControlDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_PlatformPowerStatusDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_RfProfileStatusDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_RfProfileControlDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_EnergyControlDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_ActivityStatusDetectInit(self);
	}
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED)) {
		rc = EsifUpDomain_WorkloadClassificationDetectInit(self);
	}
/* Perf state detection handled in upper framework for Sysfs model */
#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		rc = EsifUpDomain_StateDetectInit(self);
	}
#endif

	if ((rc == ESIF_OK) || (rc == ESIF_E_NOT_SUPPORTED) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		rc = ESIF_OK;
	}

	return rc;
}

static void EsifUpDomain_DisableCap(
	EsifUpDomainPtr self,
	enum esif_capability_type cap
	)
{
	ESIF_ASSERT(self != NULL);

	esif_ccb_write_lock(&self->capsLock);

	if (self->capability_for_domain.capability_flags & (1 << cap)) {
		self->capability_for_domain.capability_flags &= ~(1 << cap);
		self->capability_for_domain.capability_mask[cap] = 0;
		self->capability_for_domain.number_of_capability_flags--;
	}
	
	esif_ccb_write_unlock(&self->capsLock);
}

eEsifError EsifUpDomain_EnableCaps(
	EsifUpDomainPtr self,
	unsigned int capabilityFlags,
	unsigned char *capabilityMaskPtr
	)
{
	eEsifError rc = ESIF_OK;
	u32 i = 0;

	if (self == NULL || capabilityMaskPtr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	
	esif_ccb_write_lock(&self->capsLock);
	
	self->capability_for_domain.capability_flags = capabilityFlags;
	esif_ccb_memcpy(self->capability_for_domain.capability_mask, capabilityMaskPtr, sizeof(self->capability_for_domain.capability_mask));
	self->capability_for_domain.number_of_capability_flags = 0;
	for (i = 0; i < MAX_CAPABILITY_MASK; ++i) {
		if (capabilityFlags & (1 << i)) {
			self->capability_for_domain.number_of_capability_flags++;
		}
	}
	
	esif_ccb_write_unlock(&self->capsLock);
exit:
	return rc;
}

static eEsifError EsifUpDomain_CapDetect(
	EsifUpDomainPtr self,
	enum esif_capability_type cap,
	EsifPrimitiveTuplePtr tuplePtr,
	EsifDataPtr dataPtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(tuplePtr != NULL);
	ESIF_ASSERT(dataPtr != NULL);

	tuplePtr->domain = self->domain;

	if (!(self->capability_for_domain.capability_flags & (1 << cap))) {
		ESIF_TRACE_DEBUG("%s %s: Capability %s is not supported in DSP\n",
			self->participantName,
			self->domainStr,
			esif_capability_type_str(cap));
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}


	/* Validate the capability */
	rc = EsifUp_ExecutePrimitive(self->upPtr, tuplePtr, NULL, dataPtr);
	if ((rc != ESIF_OK) && (rc != ESIF_I_AGAIN) && (rc != ESIF_E_NEED_LARGER_BUFFER)) {
		ESIF_TRACE_DEBUG("%s %s: Error validating capability %s . Return code: %s\n",
			self->participantName,
			self->domainStr,
			esif_capability_type_str(cap),
			esif_rc_str(rc));
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

	self->tempPollType = ESIF_POLL_NONE;

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_TEMP_STATUS, &tempTuple, &tempData);
	if ((rc != ESIF_OK) && (rc != ESIF_I_AGAIN) && (rc != ESIF_E_NEED_LARGER_BUFFER)) {
		self->tempPollType = ESIF_POLL_UNSUPPORTED;
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_TEMP_STATUS);
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD);
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
		self->tempHysteresis = esif_temp_rel_to_abs(ESIF_DOMAIN_HYST_DEF);
		rc = ESIF_OK;
		goto exit;
	}
	ESIF_TRACE_DEBUG("%s %s: Setting hysteresis to %d\n",
		self->participantName,
		self->domainStr,
		esif_temp_abs_to_rel(self->tempHysteresis));

	self->tempAux0 = ESIF_DOMAIN_TEMP_INVALID;
	self->tempAux1 = ESIF_DOMAIN_TEMP_INVALID;
exit:
	return rc;
}

static eEsifError EsifUpDomain_PowerStatusDetectInit(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 powerValue = 0;
	EsifPrimitiveTuple powerTuple = { GET_RAPL_POWER, 0, 255 };
	EsifData powerData = { ESIF_DATA_POWER, &powerValue, sizeof(powerValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_POWER_STATUS, &powerTuple, &powerData);
	if ((rc != ESIF_OK) && (rc != ESIF_I_AGAIN) && (rc != ESIF_E_NEED_LARGER_BUFFER)) {
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_POWER_STATUS);
		rc = ESIF_E_NOT_SUPPORTED;
	}

	return rc;
}

static eEsifError EsifUpDomain_PowerControlDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple powerCapsTuple = { GET_RAPL_POWER_CONTROL_CAPABILITIES, 0, 255 };
	EsifData powerCapsData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_POWER_CONTROL, &powerCapsTuple, &powerCapsData);
	if ((rc != ESIF_OK) && (rc != ESIF_I_AGAIN) && (rc != ESIF_E_NEED_LARGER_BUFFER)) {
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
		rc = ESIF_E_NOT_SUPPORTED;
	}

	esif_ccb_free(powerCapsData.buf_ptr);
	return rc;
}

static eEsifError EsifUpDomain_PsysDetectInit(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 flagValue = 0;
	EsifPrimitiveTuple flagTuple = { GET_PLATFORM_POWER_LIMIT_ENABLE, 0, 0 };
	EsifData flagData = { ESIF_DATA_UINT32, &flagValue, sizeof(flagValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_PSYS_CONTROL, &flagTuple, &flagData);
	if ((rc != ESIF_OK) && (rc != ESIF_I_AGAIN) && (rc != ESIF_E_NEED_LARGER_BUFFER)) {
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_PSYS_CONTROL);
		rc = ESIF_E_NOT_SUPPORTED;
	}

	return rc;
}

static eEsifError EsifUpDomain_PerfDetectInit(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple perfTuple = { GET_PERF_SUPPORT_STATES, 0, 255 };
	EsifData perfData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	ESIF_ASSERT(self != NULL);
	
	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_PERF_CONTROL, &perfTuple, &perfData);
	if ((rc != ESIF_OK) && (rc != ESIF_I_AGAIN) && (rc != ESIF_E_NEED_LARGER_BUFFER)) {
		// Domain may be a processor domain, in this case, we need to
		// query again but with a different primitive
		rc = EsifUpDomain_ProcPerfDetect(self);

		if ((rc == ESIF_OK) && (EsifUpDomain_IsHwpCapable(self) != ESIF_FALSE)) {
			rc = ESIF_E_NOT_SUPPORTED;
		}

		if ((rc != ESIF_OK) && (rc != ESIF_I_AGAIN) && (rc != ESIF_E_NEED_LARGER_BUFFER)) {
			EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_PERF_CONTROL);
			rc = ESIF_E_NOT_SUPPORTED;
		}
	}

	esif_ccb_free(perfData.buf_ptr);
	return rc;
}

static eEsifError EsifUpDomain_ProcPerfDetect(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple procPerfTuple = { GET_PROC_PERF_SUPPORT_STATES, 0, 255 };
	EsifData procPerfData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_PERF_CONTROL, &procPerfTuple, &procPerfData);

	esif_ccb_free(procPerfData.buf_ptr);
	return rc;
}

static Bool EsifUpDomain_IsHwpCapable(
	EsifUpDomainPtr self
	)
{
	Bool isCapable = ESIF_FALSE;
	ESIF_ASSERT(self != NULL);

	if (EsifUpDomain_IsHwpSupported(self)) {
		isCapable = EsifUpDomain_IsHwpEnabled(self);
	}

	return isCapable;
}

static Bool EsifUpDomain_IsHwpSupported(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple hwpTuple = { GET_PROC_HWP_SUPPORT_CHECK, 0, 255 };
	UInt32 isSupported = ESIF_FALSE;
	EsifData supportedData = { ESIF_DATA_UINT32, &isSupported, sizeof(isSupported), 0 };

	ESIF_ASSERT(self != NULL);
	hwpTuple.domain = self->domain;

	rc = EsifUp_ExecutePrimitive(self->upPtr, &hwpTuple, NULL, &supportedData);
	if (rc != ESIF_OK) {
		isSupported = ESIF_FALSE;
	}

	return isSupported != ESIF_FALSE;
}

static Bool EsifUpDomain_IsHwpEnabled(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple hwpTuple = { GET_PROC_HWP_ENABLE, 0, 255 };
	UInt32 isEnabled = ESIF_FALSE;
	EsifData enabledData = { ESIF_DATA_UINT32, &isEnabled, sizeof(isEnabled), 0 };

	ESIF_ASSERT(self != NULL);
	hwpTuple.domain = self->domain;

	rc = EsifUp_ExecutePrimitive(self->upPtr, &hwpTuple, NULL, &enabledData);
	if (rc != ESIF_OK) {
		isEnabled = ESIF_FALSE;
	}

	return isEnabled != ESIF_FALSE;
}

static eEsifError EsifUpDomain_ActiveControlDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple fanInfoTuple = { GET_FAN_INFORMATION, 0, 255 };
	EsifData fanInfoData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	EsifPrimitiveTuple fanCapabilityTuple = { GET_FAN_CAPABILITIES, 0, 255 };
	EsifData fanCapabilityData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	EsifPrimitiveTuple fanStatusTuple = { GET_FAN_STATUS, 0, 255 };
	EsifData fanStatusData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	EsifPrimitiveTuple fanPerformanceStateTuple = { GET_FAN_PERFORMANCE_STATES, 0, 255 };
	EsifData fanPerformanceStateData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL, &fanInfoTuple, &fanInfoData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL, &fanCapabilityTuple, &fanCapabilityData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL, &fanStatusTuple, &fanStatusData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL, &fanPerformanceStateTuple, &fanPerformanceStateData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = ESIF_E_NOT_SUPPORTED;
	EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL);

exit:
	esif_ccb_free(fanInfoData.buf_ptr);
	esif_ccb_free(fanCapabilityData.buf_ptr);
	esif_ccb_free(fanStatusData.buf_ptr);
	esif_ccb_free(fanPerformanceStateData.buf_ptr);
	return rc;
}

static eEsifError EsifUpDomain_DisplayControlDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple displayBrightnessTuple = { GET_DISPLAY_BRIGHTNESS, 0, 255 };
	EsifData displayBrightnessData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	EsifPrimitiveTuple brightnessLevelTuple = { GET_DISPLAY_BRIGHTNESS_LEVELS, 0, 255 };
	EsifData brightnessLevelData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	UInt32 bdlValue = 0;
	EsifPrimitiveTuple bdlTuple = { GET_DISPLAY_DEPTH_LIMIT, 0, 255 };
	EsifData bdlData = { ESIF_DATA_UINT32, &bdlValue, sizeof(bdlValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL, &displayBrightnessTuple, &displayBrightnessData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL, &brightnessLevelTuple, &brightnessLevelData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL, &bdlTuple, &bdlData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = ESIF_E_NOT_SUPPORTED;
	EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL);

exit:
	esif_ccb_free(displayBrightnessData.buf_ptr);
	esif_ccb_free(brightnessLevelData.buf_ptr);
	return rc;
}

static eEsifError EsifUpDomain_PeakPowerControlDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	UInt32 acPeakPowerValue = 0;
	EsifPrimitiveTuple acPeakPowerTuple = { GET_AC_PEAK_POWER, 0, 255 };
	EsifData acPeakPowerData = { ESIF_DATA_POWER, &acPeakPowerValue, sizeof(acPeakPowerValue), 0 };

	UInt32 dcPeakPowerValue = 0;
	EsifPrimitiveTuple dcPeakPowerTuple = { GET_DC_PEAK_POWER, 0, 255 };
	EsifData dcPeakPowerData = { ESIF_DATA_POWER, &dcPeakPowerValue, sizeof(dcPeakPowerValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL, &acPeakPowerTuple, &acPeakPowerData);
	if (rc != ESIF_OK) {
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL);
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL, &dcPeakPowerTuple, &dcPeakPowerData);
	if (rc != ESIF_OK) {
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL);
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

exit:
	return rc;
}

static eEsifError EsifUpDomain_ProcessorControlDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	UInt32 tccValue = 0;
	EsifPrimitiveTuple tccTuple = { GET_TCC_OFFSET, 0, 255 };
	EsifData tccData = { ESIF_DATA_TEMPERATURE, &tccValue, sizeof(tccValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL, &tccTuple, &tccData);
	if (rc != ESIF_OK) {
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL);
		rc = ESIF_E_NOT_SUPPORTED;
	}

	return rc;
}

static eEsifError EsifUpDomain_PlatformPowerStatusDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	UInt32 platformPowerSourceValue = 0;
	EsifPrimitiveTuple platformPowerSourceTuple = { GET_PLATFORM_POWER_SOURCE, 0, 255 };
	EsifData platformPowerSourceData = { ESIF_DATA_UINT32, &platformPowerSourceValue, sizeof(platformPowerSourceValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS, &platformPowerSourceTuple, &platformPowerSourceData);
	if (rc != ESIF_OK) {
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS);
		rc = ESIF_E_NOT_SUPPORTED;
	}

	return rc;
}

static eEsifError EsifUpDomain_RfProfileStatusDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	UInt64 rfCenterFrequencyValue = 0;
	EsifPrimitiveTuple rfCenterFrequencyTuple = { GET_RFPROFILE_CENTER_FREQUENCY, 0, 255 };
	EsifData rfCenterFrequencyData = { ESIF_DATA_FREQUENCY, &rfCenterFrequencyValue, sizeof(rfCenterFrequencyValue), 0 };

	EsifPrimitiveTuple rfActiveChannelsTuple = { GET_RF_ACTIVE_CHANNELS, 0, 255 };
	EsifData rfActiveChannelsData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	UInt64 memBdwVaue = 0;
	EsifPrimitiveTuple memBdwTuple = { GET_MEMORY_SPEED, 0, 255 };
	EsifData memBdwData = { ESIF_DATA_FREQUENCY, &memBdwVaue, sizeof(memBdwVaue), 0 };

	EsifData listData = { ESIF_DATA_STRING, NULL, ESIF_DATA_ALLOCATE, 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS, &rfCenterFrequencyTuple, &rfCenterFrequencyData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS, &rfActiveChannelsTuple, &rfActiveChannelsData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS, &memBdwTuple, &memBdwData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifExecutePrimitive(ESIF_HANDLE_PRIMARY_PARTICIPANT, GET_AUTO_ENUM_LIST, "D0", 255, NULL, &listData);
	if (ESIF_I_NO_LEGACY_SUPPORT == rc) {
		ESIF_TRACE_DEBUG("Skipping disabling RFPROFILE Status\n");
		rc = ESIF_OK;
		goto exit;
	}

	rc = ESIF_E_NOT_SUPPORTED;
	EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS);

exit:
	esif_ccb_free(rfActiveChannelsData.buf_ptr);
	esif_ccb_free(listData.buf_ptr);
	return rc;
}

static eEsifError EsifUpDomain_RfProfileControlDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	UInt64 rfCenterFrequencyValue = 0;
	EsifPrimitiveTuple rfCenterFrequencyTuple = { GET_RFPROFILE_CENTER_FREQUENCY, 0, 255 };
	EsifData rfCenterFrequencyData = { ESIF_DATA_FREQUENCY, &rfCenterFrequencyValue, sizeof(rfCenterFrequencyValue), 0 };

	UInt64 rfMinFrequencyValue = 0;
	EsifPrimitiveTuple rfMinFrequencyTuple = { GET_RFPROFILE_MIN_FREQUENCY, 0, 255 };
	EsifData rfMinFrequencyData = { ESIF_DATA_FREQUENCY, &rfMinFrequencyValue, sizeof(rfMinFrequencyValue), 0 };

	UInt64 rfMaxFrequencyValue = 0;
	EsifPrimitiveTuple rfMaxFrequencyTuple = { GET_RFPROFILE_MAX_FREQUENCY, 0, 255 };
	EsifData rfMaxFrequencyData = { ESIF_DATA_FREQUENCY, &rfMaxFrequencyValue, sizeof(rfMaxFrequencyValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL, &rfCenterFrequencyTuple, &rfCenterFrequencyData);
	if (rc != ESIF_OK) {
		rc = ESIF_E_NOT_SUPPORTED;
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL);
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL, &rfMinFrequencyTuple, &rfMinFrequencyData);
	if (rc != ESIF_OK) {
		rc = ESIF_E_NOT_SUPPORTED;
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL);
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL, &rfMaxFrequencyTuple, &rfMaxFrequencyData);
	if (rc != ESIF_OK) {
		rc = ESIF_E_NOT_SUPPORTED;
		EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL);
		goto exit;
	}

exit:
	return rc;
}

static eEsifError EsifUpDomain_EnergyControlDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	UInt32 raplEnergyValue = 0;
	EsifPrimitiveTuple raplEnergyTuple = { GET_RAPL_ENERGY, 0, 255 };
	EsifData raplEnergyData = { ESIF_DATA_UINT32, &raplEnergyValue, sizeof(raplEnergyValue), 0 };

	EsifPrimitiveTuple raplEnergyCounterInfoTuple = { GET_RAPL_ENERGY_COUNTER_INFO, 0, 255 };
	EsifData raplEnergyCounterInfoData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_ENERGY_CONTROL, &raplEnergyTuple, &raplEnergyData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_ENERGY_CONTROL, &raplEnergyCounterInfoTuple, &raplEnergyCounterInfoData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = ESIF_E_NOT_SUPPORTED;
	EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_ENERGY_CONTROL);

exit:
	esif_ccb_free(raplEnergyCounterInfoData.buf_ptr);
	return rc;
}

static eEsifError EsifUpDomain_ActivityStatusDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;
	UInt32 residencyUtilizationValue = 0;
	EsifPrimitiveTuple residencyUtilizationTuple = { GET_PARTICIPANT_RESIDENCY_UTILIZATION, 0, 255 };
	EsifData residencyUtilizationData = { ESIF_DATA_PERCENT, &residencyUtilizationValue, sizeof(residencyUtilizationValue), 0 };

	UInt64 coreActivityCounterValue = 0;
	EsifPrimitiveTuple coreActivityCounterTuple = { GET_CORE_ACTIVITY_COUNTER, 0, 255 };
	EsifData coreActivityCounterData = { ESIF_DATA_UINT64, &coreActivityCounterValue, sizeof(coreActivityCounterValue), 0 };

	UInt64 procTscValue = 0;
	EsifPrimitiveTuple procTscTuple = { GET_PROC_TSC, 0, 255 };
	EsifData procTscData = { ESIF_DATA_UINT64, &procTscValue, sizeof(procTscValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_ACTIVITY_STATUS, &residencyUtilizationTuple, &residencyUtilizationData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_ACTIVITY_STATUS, &coreActivityCounterTuple, &coreActivityCounterData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_ACTIVITY_STATUS, &procTscTuple, &procTscData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = ESIF_E_NOT_SUPPORTED;
	EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_ACTIVITY_STATUS);

exit:
	return rc;
}

static eEsifError  EsifUpDomain_BatteryStatusDetectInit(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 pmaxValue = 0;
	EsifPrimitiveTuple pmaxTuple = { GET_PLATFORM_MAX_BATTERY_POWER, 0, 255 };
	EsifData pmaxData = { ESIF_DATA_POWER, &pmaxValue, sizeof(pmaxValue), 0 };

	UInt32 pbssValue = 0;
	EsifPrimitiveTuple pbssTuple = { GET_PLATFORM_BATTERY_STEADY_STATE, 0, 255 };
	EsifData pbssData = { ESIF_DATA_POWER, &pbssValue, sizeof(pbssValue), 0 };

	UInt32 ctypValue = 0;
	EsifPrimitiveTuple ctypTuple = { GET_CHARGER_TYPE, 0, 255 };
	EsifData ctypData = { ESIF_DATA_UINT32, &ctypValue, sizeof(ctypValue), 0 };

	UInt32 bstValue = 0;
	EsifPrimitiveTuple bstTuple = { GET_BATTERY_STATUS, 0, 255 };
	EsifData bstData = { ESIF_DATA_BINARY, &bstValue, sizeof(bstValue), 0 };

	UInt32 bixValue = 0;
	EsifPrimitiveTuple bixTuple = { GET_BATTERY_INFORMATION, 0, 255 };
	EsifData bixData = { ESIF_DATA_BINARY, &bixValue, sizeof(bixValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_BATTERY_STATUS, &pmaxTuple, &pmaxData);
	if ((rc == ESIF_OK) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_BATTERY_STATUS, &pbssTuple, &pbssData);
	if ((rc == ESIF_OK) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_BATTERY_STATUS, &ctypTuple, &ctypData);
	if ((rc == ESIF_OK) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_BATTERY_STATUS, &bstTuple, &bstData);
	if ((rc == ESIF_OK) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_BATTERY_STATUS, &bixTuple, &bixData);
	if ((rc == ESIF_OK) || (rc == ESIF_I_AGAIN) || (rc == ESIF_E_NEED_LARGER_BUFFER)) {
		goto exit;
	}

	rc = ESIF_E_NOT_SUPPORTED;
	EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_BATTERY_STATUS);

exit:
	return rc;
}

static eEsifError EsifUpDomain_WorkloadClassificationDetectInit(
	EsifUpDomainPtr self
)
{
	eEsifError rc = ESIF_OK;

	UInt32 socWorkloadSourceValue = 0;
	EsifPrimitiveTuple socWorkloadSourceTuple = { GET_SOCWLC_SOURCE, 0, 255 };
	EsifData socWorkloadSourceData = { ESIF_DATA_UINT32, &socWorkloadSourceValue, sizeof(socWorkloadSourceValue), 0 };

	UInt32 socWorkloadValue = 0;
	EsifPrimitiveTuple socWorkloadTuple;
	EsifData socWorkloadData = { ESIF_DATA_UINT32, &socWorkloadValue, sizeof(socWorkloadValue), 0 };

	ESIF_ASSERT(self != NULL);

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION, &socWorkloadSourceTuple, &socWorkloadSourceData);
	if (rc == ESIF_OK && socWorkloadSourceValue == ESIF_SOCWC_SOURCE_HARDWARE) {
		socWorkloadTuple.id = GET_HW_SOC_WORKLOAD;
		socWorkloadTuple.domain = 0;
		socWorkloadTuple.instance = ESIF_INSTANCE_INVALID;
	}
	else {
		//
		// Calling GET_SOC_WORKLOAD here results in the UPE_SOCWC being loaded,
		// and therefore, additional power consumption. Removing check for now
		// while final solution is determined.
		//
		goto exit;
	}

	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION, &socWorkloadTuple, &socWorkloadData);
	if (rc == ESIF_OK) {
		goto exit;
	}

	rc = ESIF_E_NOT_SUPPORTED;
	EsifUpDomain_DisableCap(self, ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION);

exit:
	return rc;
}

static eEsifError EsifUpDomain_StateDetectInit(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 stateValue = ESIF_DOMAIN_STATE_INVALID;
	EsifPrimitiveTuple stateTuple = {GET_PERF_PSTATE_DEPTH_LIMIT, 0, 255};
	EsifData stateData = { ESIF_DATA_UINT32, &stateValue, sizeof(stateValue), 0 };

	ESIF_ASSERT(self != NULL);
	
	self->lastState = ESIF_DOMAIN_STATE_INVALID;
	rc = EsifUpDomain_CapDetect(self, ESIF_CAPABILITY_TYPE_PERF_CONTROL, &stateTuple, &stateData);
	
	if ((rc != ESIF_OK) && (rc != ESIF_I_AGAIN) && (rc != ESIF_E_NEED_LARGER_BUFFER)) {
		self->statePollType = ESIF_POLL_UNSUPPORTED;
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s %s: Starting poll of performance state. \n",
		self->participantName,
		self->domainStr);

	EsifUpDomain_SetStatePollPeriod(self, PERF_STATE_POLL_PERIOD);
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
	esif_temp_t tempMin = (esif_temp_t)ESIF_SDK_MIN_AUX_TRIP;
	esif_temp_t tempMax = ESIF_SDK_MAX_AUX_TRIP;
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
	esif_temp_t aux0WHyst = (esif_temp_t)ESIF_SDK_MIN_AUX_TRIP;
	esif_temp_t tempAux0Def = ESIF_DOMAIN_TEMP_AUX0_DEF;
	esif_temp_t hystRel = {0};
	esif_temp_t oneDegreeRel = 1; /* 1C */

	esif_convert_temp(ESIF_TEMP_C, NORMALIZE_TEMP_TYPE, &aux0WHyst);

	esif_convert_temp(ESIF_TEMP_C, NORMALIZE_TEMP_TYPE, &oneDegreeRel);
	oneDegreeRel = esif_temp_abs_to_rel(oneDegreeRel);

	if (ESIF_DOMAIN_TEMP_INVALID == aux0) {
		esif_convert_temp(ESIF_TEMP_C, NORMALIZE_TEMP_TYPE, &tempAux0Def);
		aux0 = tempAux0Def;
	}

	hystRel = esif_temp_abs_to_rel(self->tempHysteresis);
	if (hystRel < (aux0 - oneDegreeRel)) {
		aux0WHyst = aux0 - hystRel - oneDegreeRel;
	}

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

	if (!(self->capability_for_domain.capability_flags & ESIF_CAPABILITY_TEMP_THRESHOLD)) {
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
			/*
			 Reset the poll type for cases where poll type 
			 was previously set to "unsupported" prior to the action 
			 being loaded or the device being available
			 */
			self->tempPollType = ESIF_POLL_DOMAIN;
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
	esif_temp_t tempInvalidValue = ESIF_DOMAIN_TEMP_INVALID_VALUE;

	tempTuple.domain = self->domain;
	rc = EsifUp_ExecutePrimitive(self->upPtr, &tempTuple, NULL, &tempResponse);
	if (rc != ESIF_OK) {
		if (rc == ESIF_E_STOP_POLL) {
			self->tempPollType = ESIF_POLL_UNSUPPORTED;
		}

		if (ESIF_FALSE != self->tempLastTempValid) {
			EsifEventMgr_SignalEvent(self->participantId, self->domain, ESIF_EVENT_TEMP_THRESHOLD_CROSSED, NULL);		
			ESIF_TRACE_DEBUG("Temp read invalid; Sending THRESHOLD CROSSED EVENT Participant: %s, Domain: %s, Temperature: %d \n",
				self->participantName,
				self->domainName,
				temp);
		}

		self->tempLastTempValid = ESIF_FALSE;
		goto exit;
	}
	
	ESIF_TRACE_DEBUG("Polling temperature for participant: %s, domain: %s. Temperature is: %d. \n",
		self->participantName,
		self->domainName,
		temp);

	self->tempLastTempValid = ESIF_TRUE;

	/*
	* If we see a value of 255, we consider it invalid and do not send an event.
	* We also extend the polling period in such a case (in code elsewhere.)
	*/
	esif_convert_temp(ESIF_TEMP_C, NORMALIZE_TEMP_TYPE, &tempInvalidValue);
	self->tempInvalidValueDetected = (tempInvalidValue == temp) ? ESIF_TRUE : ESIF_FALSE;

	if (self->tempNotifySent && EsifUpDomain_TempIsInThresholds(self, temp)) {
		self->tempNotifySent = ESIF_FALSE;
	}
	
	if (EsifUpDomain_IsTempOutOfThresholds(self, temp) &&
		!self->tempNotifySent &&
		!self->tempInvalidValueDetected) {

		self->tempNotifySent = ESIF_TRUE;

		EsifEventMgr_SignalEvent(self->participantId, self->domain, ESIF_EVENT_TEMP_THRESHOLD_CROSSED, NULL);

		ESIF_TRACE_DEBUG("THRESHOLD CROSSED EVENT!!! Participant: %s, Domain: %s, Temperature: %d, Aux0: %d, Aux0WHyst: %d, Aux1: %d, Hyst: %d \n",
			self->participantName,
			self->domainName,
			temp,
			self->tempAux0,
			self->tempAux0WHyst,
			self->tempAux1,
			esif_temp_abs_to_rel(self->tempHysteresis));
	}
exit:
	return rc;
}

eEsifError EsifUpDomain_CheckState(EsifUpDomainPtr self)
{
	eEsifError rc = ESIF_OK;
	UInt32 state = ESIF_DOMAIN_STATE_INVALID;
	EsifPrimitiveTuple stateTuple = {GET_PERF_PSTATE_DEPTH_LIMIT, 0, 255};
	struct esif_data stateResponse = { ESIF_DATA_UINT32, &state, sizeof(state), 0 };

	ESIF_TRACE_DEBUG("%s %s: Polled performance state. \n",
		self->participantName,
		self->domainStr);

	stateTuple.domain = self->domain;
	rc = EsifUp_ExecutePrimitive(self->upPtr, &stateTuple, NULL, &stateResponse);
	if (rc != ESIF_OK) {
		goto exit;
	}
	
	if (self->lastState != state && state != ESIF_DOMAIN_STATE_INVALID) {
		EsifEventMgr_SignalEvent(self->participantId, self->domain, ESIF_EVENT_PERF_CAPABILITY_CHANGED, NULL);
		ESIF_TRACE_DEBUG("Performance Capability CHANGED! Participant: %s, Domain: %s, State: %d \n", self->participantName, self->domainName, state);
		self->lastState = state;
	}
exit:
	return rc;
}

static void EsifUpDomain_PollState(
	const void *ctx
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpDomainPtr self = (EsifUpDomainPtr) ctx;
	EsifUpPtr upPtr = NULL;

	if (self == NULL) {
		goto exit;
	}

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(self->participantId);
	if (upPtr == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	rc = EsifUpDomain_CheckState(self);

	/* If another type of polling was started, skip the timer reset */
	if (self->statePollType != ESIF_POLL_DOMAIN) {
		goto exit;
	}

	rc = EsifUpDomain_StartStatePollPriv(self);
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}

	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Unable to set poll timer on %s %s : %s(%d)\n", self->participantName, self->domainName, esif_rc_str(rc), rc);
	}
}


static void EsifUpDomain_PollTemp(
	const void *ctx
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpDomainPtr self = (EsifUpDomainPtr) ctx;
	EsifUpPtr upPtr = NULL;
	UInt32 pollPeriod = 0;
	
	if (self == NULL) {
		goto exit;
	}

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(self->participantId);
	if (upPtr == NULL) {
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
			pollPeriod = (self->tempInvalidValueDetected && (self->tempPollPeriod < ESIF_DOMAIN_TEMP_INVALID_POLL_PERIOD)) ? ESIF_DOMAIN_TEMP_INVALID_POLL_PERIOD : self->tempPollPeriod;
			rc = esif_ccb_timer_set_msec(&self->tempPollTimer,
				pollPeriod);
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

eEsifError EsifUpDomain_InitTempPoll(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 period = 0;
	EsifPrimitiveTuple periodTuple = { GET_PARTICIPANT_SAMPLE_PERIOD,self->domain, 255 };
	EsifPrimitiveTuple behaviorTuple = { SET_PARTICIPANT_SAMPLE_BEHAVIOR, self->domain, 255 };
	EsifData periodData = { ESIF_DATA_TIME, &period, sizeof(period), 0 };
	
	//Set up polling for thermal
	rc = EsifUp_ExecutePrimitive(self->upPtr, &periodTuple, NULL, &periodData);
	if (ESIF_OK == rc) {
		ESIF_TRACE_DEBUG("Setting thermal polling period for %s:%s to %d\n", self->participantName, self->domainStr, period);

		rc = EsifUp_ExecutePrimitive(self->upPtr, &behaviorTuple, &periodData, NULL);
		if (rc != ESIF_OK) {
			ESIF_TRACE_WARN("Failed to set thermal polling period for %s:%s\n", self->participantName, self->domainStr);
		}
	}

	return rc;
}

eEsifError EsifUpDomain_InitPowerPoll(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 period = 0;
	EsifPrimitiveTuple periodTuple = { GET_POWER_SAMPLE_PERIOD, self->domain, 255 };
	EsifPrimitiveTuple behaviorTuple = { SET_POWER_SAMPLE_BEHAVIOR, self->domain, 255 };
	EsifData periodData = { ESIF_DATA_TIME, &period, sizeof(period), 0 };

	//Set up polling for power
	rc = EsifUp_ExecutePrimitive(self->upPtr, &periodTuple, NULL, &periodData);
	if (ESIF_OK == rc) {
		ESIF_TRACE_DEBUG("Setting polling period for %s:%s to %d\n", self->participantName, self->domainStr, period);

		rc = EsifUp_ExecutePrimitive(self->upPtr, &behaviorTuple, &periodData, NULL);
		if (rc != ESIF_OK) {
			ESIF_TRACE_WARN("Failed to set power polling period for %s:%s\n", self->participantName, self->domainStr);
		}
	}

	return rc;
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

static eEsifError EsifUpDomain_StartStatePollPriv(
	EsifUpDomainPtr self
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	/* Skip if another type of polling is started */
	if (self->statePollType != ESIF_POLL_NONE && self->statePollType != ESIF_POLL_DOMAIN) {
		goto exit;
	}

	/* Skip if the poll period is invalid */
	if (self->statePollPeriod <= 0) {
		ESIF_TRACE_DEBUG(
			"%s %s: Skipping perf state poll due to invalid period: %d. \n",
			self->participantName,
			self->domainName,
			self->statePollPeriod);
		goto exit;
	}

	if (self->statePollInitialized != ESIF_TRUE) {

		ESIF_TRACE_DEBUG(
			"%s %s: Starting perf state polling\n",
			self->participantName,
			self->domainName);

		rc = esif_ccb_timer_init(&self->statePollTimer, (esif_ccb_timer_cb) EsifUpDomain_PollState, self);
		if (ESIF_OK != rc)
			goto exit;

		self->statePollInitialized = ESIF_TRUE;
		self->statePollType = ESIF_POLL_DOMAIN;
	}
	
	rc = esif_ccb_timer_set_msec(&self->statePollTimer, self->statePollPeriod);
	
exit:
	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("Error with setting uf poll timer: %s(%d)\n", esif_rc_str(rc), rc);
	}
	return rc;
}

static Bool EsifUpDomain_TempIsInThresholds(
	EsifUpDomainPtr self,
	UInt32 temp
	)
{
	ESIF_ASSERT(self != NULL);

	return  ((self->tempAux0 == ESIF_DOMAIN_TEMP_INVALID) || (temp >= self->tempAux0)) &&
		((self->tempAux1 == ESIF_DOMAIN_TEMP_INVALID) || (temp < self->tempAux1));
}

static Bool EsifUpDomain_IsTempOutOfThresholds(
	EsifUpDomainPtr self,
	UInt32 temp
	)
{
	ESIF_ASSERT(self != NULL);

	return ((temp != ESIF_DOMAIN_TEMP_INVALID) &&
			( ((self->tempAux0 != ESIF_DOMAIN_TEMP_INVALID) && (temp <= self->tempAux0WHyst)) ||
			  ((self->tempAux1 != ESIF_DOMAIN_TEMP_INVALID) && (temp >= self->tempAux1))
			));
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

void EsifUpDomain_UnInitDomain(EsifUpDomainPtr self)
{
	if (self != NULL) {
		esif_ccb_lock_uninit(&self->capsLock);
		esif_ccb_lock_uninit(&self->tempLock);
	}
}

void EsifUpDomain_StopTempPoll(
	EsifUpDomainPtr self
	)
{
	esif_ccb_timer_kill_w_wait(&self->tempPollTimer);

	self->tempPollInitialized = ESIF_FALSE;
	self->tempPollType = ESIF_POLL_NONE;
}

void EsifUpDomain_StopStatePoll(
	EsifUpDomainPtr self
	)
{
	esif_ccb_timer_kill_w_wait(&self->statePollTimer);

	self->statePollInitialized = ESIF_FALSE;
	self->statePollType = ESIF_POLL_NONE;
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

eEsifError EsifUpDomain_SetStatePollPeriod(
	EsifUpDomainPtr self,
	UInt32 sampleTime
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	if (sampleTime <= 0) {
		ESIF_TRACE_ERROR("Invalid time period attempted for perf state poll. \n");
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	self->statePollPeriod = sampleTime;
	rc = EsifUpDomain_StartStatePollPriv(self);
	
	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("Error with setting uf poll timer: %s(%d)\n", esif_rc_str(rc), rc);
		goto exit;
	}
exit:
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
		EsifEventMgr_SignalEvent(self->participantId, self->domain, ESIF_EVENT_TEMP_THRESHOLD_CROSSED, NULL);
		ESIF_TRACE_DEBUG("THRESHOLD CROSSED EVENT!!! Participant: %s, Domain: %s, Temperature: %d \n", self->participantName, self->domainName, virtTemp);
	}

	return;
}

eEsifError EsifUpDomain_SetTempHysteresis(
	EsifUpDomainPtr self,
	esif_temp_t tempHysteresis
	)
{
	ESIF_ASSERT(self != NULL);

	esif_ccb_write_lock(&self->tempLock);

	self->tempHysteresis = tempHysteresis;
	self->tempAux0WHyst = EsifUpDomain_CalcAux0WHyst(self, self->tempAux0);

	esif_ccb_write_unlock(&self->tempLock);
	return ESIF_OK;
}

eEsifError EsifUpDomain_SignalOSEvent(
	EsifUpDomainPtr self,
	UInt32 updatedValue,
	eEsifEventType eventType
	)
{
	EsifData evdata;
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	evdata.type = ESIF_DATA_UINT32;
	evdata.buf_ptr = &updatedValue;
	evdata.buf_len = sizeof(updatedValue);
	evdata.data_len = sizeof(updatedValue);
	rc = EsifEventMgr_SignalEvent(self->participantId, self->domain, eventType, &evdata);
	ESIF_TRACE_DEBUG(
		"Event %s signaled with value %d. rc = %d(%s)\n", 
		esif_event_type_str(eventType), updatedValue, rc, esif_rc_str(rc));

	return rc;
}

eEsifError EsifUpDomain_SignalForegroundAppChanged(
	EsifUpDomainPtr self,
	EsifString appName
	)
{
	EsifDataPtr evdataPtr = NULL;
	eEsifError rc = ESIF_OK;
	
	EsifString appNameToSend = esif_ccb_strdup(appName);


	ESIF_ASSERT(self != NULL);

	evdataPtr = EsifData_CreateAs(ESIF_DATA_STRING, appNameToSend, ESIFAUTOLEN, ESIFAUTOLEN);

	if (evdataPtr != NULL) {
		rc = EsifEventMgr_SignalEvent(self->participantId, self->domain, ESIF_EVENT_FOREGROUND_APP_CHANGED, evdataPtr);
	}

	EsifData_Destroy(evdataPtr);

	ESIF_TRACE_DEBUG("Foreground application set to: %s. rc = %d(%s)\n", appName, rc, esif_rc_str(rc));

	return rc;
}

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
	)
{
	eEsifError rc = ESIF_OK;

	if ((NULL == iteratorPtr) || (NULL == upPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_memset(iteratorPtr, 0, sizeof(*iteratorPtr));
	rc = EsifUp_GetRef(upPtr);
	if (ESIF_OK != rc) {
		rc = ESIF_E_NO_CREATE;
		goto exit;
	}

	iteratorPtr->upPtr = upPtr;
	iteratorPtr->marker = UP_DOMAIN_ITERATOR_MARKER;
exit:
	return rc;
}


/* See EsifUpDomain_InitIterator for usage */
eEsifError EsifUpDomain_GetNextUd(
	UpDomainIteratorPtr iteratorPtr,
	EsifUpDomainPtr *upDomainPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr upPtr = NULL;

	if ((NULL == upDomainPtr) || (NULL == iteratorPtr)) {
		ESIF_TRACE_WARN("Parameter is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (iteratorPtr->marker != UP_DOMAIN_ITERATOR_MARKER) {
		ESIF_TRACE_WARN("Iterator is invalid\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	upPtr = iteratorPtr->upPtr;
	if (iteratorPtr->handle >= upPtr->domainCount) {
		*upDomainPtr = NULL;
		EsifUp_PutRef(upPtr);
		iteratorPtr->marker = 0;	// Prevent any further PutRef operations through this iterator
		rc = ESIF_E_ITERATION_DONE;
		goto exit;
	} 
	
	*upDomainPtr = &upPtr->domains[iteratorPtr->handle];
	iteratorPtr->handle++;

exit:
	return rc;
}

void EsifUpDomain_SetUpId(
	EsifUpDomainPtr self,
	esif_handle_t participantId
	)
{
	if (self != NULL) {
		self->participantId = participantId;
	}
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
