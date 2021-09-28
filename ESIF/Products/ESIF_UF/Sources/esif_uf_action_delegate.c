/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ACTION

#include "esif_uf.h"			/* Upper Framework */
#include "esif_uf_actmgr.h"		/* Action Manager */
#include "esif_uf_appmgr.h"		/* Application Manager */
#include "esif_uf_domain.h"
#include "esif_uf_primitive.h"
#include "esif_uf_cfgmgr.h"
#include "esif_uf_shell.h"		/* Dynamic Participants */
#include "esif_lib_databank.h"
#include "esif_lib_datarepo.h"
#include "esif_temp.h"
#include "esif_ccb_cpuid.h"
#include "esif_uf_eventmgr.h"
#include "esif_uf_sensors.h"

// !!!
// TODO: Once we move to the DOMAIN MGR some of the interfaces in this file might change!!!
// Currently we do not get any of the domain/participant information as part of the interface.
// !!!

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

extern char *esif_str_replace(char *orig, char *rep, char *with);

static eEsifError EsifGetActionDelegateGtt0(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateGtt1(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateTemp(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateVirtualTemperature(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateCnfg(
	const EsifUpDomainPtr domainPtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateProcBrandMod(
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateSupportSocWorkload(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateIsFaceDetectionCapableSensor(
	EsifDataPtr responsePtr
);

static eEsifError EsifGetActionDelegateBpsg(EsifDataPtr responsePtr);
static eEsifError EsifSetActionDelegateBpss(const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateSphb(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);
	
static eEsifError EsifSetActionDelegatePat0(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegatePat1(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateRset(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateActl(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	const EsifFpcActionPtr fpcActionPtr);

static eEsifError EsifSetActionDelegateEvaluateParticipantCaps(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateSampleBehavior(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateVirtualTemperature(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateToSignalOSEvent(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	eEsifEventType eventType);

static eEsifError EsifSetActionDelegateToSignalForegroundAppChanged(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateSsap(
	EsifUpPtr upPtr,
	EsifDataPtr requestPtr
	);

static eEsifError EsifSetActionDelegateSsme(EsifDataPtr requestPtr);
static eEsifError EsifSetActionDelegateScsm(EsifDataPtr requestPtr);
static eEsifError EsifSetActionDelegateScas();


#if defined(ESIF_ATTR_OS_WINDOWS)

esif_error_t set_display_state_win(EsifDataPtr requestPtr);
esif_error_t set_screen_autolock_state_win(EsifDataPtr requestPtr);
esif_error_t set_wake_on_approach_state_win(EsifDataPtr requestPtr);
esif_error_t set_workstation_lock_win();
esif_error_t set_app_ratio_period_win(EsifDataPtr requestPtr);
esif_error_t get_last_hid_input_time_win(EsifDataPtr responsePtr);
esif_error_t get_display_required_win(EsifDataPtr responsePtr);
esif_error_t get_is_ext_mon_connected_win(EsifDataPtr responsePtr);
esif_error_t get_aggregate_display_information_win(EsifDataPtr responsePtr);
esif_error_t EsifSetActionDelegatePpmActivePackageSettingWin(EsifDataPtr requestPtr);
esif_error_t EsifSetActionDelegatePpmParamSettingWin(EsifDataPtr requestPtr);
esif_error_t EsifSetActionDelegatePowerSchemeEppWin(EsifDataPtr requestPtr);
esif_error_t EsifSetActionDelegateActivePowerSchemeWin();
esif_error_t EsifSetActionDelegatePpmParamClearWin();

#define set_display_state(reqPtr) set_display_state_win(reqPtr)
#define set_screen_autolock_state(reqPtr) set_screen_autolock_state_win(reqPtr)
#define set_wake_on_approach_state(reqPtr) set_wake_on_approach_state_win(reqPtr)
#define set_workstation_lock() set_workstation_lock_win()
#define set_app_ratio_period(reqPtr) set_app_ratio_period_win(reqPtr)
#define get_last_hid_input_time(rspPtr) get_last_hid_input_time_win(rspPtr)
#define get_display_required(rspPtr) get_display_required_win(rspPtr)
#define get_is_ext_mon_connected(rspPtr) get_is_ext_mon_connected_win(rspPtr)
#define get_aggregate_display_information(rspPtr) get_aggregate_display_information_win(rspPtr)
#define EsifSetActionDelegatePpmActivePackageSetting(requestPtr) EsifSetActionDelegatePpmActivePackageSettingWin(requestPtr) 
#define EsifSetActionDelegatePpmParamSetting(requestPtr) EsifSetActionDelegatePpmParamSettingWin(requestPtr)
#define EsifSetActionDelegatePowerSchemeEpp(requestPtr) EsifSetActionDelegatePowerSchemeEppWin(requestPtr)
#define EsifSetActionDelegateActivePowerScheme() EsifSetActionDelegateActivePowerSchemeWin()
#define EsifSetActionDelegatePpmParamClear() EsifSetActionDelegatePpmParamClearWin()

#elif defined(ESIF_ATTR_OS_LINUX)

#define set_display_state(reqPtr) (ESIF_E_NOT_IMPLEMENTED)
#define set_screen_autolock_state(reqPtr) (ESIF_E_NOT_IMPLEMENTED)
#define set_wake_on_approach_state(reqPtr) (ESIF_E_NOT_IMPLEMENTED)
#define set_workstation_lock() (ESIF_E_NOT_IMPLEMENTED)
#define set_app_ratio_period(reqPtr) (ESIF_E_NOT_IMPLEMENTED)
#define get_last_hid_input_time(rspPtr) (ESIF_E_NOT_IMPLEMENTED)
#define get_display_required(rspPtr) (ESIF_E_NOT_IMPLEMENTED)
#define get_is_ext_mon_connected(rspPtr) (ESIF_E_NOT_IMPLEMENTED)
#define get_aggregate_display_information(rspPtr) (ESIF_E_NOT_IMPLEMENTED)
#define EsifSetActionDelegatePpmActivePackageSetting(requestPtr) (ESIF_E_NOT_IMPLEMENTED)
#define EsifSetActionDelegatePpmParamSetting(requestPtr) (ESIF_E_NOT_IMPLEMENTED)
#define EsifSetActionDelegatePowerSchemeEpp(requestPtr) (ESIF_E_NOT_IMPLEMENTED)
#define EsifSetActionDelegateActivePowerScheme() (ESIF_E_NOT_IMPLEMENTED)
#define EsifSetActionDelegatePpmParamClear() (ESIF_E_NOT_IMPLEMENTED)

#endif


// Delegate Opcodes
#define DELEGATE_ACTL_OPCODE_STRT	'TRTS'	// Start App
#define DELEGATE_ACTL_OPCODE_STOP	'POTS'	// Stop App
#define DELEGATE_CNFG_OPCODE_REST	'TSER'	// Restart Apps on GDDV/DCFG Change
#define DELEGATE_CNFG_OPCODE_DESC	'CSED'	// Get GDDV Description

/*
** Handle ESIF Action Request
*/

static eEsifError ESIF_CALLCONV ActionDelegateGet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData p1 = {0};
	UInt32 method;
	EsifUpDomainPtr domainPtr = NULL;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(upPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);
	ESIF_ASSERT(NULL != primitivePtr);
	ESIF_ASSERT(NULL != fpcActionPtr);

	domainPtr = EsifUp_GetDomainById(upPtr, primitivePtr->tuple.domain);
	if (NULL == domainPtr) {
		ESIF_TRACE_ERROR("Unable to get domain\n");
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	rc = EsifFpcAction_GetParamAsEsifData(fpcActionPtr, 0, &p1);
	if ((ESIF_OK != rc) || (NULL == p1.buf_ptr)) {
		ESIF_TRACE_ERROR("Unable to get parameters\n");
		goto exit;
	}

	method = *((UInt32 *)p1.buf_ptr);

	switch (method) {
	/* Get Temperature Trip Points */
	case '0TTG':	/* GTT0 */
		rc = EsifGetActionDelegateGtt0(domainPtr, responsePtr);
		break;

	case '1TTG':	/* GTT1 */
		rc = EsifGetActionDelegateGtt1(domainPtr, responsePtr);
		break;

	case 'PMT_': /* _TMP */
		rc = EsifGetActionDelegateTemp(domainPtr, requestPtr, responsePtr);
		break;

	case 'GFNC': /* CNFG */
		rc = EsifGetActionDelegateCnfg(domainPtr, fpcActionPtr, requestPtr, responsePtr);
		break;

	case 'PMTV': /* VTMP */
		rc = EsifGetActionDelegateVirtualTemperature(domainPtr, responsePtr);
		break;

	case 'MBRP': /* PRBM */
		rc = EsifGetActionDelegateProcBrandMod(responsePtr);
		break;

	case 'KWSS': /* SSWK */
		rc = EsifGetActionDelegateSupportSocWorkload(domainPtr, requestPtr, responsePtr);
		break;

	case 'IHLG': /* GLHI **/
		rc = get_last_hid_input_time(responsePtr);
		break;

	case 'RDEG': /* GEDR */
		rc = get_display_required(responsePtr);
		break;

	case 'CMEI': /* IEMC **/
		rc = get_is_ext_mon_connected(responsePtr);
		break;

	case 'IDAG': /* GADI */
		rc = get_aggregate_display_information(responsePtr);
		break;

	case 'SCIG': /* GICS **/
		rc = EsifGetActionDelegateIsFaceDetectionCapableSensor(responsePtr);
		break;

	case 'GSPB': /* BPSG - Get Biometric Presence Sensor selection */
		rc = EsifGetActionDelegateBpsg(responsePtr);
		break;


	default:
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}
exit:
	return rc;
}


static eEsifError ESIF_CALLCONV ActionDelegateSet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData p1 = {0};
	UInt32 method;
	EsifUpDomainPtr domainPtr = NULL;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(upPtr);

	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr->buf_ptr);
	ESIF_ASSERT(NULL != primitivePtr);
	ESIF_ASSERT(NULL != fpcActionPtr);
	
	domainPtr = EsifUp_GetDomainById(upPtr, primitivePtr->tuple.domain);
	if (NULL == domainPtr) {
		ESIF_TRACE_ERROR("Unable to get domain\n");
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	rc = EsifFpcAction_GetParamAsEsifData(fpcActionPtr, 0, &p1);
	if ((ESIF_OK != rc) || (NULL == p1.buf_ptr)) {
		ESIF_TRACE_ERROR("Unable to get parameters\n");
		goto exit;
	}

	method = *((UInt32 *)p1.buf_ptr);
	switch (method) {
	
	/* Set Temperature Trip Points */
	case '0TAP':	/* PAT0 */
		ESIF_TRACE_INFO("PAT0 received\n");
		rc = EsifSetActionDelegatePat0(domainPtr, requestPtr);
		break;

	case '1TAP':	/* PAT1 */
		ESIF_TRACE_INFO("PAT1 received\n");
		rc = EsifSetActionDelegatePat1(domainPtr, requestPtr);
		break;

	case 'BSPS':	/* SPSB: Set Participant Sample Behavior */
		ESIF_TRACE_INFO("Set Sample Behavior received\n");
		rc = EsifSetActionDelegateSampleBehavior(domainPtr, requestPtr);
		break;

	case 'PMTV':	/* VTMP: Virtual Temperature */
		ESIF_TRACE_INFO("Set Virtual Temperature received\n");
		rc = EsifSetActionDelegateVirtualTemperature(domainPtr, requestPtr);
		break;

	case 'BHPS':	/* SPHB: Set Participant Hysteresis Behavior */
		ESIF_TRACE_INFO("Set Participant Hysteresis Behavior received\n");
		rc = EsifSetActionDelegateSphb(domainPtr, requestPtr);
		break;

	case 'CSPS':    /* SPSC: Set Platform State Of Charge */
		ESIF_TRACE_INFO("Set OS Battery Percentage received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_BATTERY_PERCENT_CHANGED);
		break;

	case 'SPPS':    /* SPPS: Set Platform Power Source */
		ESIF_TRACE_INFO("Set Platform Power Source received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_POWER_SOURCE_CHANGED);
		break;

	case 'OPDS':    /* SDPO: Set Display Orientation */
		ESIF_TRACE_INFO("Set Display Orientation received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED);
		break;

	case 'OVDS':    /* SDVO: Set Device Orientation */
		ESIF_TRACE_INFO("Set Device Orientation received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_DEVICE_ORIENTATION_CHANGED);
		break;

	case 'COMS':    /* SMOC: Set Motion Changed */
		ESIF_TRACE_INFO("Set Motion Changed received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_MOTION_CHANGED);
		break;

	case 'MKDS':    /* SDKM: Set Dock Mode */
		ESIF_TRACE_INFO("Set Dock Mode received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_DOCK_MODE_CHANGED);
		break;

	case 'MLCS':    /* SCLM: Set Cooling Mode */
		ESIF_TRACE_INFO("Set Cooling Mode received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_DTT_SYSTEM_COOLING_POLICY_CHANGED);
		break;

	case 'TSLS':    /* SLST: Set Lid State */
		ESIF_TRACE_INFO("Set Lid State received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_LID_STATE_CHANGED);
		break;

	case 'TFPS':    /* SPFT: Set Platform Type */
		ESIF_TRACE_INFO("Set Platform Type received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED);
		break;

	case 'AGFS':    /* SFGA: Set Foreground Application */
		ESIF_TRACE_INFO("Set Foreground Application received\n");
		rc = EsifSetActionDelegateToSignalForegroundAppChanged(domainPtr, requestPtr);
		break;

	case 'NOMS':    /* SMON: Set Mobile Notification */
		ESIF_TRACE_INFO("Set Mobile Notification request received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_MOBILE_NOTIFICATION);
		break;

	case 'MRMS':    /* SMRM: Set Mixed Reality Mode */
		ESIF_TRACE_INFO("Set Mixed Reality Mode request received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_MIXED_REALITY_MODE_CHANGED);
		break;

	case 'COSB':    /* BSOC: Set Battery State Of Charge */
		ESIF_TRACE_INFO("Set Battery State Of Charge request received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_BATTERY_STATE_OF_CHARGE_CHANGED);
		break;

	case 'PMTB':    /* BTMP: Set Battery Temperature */
		ESIF_TRACE_INFO("Set Battery Temperature request received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_BATTERY_TEMPERATURE_CHANGED);
		break;

	case 'TESR':    /* RSET: Reset Override */
		ESIF_TRACE_INFO("Reset Override request received\n");
		rc = EsifSetActionDelegateRset(domainPtr, requestPtr);
		break;

	case 'LAVE':    /* EVAL: Re-evaluate participant capabilities */
		ESIF_TRACE_INFO("Re-evaluate participant capabilities request received\n");
		rc = EsifSetActionDelegateEvaluateParticipantCaps(domainPtr, requestPtr);
		break;

	case 'LTCA':    /* ACTL: Application Control  */
		ESIF_TRACE_INFO("Application Control\n");
		rc = EsifSetActionDelegateActl(domainPtr, requestPtr, fpcActionPtr);
		break;

	case 'PASS':	/* SSAP: Specific Action Primitive execution */
		rc = EsifSetActionDelegateSsap(upPtr, requestPtr);
		break;

	case 'PPAS':	/* SAPP: Set Active PPM Package */
		ESIF_TRACE_INFO("Set Active PPM Package request received\n");
		rc = EsifSetActionDelegatePpmActivePackageSetting(requestPtr);
		break;

	case 'PPPS':	/* SPPP: Set PPM Package Parameters */
		ESIF_TRACE_INFO("Set PPM Package Parameters request received\n");
		rc = EsifSetActionDelegatePpmParamSetting(requestPtr);
		break;

	case 'ESPS':	/* SPSE: Set Power Scheme EPP */
		ESIF_TRACE_INFO("Set Power Scheme EPP request received\n");
		rc = EsifSetActionDelegatePowerSchemeEpp(requestPtr);
		break;

	case 'SPAS':	/* SAPS: Set (Reload) Active Power Scheme */
		ESIF_TRACE_INFO("Set (Reload) Active Power Scheme request received\n");
		rc = EsifSetActionDelegateActivePowerScheme();
		break;

	case 'NTPS':	/* SPTN: Clear PPM Package Parameters */
		ESIF_TRACE_INFO("Clear PPM Package Parameters request received\n");
		rc = EsifSetActionDelegatePpmParamClear();
		break;

	case 'SNSS': /* SSNS */
		rc = set_display_state(requestPtr);
		break;

	case 'SPRA': /* ARPS */
		rc = set_app_ratio_period(requestPtr);
		break;

	case 'SLAS': /* SALS */
		rc = set_screen_autolock_state(requestPtr);
		break;

	case 'LKWS': /* SWKL */
		rc = set_workstation_lock();
		break;

	case 'SAOW': /* WOAS */
		rc = set_wake_on_approach_state(requestPtr);
		break;

	case 'EMSS': /* SSME */
		rc = EsifSetActionDelegateSsme(requestPtr);
		break;

	case 'MSCS': /* SCSM */
		rc = EsifSetActionDelegateScsm(requestPtr);
		break;

	case 'SACS': /* SCAS */
		rc = EsifSetActionDelegateScas();
		break;

	case 'SSPB': /* BPSS - Set Biometric Presence Sensor preference*/
		rc = EsifSetActionDelegateBpss(requestPtr);
		break;

	default:
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}
exit:
	return rc;
}

static eEsifError EsifGetActionDelegateTemp(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	EsifPrimitiveTuple tempTuple = {GET_TEMPERATURE_SUR, 0, 255};

	ESIF_ASSERT(NULL != domainPtr);
	ESIF_ASSERT(NULL != responsePtr);

	tempTuple.domain = domainPtr->domain;
	return EsifUp_ExecutePrimitive(domainPtr->upPtr, &tempTuple, requestPtr, responsePtr);
}

static eEsifError EsifSetActionDelegateSphb(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	u32 tempHysteresis = ESIF_DOMAIN_TEMP_INVALID;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	tempHysteresis = *(u32 *)requestPtr->buf_ptr;

	rc = EsifUpDomain_SetTempHysteresis(domainPtr, tempHysteresis);

	ESIF_TRACE_DEBUG("Set Hysteresis = %d\n", esif_temp_abs_to_rel(tempHysteresis));

exit:
	return rc;
}

static eEsifError EsifSetActionDelegatePat0(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple auxTuple = { SET_TEMPERATURE_THRESHOLDS_SUR, 0, (UInt8)ESIF_DOMAIN_AUX0 };
	u32 tempThreshold = ESIF_DOMAIN_TEMP_INVALID;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	tempThreshold = *(u32 *)requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting AUX0 = %d\n", tempThreshold);

	EsifUpDomain_SetTempThresh(domainPtr, ESIF_DOMAIN_AUX0, tempThreshold);

	auxTuple.domain = domainPtr->domain;
	rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &auxTuple, requestPtr, NULL);
	if ((rc == ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP) || (domainPtr->tempPollPeriod > 0))
		rc = ESIF_OK;
exit:
	return rc;
}

static eEsifError EsifSetActionDelegatePat1(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	u32 tempThreshold = ESIF_DOMAIN_TEMP_INVALID;
	EsifPrimitiveTuple auxTuple = {SET_TEMPERATURE_THRESHOLDS_SUR, 0, (UInt8)ESIF_DOMAIN_AUX1};

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	
	tempThreshold = *(u32 *) requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting AUX1 = %d\n", tempThreshold);
	
	EsifUpDomain_SetTempThresh(domainPtr, ESIF_DOMAIN_AUX1, tempThreshold);

	auxTuple.domain = domainPtr->domain;
	rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &auxTuple, requestPtr, NULL);
	if ((rc == ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP) || (domainPtr->tempPollPeriod > 0))
		rc = ESIF_OK;
exit:
	return rc;
}

static eEsifError EsifSetActionDelegateRset(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
	EsifPrimitiveTupleParameter parameters = { 0 };
	EsifPrimitiveTuple tuple = { 0 };
	Bool signal_event = ESIF_FALSE;
	char domain_str[8] = { 0 };
	int j = 0;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);
	
	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	if (requestPtr->data_len != sizeof(parameters)) {
		rc = ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS;
		goto exit;
	}
	
	// Convert BINARY Parameters to Primitive Tuple
	esif_ccb_memcpy(&parameters, requestPtr->buf_ptr, sizeof(parameters));
	
	ESIF_TRACE_DEBUG("CONFIG RESET: { %s (%hd), %s, %hd }\n",
		esif_primitive_str(parameters.id.integer.value),
		(u16)parameters.id.integer.value,
		esif_primitive_domain_str((u16)parameters.domain.integer.value, domain_str, sizeof(domain_str)),
		(u16)parameters.instance.integer.value
		);

	// Look up Primitive Tuple in the DSP and verify it is a valid SET primtive
	EsifDspPtr dspPtr = EsifUp_GetDsp(domainPtr->upPtr);
	if (dspPtr == NULL) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}
	tuple.id = (u16) parameters.id.integer.value;
	tuple.domain = (u16) parameters.domain.integer.value;
	tuple.instance = (u16) parameters.instance.integer.value;
	EsifFpcPrimitivePtr primitivePtr = dspPtr->get_primitive(dspPtr, &tuple);
	if (primitivePtr == NULL) {
		rc = ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;
		goto exit;
	}
	if (primitivePtr->operation != ESIF_PRIMITIVE_OP_SET) {
		rc = ESIF_E_INVALID_REQUEST_TYPE;
		goto exit;
	}

	// Find first CONFIG Action and Delete its Key from its DataVault
	for (j = 0; j < (int)primitivePtr->num_actions; j++) {
		EsifFpcActionPtr fpcActionPtr = dspPtr->get_action(dspPtr, primitivePtr, (u8)j);
		DataItemPtr paramDataVault = EsifFpcAction_GetParam(fpcActionPtr, (const UInt8)0);
		DataItemPtr paramKeyName = EsifFpcAction_GetParam(fpcActionPtr, (const UInt8)1);
		DataItemPtr paramSignalEvent = NULL;
		if (fpcActionPtr->param_valid[3]) {
			paramSignalEvent = EsifFpcAction_GetParam(fpcActionPtr, (const UInt8)3);
		}
		if (fpcActionPtr->type != ESIF_ACTION_CONFIG) {
			continue;
		}
		if (paramDataVault == NULL || paramKeyName == NULL || paramDataVault->data_type != ESIF_DSP_PARAMETER_TYPE_STRING || paramKeyName->data_type != ESIF_DSP_PARAMETER_TYPE_STRING) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}
		
		// Valid SET CONFIG Primitive found with valid DV/Key Name; Delete the associated Key(s) from the DataVault.
		// Delete multiple keys to support GDDV Objects that contain non-IETM Primary Participants
		EsifString primaryParticipants[] = { ESIF_PARTICIPANT_DPTF_NAME, "DPTFZ", NULL };

		for (size_t part = 0; (part == 0 || (EsifUp_IsPrimaryParticipant(domainPtr->upPtr) && primaryParticipants[part - 1] != NULL)); part++) {
			// Replace "%nm%" tokens in the key name or make a copy of the key name for static keys
			EsifString expandedKeyName = NULL;
			if (part == 0) {
				expandedKeyName = EsifUp_CreateTokenReplacedParamString(domainPtr->upPtr, primitivePtr, (StringPtr)paramKeyName->data);
			}
			else {
				char nameTag[ESIF_NAME_LEN] = { 0 };
				esif_ccb_sprintf(sizeof(nameTag), nameTag, "%s.D0", primaryParticipants[part - 1]);
				expandedKeyName = esif_str_replace((StringPtr)paramKeyName->data, "%nm%", nameTag);
			}
			if (expandedKeyName == NULL) {
				expandedKeyName = esif_ccb_strdup((StringPtr)paramKeyName->data);
			}

			EsifDataPtr data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, (StringPtr)paramDataVault->data, 0, ESIFAUTOLEN);
			EsifDataPtr data_key = EsifData_CreateAs(ESIF_DATA_STRING, expandedKeyName, 0, ESIFAUTOLEN);

			if (expandedKeyName == NULL || data_nspace == NULL || data_key == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			// Do not signal an Event if Key does not exist in DataVault
			else if (DataBank_KeyExists((StringPtr)paramDataVault->data, expandedKeyName) == ESIF_FALSE) {
				rc = ESIF_OK;
			}
			else {
				// Delete Existing Key from DataVault
				rc = EsifConfigDelete(data_nspace, data_key);
				if (rc == ESIF_OK) {
					signal_event = ESIF_TRUE;
				}

				ESIF_TRACE_DEBUG("CONFIG RESET: config delete @%s %s [rc=%s (%d)]\n",
					(StringPtr)data_nspace->buf_ptr,
					expandedKeyName,
					esif_rc_str(rc),
					rc
				);
			}
			EsifData_Destroy(data_nspace);
			EsifData_Destroy(data_key);
			esif_ccb_free(expandedKeyName);
		}

		// Signal any Event associated with this SET Primitive
		if (signal_event) {
			esif_event_type_t signalEventId = ESIF_EVENT_NONE;
			if (paramSignalEvent && paramSignalEvent->data_type == ESIF_DSP_PARAMETER_TYPE_VARIANT && (size_t)(paramSignalEvent->data_length_in_bytes) == sizeof(esif_event_type_t)) {
				signalEventId = *(esif_event_type_t *)paramSignalEvent->data;
			}
			rc = EsifActConfigSignalChangeEvents(domainPtr->upPtr, tuple, NULL, signalEventId);
		}
		break;
	}
	if (j >= (int)primitivePtr->num_actions) {
		rc = ESIF_E_UNSUPPORTED_ACTION_TYPE;
	}

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateActl(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	const EsifFpcActionPtr fpcActionPtr)
{
	eEsifError rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
	EsifData p2 = { 0 };
	EsifString appName = NULL;
	UInt32 opcode = 0;

	UNREFERENCED_PARAMETER(domainPtr);

	ESIF_ASSERT(NULL != domainPtr);
	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr);

	// Action Parameter 2 is App Control Opcode ('STRT','STOP', etc)
	// Primitive requestPtr is App Name ("dptf", ...)
	rc = EsifFpcAction_GetParamAsEsifData(fpcActionPtr, 1, &p2);
	if ((ESIF_OK != rc) || (NULL == p2.buf_ptr)) {
		rc = ESIF_E_INVALID_ARGUMENT_COUNT;
		goto exit;
	}
	if ((p2.type == ESIF_DATA_UINT32) && (p2.data_len == sizeof(UInt32))) {
		opcode = *(UInt32 *)p2.buf_ptr;
	}

	if ((requestPtr->type == ESIF_DATA_STRING) && (NULL != requestPtr->buf_ptr)) {
		appName = (EsifString)requestPtr->buf_ptr;

		switch (opcode) {
		case DELEGATE_ACTL_OPCODE_STRT: // Start App
			rc = EsifAppMgr_AppStart(appName);
			break;
		case DELEGATE_ACTL_OPCODE_STOP: // Stop App
			rc = EsifAppMgr_AppStop(appName);
			break;
		default:
			rc = ESIF_E_INVALID_REQUEST_TYPE;
			break;
		}
	}

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateEvaluateParticipantCaps(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifDspPtr dspPtr = NULL;
	EsifUpPtr upPtr = NULL;
	eEsifError iterRc = ESIF_OK;
	EsifFpcDomainIterator dspDomainiterator = { 0 };
	UInt8 currentDomainIndex = 0;
	UInt16 targetDomain = 0;
	EsifFpcDomainPtr fpcDomainPtr = NULL;
	
	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(domainPtr->upPtr != NULL);

	UNREFERENCED_PARAMETER(requestPtr);
	
	targetDomain = domainPtr->domain;

	upPtr = domainPtr->upPtr; 
	dspPtr = EsifUp_GetDsp(upPtr);

	if (NULL == dspPtr) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	if ((NULL == dspPtr->init_fpc_iterator) ||
		(NULL == dspPtr->get_next_fpc_domain)) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	iterRc = dspPtr->init_fpc_iterator(dspPtr, &dspDomainiterator);
	if (ESIF_OK != iterRc) {
		goto exit;
	}

	iterRc = dspPtr->get_next_fpc_domain(dspPtr, &dspDomainiterator, &fpcDomainPtr);
	while (ESIF_OK == iterRc) {
		if (NULL == fpcDomainPtr) {
			iterRc = dspPtr->get_next_fpc_domain(dspPtr, &dspDomainiterator, &fpcDomainPtr);
			currentDomainIndex++;
			continue;
		}
		
		if ((UInt16) fpcDomainPtr->descriptor.domain == targetDomain) {
			/* Reset capabilities on the domain */
			EsifUpDomain_EnableCaps(domainPtr, fpcDomainPtr->capability_for_domain.capability_flags, fpcDomainPtr->capability_for_domain.capability_mask);
		}
		
		iterRc = dspPtr->get_next_fpc_domain(dspPtr, &dspDomainiterator, &fpcDomainPtr);
		currentDomainIndex++;
	}

	/* Perform capability detection */
	rc = EsifUpDomain_DspReadyInit(domainPtr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/*
	 * Remove and re-add participant from app to re-establish capabilities and
	 * start polling temperature if necessary
	 */
	rc = EsifAppMgr_DestroyParticipantInAllApps(upPtr);
	if (ESIF_OK == rc) {
		rc = EsifAppMgr_CreateParticipantInAllApps(upPtr);
	}

exit:
	return rc;
}

static eEsifError EsifGetActionDelegateGtt0(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	*(u32 *) responsePtr->buf_ptr = (u32) domainPtr->tempAux0;
exit:
	return rc;
}

static eEsifError EsifGetActionDelegateGtt1(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	*(u32 *) responsePtr->buf_ptr = (u32) domainPtr->tempAux1;
exit:
	return rc;
}

static eEsifError EsifGetActionDelegateVirtualTemperature(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	*(u32 *)responsePtr->buf_ptr = (u32)domainPtr->virtTemp;
exit:
	return rc;
}

static eEsifError EsifGetActionDelegateCnfg(
	const EsifUpDomainPtr domainPtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	static esif_ccb_spinlock_t spinlock = ATOMIC_INIT(0); // Static: No init/uninit necessary
	static DCfgOptions lastDcfg = { .asU32 = 0 };
	static UInt8 lastGddvHash[SHA256_HASH_BYTES] = { 0 };
	static eEsifError lastGddvError = ESIF_E_NO_LOWER_FRAMEWORK;
	extern int g_shell_enabled; // ESIF Shell Enabled Flag
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple dcfgTuple = { GET_CONFIG_ACCESS_CONTROL_SUR, 0, 255 };
	EsifPrimitiveTuple gddvTuple = { GET_CONFIG_DATAVAULT_SUR, 0, 255 };
	EsifData dcfgData = { ESIF_DATA_UINT32, NULL, ESIF_DATA_ALLOCATE, 0 };
	EsifData gddvData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };
	EsifData p2 = { 0 };
	UInt32 opcode = 0;
	Bool reloadApp = ESIF_FALSE;
	Bool reloadParts = ESIF_FALSE;

	ESIF_ASSERT(NULL != domainPtr);
	ESIF_ASSERT(NULL != requestPtr);

	// Get Optional Parameter 2
	rc = EsifFpcAction_GetParamAsEsifData(fpcActionPtr, 1, &p2);
	if ((ESIF_OK == rc) && (NULL != p2.buf_ptr) && (p2.buf_len == sizeof(opcode))) {
		opcode = *((UInt32 *)p2.buf_ptr);
	}

	dcfgTuple.domain = domainPtr->domain;
	gddvTuple.domain = domainPtr->domain;

	// Execute DCFG to read Access Control List Bitmask from BIOS, if it exists
	if (opcode != DELEGATE_CNFG_OPCODE_DESC) {
		rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &dcfgTuple, requestPtr, &dcfgData);
		if (rc == ESIF_OK && dcfgData.buf_ptr != NULL && dcfgData.buf_len >= sizeof(UInt32)) {
			DCfgOptions newmask = { .asU32 = *(UInt32 *)dcfgData.buf_ptr };
			DCfg_Set(newmask);

			ESIF_TRACE_INFO("DCFG Loaded: 0x%08X\n", newmask.asU32);

			// Disable ESIF Shell if Access Control forbids it
			if (DCfg_Get().opt.ShellAccessControl) {
				esif_uf_os_shell_disable();
				g_shell_enabled = 0;
			}

			// Check if Configuration Changed by comparing to last DCFG value
			esif_ccb_spinlock_lock(&spinlock);
			if (lastDcfg.asU32 != newmask.asU32) {
				lastDcfg.asU32 = newmask.asU32;
				reloadApp = ESIF_TRUE;
			}
			esif_ccb_spinlock_unlock(&spinlock);
		}
	}

	// Execute GDDV to read DataVault from BIOS, if it exists
	rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &gddvTuple, requestPtr, &gddvData);
	if (rc != ESIF_OK) {
		// Always Return OK if no ESIF_LF, no GDDV object in BIOS, or IETM Participant not yet loaded
		if (rc == ESIF_E_NO_LOWER_FRAMEWORK ||		// No LF Loaded or Linux-based OS
			rc == ESIF_E_NO_ACPI_SUPPORT ||			// LF Loaded, No IETM Participant yet
			rc == ESIF_E_ACPI_OBJECT_NOT_FOUND) {	// LF Loaded, IETM Participant Loaded, no GDDV object
			
			// Check if IETM participant created by comparing to previous error
			esif_ccb_spinlock_lock(&spinlock);
			if (rc == ESIF_E_ACPI_OBJECT_NOT_FOUND && rc != lastGddvError) {
				reloadParts = ESIF_TRUE;
				reloadApp = ESIF_TRUE;
			}
			lastGddvError = rc;
			esif_ccb_spinlock_unlock(&spinlock);
			rc = ESIF_OK;
		}
	}
	else {
		DataRepoPtr repo = DataRepo_CreateAs(StreamNull, StoreReadOnly, DataBank_GetDefault());
		if (repo == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else if (NULL == gddvData.buf_ptr || gddvData.data_len <= (u32)sizeof(union esif_data_variant)) {
			ESIF_TRACE_ERROR("Invalid GDDV Object [length=%u]\n", gddvData.data_len);
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else {
			// Support GDDV Objects that start with either DV Signature or esif_data_variant header
			u32 skipbytes = (DataVault_IsValidSignature(*(UInt16 *)gddvData.buf_ptr) || EsifData_IsCompressed(&gddvData) ? 0 : sizeof(union esif_data_variant));
			BytePtr gddvBufPtr = (BytePtr)gddvData.buf_ptr + skipbytes;
			u32 gddvBufLen = gddvData.data_len - skipbytes;
			EsifDataPtr gddvBuffer = EsifData_CreateAs(ESIF_DATA_BLOB, gddvBufPtr, 0, gddvBufLen);

			// Entire GDDV object can be compressed, including DV header
			if (gddvBuffer == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else if (EsifData_IsCompressed(gddvBuffer)) {
				rc = EsifData_Decompress(gddvBuffer);
			}

			if (rc == ESIF_OK && IOStream_SetMemory(repo->stream, StoreReadOnly, gddvBuffer->buf_ptr, gddvBuffer->data_len) == EOK) {
				if (opcode == DELEGATE_CNFG_OPCODE_DESC) {
					// Return GDDV Comment from first Header in Repo
					DataRepoInfo info = { 0 };
					rc = DataRepo_GetInfo(repo, &info);
					if (rc == ESIF_OK) {
						rc = DataRepo_ValidateSegment(repo);
					}
					if (rc == ESIF_OK) {
						UInt32 data_len = (UInt32)esif_ccb_strlen(info.comment, sizeof(info.comment)) + 1;
						if (responsePtr == NULL) {
							rc = ESIF_E_PARAMETER_IS_NULL;
						}
						else if (responsePtr->buf_ptr && responsePtr->buf_len >= data_len) {
							esif_ccb_strcpy(responsePtr->buf_ptr, info.comment, responsePtr->buf_len);
							responsePtr->data_len = data_len;
						}
						else {
							rc = ESIF_E_NEED_LARGER_BUFFER;
							responsePtr->data_len = data_len;
						}
					}
				}
				else {
					rc = DataRepo_LoadSegments(repo);
				}
			}

			// Check if configuration changed by comparing to last GDDV Hash
			if (rc == ESIF_OK && opcode != DELEGATE_CNFG_OPCODE_DESC) {
				esif_sha256_t gddvHash = { 0 };
				esif_sha256_init(&gddvHash);
				esif_sha256_update(&gddvHash, gddvBuffer->buf_ptr, gddvBuffer->data_len);
				esif_sha256_finish(&gddvHash);

				esif_ccb_spinlock_lock(&spinlock);
				if (memcmp(lastGddvHash, gddvHash.hash, sizeof(lastGddvHash)) != 0) {
					esif_ccb_memcpy(lastGddvHash, gddvHash.hash, sizeof(lastGddvHash));
					reloadParts = ESIF_TRUE;
					reloadApp = ESIF_TRUE;
				}
				esif_ccb_spinlock_unlock(&spinlock);
			}
			EsifData_Destroy(gddvBuffer);
		}
		DataRepo_Destroy(repo);
	}

	// Reload all Dymamic Participants if Primary Participant arrived
	if (rc == ESIF_OK && reloadParts == ESIF_TRUE) {
		CreateDynamicParticipants();
	}

	// Reload all Apps if DCFG or GDDV Configuration changed and DSP Action Parameter 2 = 'REST'
	if (rc == ESIF_OK && reloadApp == ESIF_TRUE && opcode == DELEGATE_CNFG_OPCODE_REST) {
		rc = EsifAppMgr_AppRestartAll();
	}

	esif_ccb_free(dcfgData.buf_ptr);
	esif_ccb_free(gddvData.buf_ptr);
	return rc;
}

static eEsifError EsifGetActionDelegateProcBrandMod(
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	esif_ccb_cpuid_t cpuInfo = { 0 };
	char *cpuInfoStr = NULL;
	UInt32 brandStrOffset = 0;
	char brandString[ESIF_CPUID_BRAND_STR_LEN];
	UInt32 len = 0;
	char *brandMod = NULL;

	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	responsePtr->data_len = ESIF_CPUID_BRAND_MOD_LEN + 1; // +1 for NUL terminator
	if (responsePtr->buf_len < responsePtr->data_len) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		goto exit;
	}

	cpuInfo.leaf = ESIF_CPUID_LEAF_CPU_BRAND_STR_SUPPORT;
	esif_ccb_cpuid(&cpuInfo);

	// Check brand string support
	if (cpuInfo.eax <= ESIF_CPUID_LEAF_CPU_BRAND_STR_PART3) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	cpuInfoStr = (char *)&cpuInfo;
	len = sizeof(cpuInfo) - sizeof(cpuInfo.leaf);

	// The brand string is spread across three leaf's
	cpuInfo.leaf = ESIF_CPUID_LEAF_CPU_BRAND_STR_PART1;
	esif_ccb_cpuid(&cpuInfo);
	esif_ccb_memcpy(brandString, cpuInfoStr, len);

	brandStrOffset += len;
	cpuInfo.leaf = ESIF_CPUID_LEAF_CPU_BRAND_STR_PART2;
	esif_ccb_cpuid(&cpuInfo);
	esif_ccb_memcpy(brandString + brandStrOffset, cpuInfoStr, len);

	brandStrOffset += len;
	cpuInfo.leaf = ESIF_CPUID_LEAF_CPU_BRAND_STR_PART3;
	esif_ccb_cpuid(&cpuInfo);
	esif_ccb_memcpy(brandString + brandStrOffset, cpuInfoStr, len);

	ESIF_TRACE_INFO("Proc Brand Modifier [%s]\n", brandString);

	if (esif_ccb_strstr(brandString, ESIF_CPUID_BRAND_MOD_I3)) {
		brandMod = ESIF_CPUID_BRAND_MOD_I3;
	}
	else if (esif_ccb_strstr(brandString, ESIF_CPUID_BRAND_MOD_I5)) {
		brandMod = ESIF_CPUID_BRAND_MOD_I5;
	}
	else if (esif_ccb_strstr(brandString, ESIF_CPUID_BRAND_MOD_I7)) {
		brandMod = ESIF_CPUID_BRAND_MOD_I7;
	}
	else if (esif_ccb_strstr(brandString, ESIF_CPUID_BRAND_MOD_I9)) {
		brandMod = ESIF_CPUID_BRAND_MOD_I9;
	}
	else {
		brandMod = ESIF_CPUID_BRAND_MOD_NA;
	}

	esif_ccb_memcpy(responsePtr->buf_ptr, brandMod, ESIF_CPUID_BRAND_MOD_LEN + 1);

exit:
	return rc;
}

static eEsifError EsifGetActionDelegateSupportSocWorkload(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	char brandMod[ESIF_CPUID_BRAND_MOD_LEN + 1];
	UInt8 isSupported = ESIF_FALSE;
	EsifPrimitiveTuple brandModTuple = { GET_PROC_BRAND_MODIFIER, ESIF_PRIMITIVE_DOMAIN_D0, ESIF_INSTANCE_INVALID };
	struct esif_data esifBrandmod = {
		ESIF_DATA_STRING,
		&brandMod,
		sizeof(brandMod),
		sizeof(brandMod)
	};

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	responsePtr->data_len = sizeof(isSupported);
	if (responsePtr->buf_len < responsePtr->data_len) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		goto exit;
	}

	rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &brandModTuple, requestPtr, &esifBrandmod);
	if (rc != ESIF_OK) {
		goto exit;
	}

	if (!esif_ccb_strncmp(brandMod, ESIF_CPUID_BRAND_MOD_I5, ESIF_CPUID_BRAND_MOD_LEN) ||
		!esif_ccb_strncmp(brandMod, ESIF_CPUID_BRAND_MOD_I7, ESIF_CPUID_BRAND_MOD_LEN) ||
		!esif_ccb_strncmp(brandMod, ESIF_CPUID_BRAND_MOD_I9, ESIF_CPUID_BRAND_MOD_LEN)) {
		isSupported = ESIF_TRUE;
	}

	*((UInt8 *)responsePtr->buf_ptr) = isSupported;

exit:
	return rc;
}

static eEsifError EsifGetActionDelegateIsFaceDetectionCapableSensor(
	EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	*(u32 *)responsePtr->buf_ptr = (u32)(esif_is_face_detection_capable_sensor());

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateSampleBehavior(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	u32 samplePeriod = 0;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	samplePeriod = *(u32 *)requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting Sample Period = %d\n", samplePeriod);

	EsifUpDomain_SetTempPollPeriod(domainPtr, samplePeriod);

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateVirtualTemperature(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	u32 virtTemp = ESIF_DOMAIN_TEMP_INVALID;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	virtTemp = *(u32 *)requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting Virtual Temp = %d\n", virtTemp);

	EsifUpDomain_SetVirtualTemperature(domainPtr, virtTemp);

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateToSignalOSEvent(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	eEsifEventType eventType)
{
	eEsifError rc = ESIF_OK;
	u32 updatedValue = 0;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	updatedValue = *(u32 *)requestPtr->buf_ptr;
	EsifUpDomain_SignalOSEvent(domainPtr, updatedValue, eventType);

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateToSignalForegroundAppChanged(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	EsifString appName = NULL;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	appName = (EsifString)requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting Foreground App = %s\n", appName);

	EsifUpDomain_SignalForegroundAppChanged(domainPtr, appName);

exit:
	return rc;
}

/* Non-public function used by EsifSetActionDelegateSsap */
eEsifError EsifUp_ExecuteSpecificActionPrimitive(
	EsifUpPtr self,
	EsifPrimitiveTuplePtr tuplePtr,
	const EsifPrimitiveActionSelectorPtr selectorPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
);


static eEsifError EsifSetActionDelegateSsap(
	EsifUpPtr upPtr,
	EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifSpecificActionRequestPtr sarPtr = NULL;

	ESIF_ASSERT(requestPtr != NULL);

	if ((NULL == requestPtr->buf_ptr) || (requestPtr->buf_len < sizeof(*sarPtr))) {
		rc = ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS;
		goto exit;
	}

	sarPtr = (EsifSpecificActionRequestPtr)requestPtr->buf_ptr;

	rc = EsifUp_ExecuteSpecificActionPrimitive(upPtr, &sarPtr->tuple, &sarPtr->selector, sarPtr->req_ptr, sarPtr->rsp_ptr);
	if (ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP == rc) {
		rc = ESIF_E_PRIMITIVE_SUR_NOT_FOUND_IN_DSP;
	}
exit:
	return rc;
}



static eEsifError EsifSetActionDelegateSsme(
	EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	eEsifEventType *eventPtr = NULL;

	ESIF_ASSERT(requestPtr != NULL);

	if ((NULL == requestPtr->buf_ptr) || (requestPtr->buf_len < sizeof(*eventPtr))) {
		rc = ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS;
		goto exit;
	}

	eventPtr = (eEsifEventType *)requestPtr->buf_ptr;

	rc = EsifEventMgr_FilterEventType(*eventPtr);
exit:
	return rc;
}


static eEsifError EsifSetActionDelegateScsm(
	EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	eEsifEventType *eventPtr = NULL;

	ESIF_ASSERT(requestPtr != NULL);

	if ((NULL == requestPtr->buf_ptr) || (requestPtr->buf_len < sizeof(*eventPtr))) {
		rc = ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS;
		goto exit;
	}

	eventPtr = (eEsifEventType *)requestPtr->buf_ptr;

	rc = EsifEventMgr_UnfilterEventType(*eventPtr);
exit:
	return rc;
}


static eEsifError EsifSetActionDelegateScas()
{
	return EsifEventMgr_UnfilterAllEventTypes();
}


/* Get Biometric Presence Sensor selection setting */
static eEsifError EsifGetActionDelegateBpsg(
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 setting = 0;

	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	responsePtr->data_len = sizeof(u32);
	if (responsePtr->buf_len < responsePtr->data_len) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		goto exit;
	}

	rc = esif_get_bp_sensor_instance(&setting);
	if (ESIF_OK == rc) {
		*(u32 *)responsePtr->buf_ptr = setting;
	}
exit:
	return rc;
}


/* Set Biometric Presence Sensor selection setting */
static eEsifError EsifSetActionDelegateBpss(
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	u32 value = 0;

	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (requestPtr->buf_len < sizeof(value)) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	value = *(u32 *)requestPtr->buf_ptr;

	rc = esif_set_bp_sensor_instance(value);
exit:
	return rc;
}



/*
 *******************************************************************************
 ** Register ACTION with ESIF
 *******************************************************************************
 */
static EsifActIfaceStatic g_delegate = {
	eIfaceTypeAction,
	ESIF_ACT_IFACE_VER_STATIC,
	sizeof(g_delegate),
	ESIF_ACTION_DELEGATE,
	ESIF_ACTION_FLAGS_DEFAULT,
	"DELEGATE",
	"Delegate Action",
	ESIF_ACTION_VERSION_DEFAULT,
	NULL,
	NULL,
	ActionDelegateGet,
	ActionDelegateSet
};

eEsifError EsifActDelegateInit()
{
	EsifActMgr_RegisterAction((EsifActIfacePtr)&g_delegate);
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}

void EsifActDelegateExit()
{
	EsifActMgr_UnregisterAction((EsifActIfacePtr)&g_delegate);
	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
