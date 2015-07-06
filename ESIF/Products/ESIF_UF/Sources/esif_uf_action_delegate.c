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
#include "esif_uf.h"			/* Upper Framework */
#include "esif_uf_actmgr.h"		/* Action Manager */
#include "esif_uf_domain.h"
#include "esif_uf_primitive.h"
#include "esif_uf_cfgmgr.h"
#include "esif_lib_databank.h"

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

static eEsifError EsifGetActionDelegateGddv(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifSetActionDelegateSphb(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);
	
static eEsifError EsifSetActionDelegatePat0(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegatePat1(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateSampleBehavior(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateVirtualTemperature(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

/*
** Handle ESIF Action Request
*/

static eEsifError ESIF_CALLCONV ActionDelegateGet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData p1 = {0};
	UInt32 method;
	EsifUpDomainPtr domainPtr = NULL;

	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(upPtr);
	UNREFERENCED_PARAMETER(primitivePtr);
	UNREFERENCED_PARAMETER(actionPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	domainPtr = EsifUp_GetDomainById(upPtr, primitivePtr->tuple.domain);
	if (NULL == domainPtr) {
		ESIF_TRACE_ERROR("Unable to get domain\n");
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	rc = EsifActionGetParamAsEsifData(actionPtr, 0, &p1);
	if (ESIF_OK != rc) {
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

	case 'VDDG': /* GDDV */
		rc = EsifGetActionDelegateGddv(domainPtr, requestPtr, responsePtr);
		break;

	case 'PMTV': /* VTMP */
		rc = EsifGetActionDelegateVirtualTemperature(domainPtr, responsePtr);
		break;

	default:
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}
exit:
	return rc;
}

static eEsifError ESIF_CALLCONV ActionDelegateSet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData p1 = {0};
	UInt32 method;
	EsifUpDomainPtr domainPtr = NULL;

	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(upPtr);
	UNREFERENCED_PARAMETER(primitivePtr);
	UNREFERENCED_PARAMETER(actionPtr);

	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr->buf_ptr);
	
	domainPtr = EsifUp_GetDomainById(upPtr, primitivePtr->tuple.domain);
	if (NULL == domainPtr) {
		ESIF_TRACE_ERROR("Unable to get domain\n");
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	rc = EsifActionGetParamAsEsifData(actionPtr, 0, &p1);
	if (ESIF_OK != rc) {
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

	ESIF_TRACE_DEBUG("Setting Hysterisis = %d\n", tempHysteresis);

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

static eEsifError EsifGetActionDelegateGddv(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple surrogateTuple = {GET_DPTF_CONFIGURATION_SUR, 0, 255};
	EsifData surrogateData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	ESIF_ASSERT(NULL != domainPtr);
	ESIF_ASSERT(NULL != requestPtr);

	UNREFERENCED_PARAMETER(responsePtr); // Not currently used by DPTF

	surrogateTuple.domain = domainPtr->domain;
	rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &surrogateTuple, requestPtr, &surrogateData);
	if (rc != ESIF_OK) {
		// Always Return OK to DPTF if no ESIF_LF or GDDV object in BIOS
		if (rc == ESIF_E_NO_LOWER_FRAMEWORK || rc == ESIF_E_ACPI_OBJECT_NOT_FOUND) {
			rc = ESIF_OK;
		}
	}
	else {
		char *dv_name = "__merge"; // Temporary DV Name
		DataVaultPtr DB = DataBank_GetNameSpace(g_DataBankMgr, dv_name);
		if (DB != NULL) {
			DataBank_CloseNameSpace(g_DataBankMgr, dv_name);
			DB = NULL;
		}
		if (DB == NULL) {
			DB = DataBank_OpenNameSpace(g_DataBankMgr, dv_name);

			// Load Datavault into temporary namespace. DV may or may not be preceded by a variant
			if (DB) {
				u32 skipbytes = (memcmp(surrogateData.buf_ptr, "\xE5\x1F", 2) == 0 ? 0 : sizeof(union esif_data_variant));
				void *buffer = esif_ccb_malloc(surrogateData.data_len);
				esif_ccb_memcpy(buffer, (u8*)surrogateData.buf_ptr + skipbytes, surrogateData.data_len - skipbytes);
				IOStream_SetMemory(DB->stream, buffer, surrogateData.data_len - skipbytes);
				if ((rc = DataVault_ReadVault(DB)) != ESIF_OK) {
					DataBank_CloseNameSpace(g_DataBankMgr, dv_name);
					ESIF_TRACE_DEBUG("Unable to Open DataVault: %s\n", esif_rc_str(rc));
					rc = ESIF_OK;
				}
				else {
					EsifDataPtr data_nspace = NULL;
					EsifDataPtr data_key = NULL;
					EsifDataPtr data_targetdv = NULL;
					esif_flags_t options = 0; // NOPERSIST
					esif_string keyspec = "*"; // Merge All Keys
					esif_string targetdv = g_DataVaultDefault;

					DB->flags |= (ESIF_SERVICE_CONFIG_READONLY);
					ESIF_TRACE_DEBUG("DV Opened: %s\n", dv_name);

					// Merge Contents into Default DataVault
					data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, dv_name, 0, ESIFAUTOLEN);
					data_targetdv = EsifData_CreateAs(ESIF_DATA_STRING, targetdv, 0, ESIFAUTOLEN);
					data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
					if (data_nspace == NULL || data_key == NULL || data_targetdv == NULL) {
						rc = ESIF_E_NO_MEMORY;
					}
					else {
						rc = EsifConfigCopy(data_nspace, data_targetdv, data_key, options, ESIF_FALSE);
					}
					EsifData_Destroy(data_nspace);
					EsifData_Destroy(data_key);
					EsifData_Destroy(data_targetdv);
					DataBank_CloseNameSpace(g_DataBankMgr, dv_name);
				}
				esif_ccb_free(buffer);
			}
		}
	}
	esif_ccb_free(surrogateData.buf_ptr);
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

/*
 *******************************************************************************
 ** Register ACTION with ESIF
 *******************************************************************************
 */
static EsifActType g_delegate = {
	0,
	ESIF_ACTION_DELEGATE,
	{PAD},
	"DELEGATE",
	"Delegate Action",
	"ALL",
	"x1.0.0.1",
	{0},
	ESIF_ACTION_IS_NOT_KERNEL_ACTION,
	ESIF_ACTION_IS_NOT_PLUGIN,
	{PAD},
	ActionDelegateGet,
	ActionDelegateSet,
	NULL,
	NULL
};

eEsifError EsifActDelegateInit()
{
	if (NULL != g_actMgr.AddActType) {
		g_actMgr.AddActType(&g_actMgr, &g_delegate);
	}
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}

void EsifActDelegateExit()
{
	if (NULL != g_actMgr.RemoveActType) {
		g_actMgr.RemoveActType(&g_actMgr, 0);
	}
	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
