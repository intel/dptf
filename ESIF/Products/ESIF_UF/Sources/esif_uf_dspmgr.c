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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_DSP

#include "esif_uf.h"			/* Upper Framework */
#include "esif_ipc.h"			/* IPC Abstraction */
#include "esif_dsp.h"			/* Device Support Package */
#include "esif_hash_table.h"	/* Hash Table */
#include "esif_uf_fpc.h"		/* Full Primitive Catalog */

#include "esif_lib_esifdata.h"
#include "esif_lib_databank.h"
#include "esif_uf_cfgmgr.h"
#include "esif_participant.h"

#define MAX_MINTERM_LENGTH 256	/* Max Length for Minterm Match Lists */

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// Limits
#define MAX_EDP_SIZE    0x7ffffffe
#define MAX_FPC_SIZE    0x7ffffffd

#define MIN_VIABLE_DSP_WEIGHT 1

enum dspSelectorWeight {
	ACPI_HID_WEIGHT = 10,
	ACPI_PTYPE_WEIGHT = 18,
	ACPI_UID_WEIGHT = 16,
	PCI_DEVICE_ID_WEIGHT = 20,
	PCI_VENDOR_ID_WEIGHT = 2,
	ENUMERATOR_TYPE_WEIGHT = 4,
	PARTICIPANT_NAME_WEIGHT = 8,
};

static int compare_and_weight_minterm(
	esif_string dsp,
	esif_string qry,
	enum dspSelectorWeight weight
	);

EsifString GetDSPFromList(
	EsifString name
	);


/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */

/*
** DSP Manager. Keeps Track of all the Device Support Packages
** and their attributes.
*/
EsifUfDm g_dm = {0};


/* Allocate DSP Upper Insance */
static EsifDspPtr esif_dsp_create(void)
{
	EsifDspPtr dspPtr = NULL;
	dspPtr = (EsifDspPtr)esif_ccb_malloc(sizeof(*dspPtr));
	return dspPtr;
}


/* Free DSP Upper Instance */
static void esif_dsp_destroy(EsifDspPtr dspPtr)
{
	if (NULL == dspPtr) {
		return;
	}
	esif_ht_destroy(dspPtr->ht_ptr, NULL);
	esif_link_list_destroy(dspPtr->algo_ptr);
	esif_link_list_destroy(dspPtr->domain_ptr);
	esif_link_list_destroy(dspPtr->cap_ptr);
	esif_link_list_destroy(dspPtr->evt_ptr);
	esif_ccb_free(dspPtr);
}


/* DSP Interface */
static esif_string get_code(EsifDspPtr dspPtr)
{
	return ((dspPtr != NULL) && (dspPtr->code_ptr != NULL)) ? dspPtr->code_ptr : "N/A";
}


static UInt8 get_ver_major(EsifDspPtr dspPtr)
{
	return ((dspPtr != NULL) && (dspPtr->ver_major_ptr != NULL)) ? *dspPtr->ver_major_ptr : 0;
}


static UInt8 get_ver_minor(EsifDspPtr dspPtr)
{
	return ((dspPtr != NULL) && (dspPtr->ver_minor_ptr != NULL)) ? *dspPtr->ver_minor_ptr : 0;
}


static UInt32 get_temp_c1(
	EsifDspPtr dspPtr,
	const enum esif_action_type action
	)
{
	/* TODO: Assume tempC1 Is Slope */
	EsifFpcAlgorithmPtr algoPtr = NULL;

	if (NULL == dspPtr) {
		return 0xffffffff;
	}

	algoPtr = dspPtr->get_algorithm(dspPtr, action);
	if (algoPtr) {
		return algoPtr->tempC1;
	} else {
		return 0xffffffff;
	}
}


static UInt32 get_percent_xform(
	EsifDspPtr dspPtr,
	const enum esif_action_type action
	)
{
	EsifFpcAlgorithmPtr algoPtr = NULL;

	if (NULL == dspPtr) {
		return 0xffffffff;
	}

	algoPtr = dspPtr->get_algorithm(dspPtr, action);
	if (algoPtr) {
		return algoPtr->percent_xform;
	} else {
		return 0xffffffff;
	}
}


/* Insert Primitive */
static eEsifError insert_primitive(
	EsifDspPtr dspPtr,
	EsifFpcPrimitive *primitivePtr
	)
{
	EsifFpcActionPtr actionPtr = NULL;
	unsigned int i = 0;
	int remainingSize;

	if (NULL == dspPtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}
	if (NULL == primitivePtr) {
		return ESIF_E_NULL_PRIMITIVE;
	} 

	remainingSize = (int)primitivePtr->size;
	remainingSize -= sizeof(*primitivePtr);

	/* Update the DSP metadata with the types of actions contained in the DSP */
	actionPtr = (EsifFpcActionPtr)(primitivePtr + 1);
	for (i = 0; i < primitivePtr->num_actions; i++) {
		if (remainingSize < sizeof(*actionPtr)) {
			break;
		}
		remainingSize -= actionPtr->size;

		if (actionPtr->type < (sizeof(dspPtr->contained_actions)/sizeof(*dspPtr->contained_actions)) &&
			(int)actionPtr->type >= 0) {
			dspPtr->contained_actions[actionPtr->type] = 1;
		}
		actionPtr = (EsifFpcActionPtr)((char*)actionPtr + actionPtr->size);
	}

	return esif_ht_add_item(dspPtr->ht_ptr, /* Hash Table  */
		(UInt8 *)&primitivePtr->tuple,	/* Key */
		sizeof(primitivePtr->tuple),/* Size Of Key */
		primitivePtr); /* Item        */
}

/* Get Primitive */
static EsifFpcPrimitivePtr get_primitive(
	EsifDspPtr dspPtr,
	const EsifPrimitiveTuplePtr tuplePtr
	)
{
	EsifFpcPrimitivePtr primitivePtr = NULL;

	if (NULL == dspPtr) {
		return NULL;
	}

	primitivePtr = (EsifFpcPrimitivePtr)
		esif_ht_get_item(dspPtr->ht_ptr,
		(UInt8 *)tuplePtr,
		sizeof(*tuplePtr));

	return primitivePtr;
}


/* Get i-th Action */
static EsifFpcActionPtr get_action(
	EsifDspPtr dspPtr,
	EsifFpcPrimitivePtr primitivePtr,
	UInt8 index
	)
{
	EsifFpcActionPtr fpcActionPtr = NULL;
	int i;

	if (NULL == dspPtr || NULL == primitivePtr) {
		return NULL;
	}

	/* First Action */
	fpcActionPtr = (EsifFpcActionPtr)(primitivePtr + 1);

	/* Locate i-th Action We Are Looking For */
	for (i = 0; i < index; i++) {
		fpcActionPtr = (EsifFpcActionPtr)((UInt8 *)fpcActionPtr + fpcActionPtr->size);
	}

	return fpcActionPtr;
}


/* Insert Algorithm Into Linked List */
static eEsifError insert_algorithm(
	EsifDspPtr dspPtr,
	EsifFpcAlgorithmPtr algoPtr
	)
{
	if ((NULL == dspPtr) || (NULL == algoPtr)) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	return esif_link_list_add_at_back(dspPtr->algo_ptr, (void *)algoPtr);
}


/* Insert Event Into Linked List */
static eEsifError insert_event(
	EsifDspPtr dspPtr,
	EsifFpcEventPtr evtPtr
	)
{
	if ((NULL == dspPtr) || (NULL == evtPtr)) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	return esif_link_list_add_at_back(dspPtr->evt_ptr, (void *)evtPtr);
}


/* Get Algorithm From Linked List */
static EsifFpcAlgorithmPtr get_algorithm(
	EsifDspPtr dspPtr,
	const enum esif_action_type actionType
	)
{
	struct esif_link_list *listPtr = NULL;
	struct esif_link_list_node *currPtr = NULL;
	EsifFpcAlgorithmPtr curAlgoPtr = NULL;

	if (NULL == dspPtr) {
		return NULL;
	}

	listPtr = dspPtr->algo_ptr;
	currPtr = listPtr->head_ptr;

	while (currPtr) {
		curAlgoPtr = (EsifFpcAlgorithmPtr)currPtr->data_ptr;
		if (curAlgoPtr != NULL) {
			if (curAlgoPtr->action_type == actionType) {
				return curAlgoPtr;
			}
		}
		currPtr = currPtr->next_ptr;
	}
	return NULL;
}


/* Get Event From Linked List By Type */
static EsifFpcEventPtr get_event_by_type(
	EsifDspPtr dspPtr,
	const enum esif_event_type eventType
	)
{
	struct esif_link_list *listPtr = NULL;
	struct esif_link_list_node *currPtr = NULL;
	EsifFpcEventPtr curEvtPtr = NULL;

	if (NULL == dspPtr) {
		return NULL;
	}

	listPtr = dspPtr->evt_ptr;
	currPtr = listPtr->head_ptr;

	while (currPtr) {
		curEvtPtr = (EsifFpcEventPtr)currPtr->data_ptr;
		if (curEvtPtr != NULL) {
			if (curEvtPtr->esif_event == eventType) {
				return curEvtPtr;
			}
		}
		currPtr = currPtr->next_ptr;
	}
	return NULL;
}


/* Get Event From Linked List By GUID */
EsifFpcEventPtr get_event_by_guid(
	EsifDspPtr dspPtr,
	const esif_guid_t guid
	)
{
	struct esif_link_list *listPtr      = NULL;
	struct esif_link_list_node *currPtr = NULL;
	EsifFpcEventPtr curEvtPtr  = NULL;

	if (NULL == dspPtr) {
		goto exit;
	}
	listPtr = dspPtr->evt_ptr;
	currPtr = listPtr->head_ptr;

	while (currPtr) {
		curEvtPtr = (EsifFpcEventPtr)currPtr->data_ptr;
		if (curEvtPtr != NULL) {
			if (memcmp(curEvtPtr->event_guid, guid, ESIF_GUID_LEN) == 0) {
				return curEvtPtr;
			}
		}
		currPtr = currPtr->next_ptr;
	}
exit:
	return NULL;
}


/* Insert Domain Into Linked List */
static eEsifError insert_domain(
	EsifDspPtr dspPtr,
	EsifFpcDomainPtr domainPtr
	)
{
	if ((NULL == dspPtr) || (NULL == domainPtr)) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	return esif_link_list_add_at_back(dspPtr->domain_ptr, (void *)domainPtr);
}


static UInt32 get_domain_count(EsifDspPtr dspPtr)
{
	return ((dspPtr != NULL) && (dspPtr->domain_count != NULL)) ? *dspPtr->domain_count : 0;
}


/* Get i-th Domain */
static EsifFpcDomainPtr get_domain(
	EsifDspPtr dspPtr,
	const UInt32 index
	)
{
	struct esif_link_list *listPtr = NULL;
	struct esif_link_list_node *currPtr = NULL;
	EsifFpcDomainPtr curDmnPtr = NULL;
	UInt32 i = 0;

	if (NULL == dspPtr) {
		goto exit;
	}

	listPtr = dspPtr->domain_ptr;
	currPtr = listPtr->head_ptr;

	while ((++i <= index) && (currPtr != NULL)) {
		currPtr = currPtr->next_ptr;
	}
	if (currPtr != NULL) {
		curDmnPtr = (EsifFpcDomainPtr)currPtr->data_ptr;
	}
exit:
	return curDmnPtr;
}

static eEsifError init_fpc_iterator(
	EsifDspPtr dspPtr,
	EsifFpcDomainIteratorPtr iteratorPtr
	)
{
	eEsifError rc = ESIF_OK;

	if ((NULL == dspPtr) || (NULL == iteratorPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_memset(iteratorPtr, 0, sizeof(*iteratorPtr));
	iteratorPtr->marker = FPC_DOMAIN_ITERATOR_MARKER;
	iteratorPtr->currPtr = dspPtr->domain_ptr->head_ptr;
exit:
	return rc;
}

/* See EsifUpDomain_InitIterator for usage */
static eEsifError get_next_fpc_domain(
	EsifDspPtr dspPtr,
	EsifFpcDomainIteratorPtr iteratorPtr,
	EsifFpcDomainPtr *fpcDomainPtr
	)
{
	eEsifError rc = ESIF_OK;

	if ((NULL == dspPtr) || (NULL == iteratorPtr) || (NULL == fpcDomainPtr)) {
		ESIF_TRACE_WARN("Parameter is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Verify the iterator is initialized */
	if (iteratorPtr->marker != FPC_DOMAIN_ITERATOR_MARKER) {
		ESIF_TRACE_WARN("Iterator invalid\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	if (iteratorPtr->currPtr == NULL) {
		*fpcDomainPtr = NULL;
		rc = ESIF_E_ITERATION_DONE;
	}
	else {
		*fpcDomainPtr = (EsifFpcDomainPtr) iteratorPtr->currPtr->data_ptr;
		iteratorPtr->currPtr = iteratorPtr->currPtr->next_ptr;
	}
	
exit:
	return rc;
}

eEsifError esif_fpc_load(
	EsifFpcPtr fpcPtr,
	EsifDspPtr dspPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifFpcDomainPtr domainPtr;
	EsifFpcPrimitivePtr primitivePtr;
	EsifFpcAlgorithmPtr algoPtr;
	EsifFpcEventPtr eventPtr;
	unsigned long offset = 0;
	UInt8 *basePtr    = (UInt8 *)fpcPtr;
	UInt32 numPrim    = 0;
	UInt32 i;
	UInt32 j;

	if ((fpcPtr == NULL) || (dspPtr == NULL))
	{
		ESIF_TRACE_ERROR("The fpc pointer or dsp pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (fpcPtr->number_of_domains < 1) {
		ESIF_TRACE_WARN("No domain error, number_of_domain = %d (less than 1)\n",
			fpcPtr->number_of_domains);
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	dspPtr->domain_count = &fpcPtr->number_of_domains;

	/* Allocate Hash and List. Hash Size 31 Is Hard-Coded And Chosen By DSP Compiler */
	dspPtr->ht_ptr     = esif_ht_create(ESIF_DSP_HASHTABLE_SIZE);
	dspPtr->algo_ptr   = esif_link_list_create();
	dspPtr->domain_ptr = esif_link_list_create();
	dspPtr->cap_ptr    = esif_link_list_create();
	dspPtr->evt_ptr    = esif_link_list_create();

	if (!dspPtr->ht_ptr || !dspPtr->algo_ptr || !dspPtr->domain_ptr || !dspPtr->cap_ptr || !dspPtr->evt_ptr) {
		ESIF_TRACE_ERROR("Fail to allocate linked list or hash table\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ESIF_TRACE_DEBUG("<fpc @ %p> FPC name '%s' ver %x,%x desc '%s' size %u num_domains %d num_algorithms %d num_events %d",
					 fpcPtr,
					 fpcPtr->header.name,
					 fpcPtr->header.ver_major,
					 fpcPtr->header.ver_minor,
					 fpcPtr->header.description,
					 fpcPtr->size,
					 fpcPtr->number_of_domains,
					 fpcPtr->number_of_algorithms,
					 fpcPtr->number_of_events);

	/* First Domain, Ok To Have Zero Primitive Of A Domain */
	domainPtr = (EsifFpcDomainPtr)(fpcPtr + 1);
	for (i = 0; i < fpcPtr->number_of_domains; i++) {
		offset = (unsigned long)((UInt8 *)domainPtr - basePtr);
		ESIF_TRACE_DEBUG("<%04lu> Domain[%d] name %s size %d num_of_primitives %d  "
						 "num_of_capabilites %u (0x%x)",
						 offset, i, domainPtr->descriptor.name, domainPtr->size,
						 domainPtr->number_of_primitives,
						 domainPtr->capability_for_domain.number_of_capability_flags,
						 domainPtr->capability_for_domain.capability_flags);

		/* Insert Domain Into Linked List */
		rc = dspPtr->insert_domain(dspPtr, domainPtr);
		if (ESIF_OK != rc) {
			ESIF_TRACE_ERROR("Fail to insert domain #%d\n", i);
			goto exit;
		}

		/* Capability */
		for (j = 0; j < domainPtr->capability_for_domain.number_of_capability_flags; j++) {
			offset = (unsigned long)(((UInt8 *)&domainPtr->capability_for_domain) - basePtr);
			ESIF_TRACE_DEBUG("<%04lu> Capability[%d] 0x%x", offset, j,
							 domainPtr->capability_for_domain.capability_mask[j]);
		}

		/* First Primtive */
		primitivePtr = (EsifFpcPrimitivePtr)(domainPtr + 1);
		for (j = 0; j < domainPtr->number_of_primitives; j++, numPrim++) {
			offset = (unsigned long)(((UInt8 *)primitivePtr) - basePtr);
			ESIF_TRACE_DEBUG("<%04lu> Primitive[%03d]: size %3d tuple_id <%03u %03u %03u> "
							 "operation %u(%s) req_type %u(%s) res_type %u(%s) num_actions %u",
							 offset, j,
							 primitivePtr->size,
							 primitivePtr->tuple.id,
							 primitivePtr->tuple.domain,
							 primitivePtr->tuple.instance,
							 primitivePtr->operation,
							 esif_primitive_opcode_str(primitivePtr->operation),
							 primitivePtr->request_type,
							 esif_data_type_str(primitivePtr->request_type),
							 primitivePtr->result_type,
							 esif_data_type_str(primitivePtr->result_type),
							 primitivePtr->num_actions);
			/* Insert Primitive Into Hash */
			rc = dspPtr->insert_primitive(dspPtr, primitivePtr);
			if (ESIF_OK != rc) {
				ESIF_TRACE_ERROR("Fail to insert primitive (id = %d)\n", primitivePtr->tuple.id);
				goto exit;
			}
			/* Next Primitive */
			primitivePtr = (EsifFpcPrimitivePtr)((UInt8 *)primitivePtr + primitivePtr->size);
		}
		/* Next Domain */
		domainPtr = (EsifFpcDomainPtr)((UInt8 *)domainPtr + domainPtr->size);
	}

	/* First Algorithm (Laid After The Last Domain) */
	algoPtr = (EsifFpcAlgorithmPtr)domainPtr;
	for (i = 0; i < fpcPtr->number_of_algorithms; i++) {
		offset = (unsigned long)((UInt8 *)algoPtr - basePtr);
		ESIF_TRACE_DEBUG("<%04lu> Algorithm[%03d]:  action_type %u(%s) temp_xform %u "
						 "tempC1 %u percent_xform %u size %u",
						 offset, i,
						 algoPtr->action_type,
						 esif_action_type_str(algoPtr->action_type),
						 algoPtr->temp_xform,
						 algoPtr->tempC1,
						 algoPtr->percent_xform,
						 algoPtr->size);

		/* Insert Algorithm Into Linked List */
		rc = dspPtr->insert_algorithm(dspPtr, algoPtr);
		if (ESIF_OK != rc) {
			ESIF_TRACE_ERROR("Fail to insert algorithm - %s\n", esif_action_type_str(algoPtr->action_type));
			goto exit;
		}

		/* Next Algorithm */
		algoPtr = (EsifFpcAlgorithmPtr)(algoPtr + 1);
	}

	/* First Event (Laid After The Last Algorithm) */
	eventPtr = (EsifFpcEventPtr)algoPtr;
	for (i = 0; i < fpcPtr->number_of_events; i++) {
		offset = (unsigned long)((UInt8 *)eventPtr - basePtr);
		ESIF_TRACE_DEBUG("<%04lu> Event [%03d] type %s(%d)\n",
						 offset, i, esif_event_type_str(eventPtr->esif_event), eventPtr->esif_event);

		/* Insert Algorithm Into Linked List */
		rc = dspPtr->insert_event(dspPtr, eventPtr);
		if (ESIF_OK != rc) {
			ESIF_TRACE_ERROR("Fail to insert event - %s\n", esif_event_type_str(eventPtr->esif_event));
			goto exit;
		}

		/* Next Event */
		eventPtr = (EsifFpcEventPtr)(eventPtr + 1);
	}

exit:
	if (fpcPtr != NULL) {
		ESIF_TRACE_DEBUG("%u domains, %u primitives and %u algorithms %u events inserted! status %s",
						 fpcPtr->number_of_domains, numPrim, fpcPtr->number_of_algorithms, fpcPtr->number_of_events,
						 esif_rc_str(rc));
	}
	return rc;
}


/* Add DSP Entry */
static eEsifError esif_dsp_entry_create(struct esif_ccb_file *file_ptr)
{
	eEsifError rc  = ESIF_E_UNSPECIFIED;
	EsifDspPtr dspPtr = NULL;
	EsifFpcPtr fpcPtr    = NULL;
	UInt32 fpcIsStatic = ESIF_FALSE;
	UInt8 i = 0;
	char path[MAX_PATH]={0};
	UInt32 fpcSize = 0;
	UInt32 edpSize = 0;
	size_t numFpcBytesRead = 0;
	struct edp_dir edp_dir;
	EsifDataPtr nameSpace = 0;
	EsifDataPtr key = 0;
	EsifDataPtr value = 0;
	IOStreamPtr ioPtr = IOStream_Create();

	if ((NULL == file_ptr) || (NULL == ioPtr)) {
		ESIF_TRACE_ERROR("The file pointer or IO stream is NULL\n");
		goto exit;
	}
	nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, ESIF_DSP_NAMESPACE, 0, ESIFAUTOLEN);
	key       = EsifData_CreateAs(ESIF_DATA_STRING, file_ptr->filename, 0, ESIFAUTOLEN);
	value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

	if (nameSpace == NULL || key == NULL || value == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Filename: %s", file_ptr->filename);

	dspPtr = esif_dsp_create();
	if (NULL == dspPtr) {
		ESIF_TRACE_ERROR("Fail to allocate dsp entry\n");
		goto exit;
	}

	// Look for EDP file on disk first then in DataVault (static or file)
	esif_build_path(path, sizeof(path), ESIF_PATHTYPE_DSP, file_ptr->filename, NULL);
	if (!esif_ccb_file_exists(path) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		esif_ccb_strcpy(path, file_ptr->filename, MAX_PATH);
		IOStream_SetMemory(ioPtr, StoreReadOnly, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(ioPtr, StoreReadOnly, path, "rb");
	}
	ESIF_TRACE_DEBUG("Fullpath: %s", path);

	if (IOStream_Open(ioPtr) != 0) {
		ESIF_TRACE_ERROR("File not found (%s)", path);
		goto exit;
	}

	/* Read FPC From EDP File */
	if (esif_ccb_strstr(&path[0], ".edp")) {
		/* EDP - Only Read The FPC Part */
		edpSize = (UInt32)IOStream_GetSize(ioPtr);
		if (!edpSize) {
			goto exit;
		}
		numFpcBytesRead = IOStream_Read(ioPtr, &edp_dir, sizeof(edp_dir));
		if (!esif_verify_edp(&edp_dir, numFpcBytesRead)) {
			ESIF_TRACE_ERROR("Invalid EDP Header: Signature=%4.4s Version=%d\n", (char *)&edp_dir.signature, edp_dir.version);
			goto exit;
		}
		if (edpSize > MAX_EDP_SIZE || edp_dir.fpc_offset > MAX_FPC_SIZE || edp_dir.fpc_offset > edpSize) {
			ESIF_TRACE_ERROR("The edp or fpc file size is larger than maximum\n");
			goto exit;
		}
		fpcSize = edpSize - edp_dir.fpc_offset;
		IOStream_Seek(ioPtr, edp_dir.fpc_offset, SEEK_SET);
		ESIF_TRACE_DEBUG("File found (%s) size %u, FPC size %u from offset %u", path, edpSize, fpcSize, edp_dir.fpc_offset);
	} else {
		ESIF_TRACE_DEBUG("File %s does not have .fpc and .edp format!", path);
	}

	// use static DataVault buffer (if available), otherwise allocate space for our FPC file contents (which will not be freed)
	if (IOStream_GetType(ioPtr) == StreamMemory && value->buf_len == 0) {
		fpcPtr = (EsifFpcPtr)IOStream_GetMemoryBuffer(ioPtr); 
		if (NULL == fpcPtr) {
			ESIF_TRACE_ERROR("NULL buffer");
			goto exit;
		}
		fpcPtr  = (EsifFpcPtr) (((BytePtr) fpcPtr) + IOStream_GetOffset(ioPtr));
		numFpcBytesRead = fpcSize;
		ESIF_TRACE_DEBUG("Static vault size %u buf_ptr=0x%p\n", (int)numFpcBytesRead, fpcPtr);
		fpcIsStatic = ESIF_TRUE;
	} else {
		fpcPtr = (EsifFpcPtr)esif_ccb_malloc(fpcSize);
		if (NULL == fpcPtr) {
			ESIF_TRACE_ERROR("malloc failed to allocate %u bytes\n", fpcSize);
			goto exit;
		}
		ESIF_TRACE_DEBUG("File malloc size %u", fpcSize);

		// read file contents
		numFpcBytesRead = IOStream_Read(ioPtr, fpcPtr, fpcSize);
		if (numFpcBytesRead < fpcSize) {
			ESIF_TRACE_ERROR("Read short received %u of %u bytes\n", (int)numFpcBytesRead, fpcSize);
			goto exit;
		}

		ESIF_TRACE_DEBUG("File read size %u", (int)numFpcBytesRead);
	}
	ESIF_TRACE_DEBUG("\nDecode Length:  %u", fpcPtr->size);
	ESIF_TRACE_DEBUG("Code:           %s", fpcPtr->header.code);
	ESIF_TRACE_DEBUG("Ver Major:      %u", fpcPtr->header.ver_major);
	ESIF_TRACE_DEBUG("Ver Minor:      %u", fpcPtr->header.ver_minor);
	ESIF_TRACE_DEBUG("Name:           %s", fpcPtr->header.name);
	ESIF_TRACE_DEBUG("Description:    %s", fpcPtr->header.description);
	ESIF_TRACE_DEBUG("Type:           %s", fpcPtr->header.type);
	ESIF_TRACE_DEBUG("Bus Enumerator: %u", fpcPtr->header.bus_enum);
	ESIF_TRACE_DEBUG("ACPI Device:    %s", fpcPtr->header.acpi_device);
	ESIF_TRACE_DEBUG("ACPI Scope:     %s", fpcPtr->header.acpi_scope);
	ESIF_TRACE_DEBUG("ACPI Type:      %s", fpcPtr->header.acpi_type);
	ESIF_TRACE_DEBUG("ACPI UID:       %s", fpcPtr->header.acpi_UID);
	ESIF_TRACE_DEBUG("PCI Vendor ID:  %s", fpcPtr->header.pci_vendor_id);
	ESIF_TRACE_DEBUG("PCI Device ID:  %s", fpcPtr->header.pci_device_id);
	ESIF_TRACE_DEBUG("PCI Bus:        %s", fpcPtr->header.pci_bus);
	ESIF_TRACE_DEBUG("PCI Device:     %s", fpcPtr->header.pci_device);
	ESIF_TRACE_DEBUG("PCI Function:   %s", fpcPtr->header.pci_function);

	dspPtr->code_ptr = (EsifString)fpcPtr->header.name;
	dspPtr->bus_enum = (UInt8 *)&fpcPtr->header.bus_enum;
	dspPtr->type     = (EsifString)fpcPtr->header.type;
	dspPtr->ver_major_ptr  = (UInt8 *)&fpcPtr->header.ver_major;
	dspPtr->ver_minor_ptr  = (UInt8 *)&fpcPtr->header.ver_minor;
	dspPtr->acpi_device    = (EsifString)fpcPtr->header.acpi_device;
	dspPtr->acpi_scope     = (EsifString)fpcPtr->header.acpi_scope;
	dspPtr->acpi_type      = (EsifString)fpcPtr->header.acpi_type;
	dspPtr->acpi_uid       = (EsifString)fpcPtr->header.acpi_UID;
	dspPtr->vendor_id      = (EsifString)fpcPtr->header.pci_vendor_id;
	dspPtr->device_id      = (EsifString)fpcPtr->header.pci_device_id;
	dspPtr->pci_bus		= (EsifString)&fpcPtr->header.pci_bus;
	dspPtr->pci_bus_device = (EsifString)&fpcPtr->header.pci_device;
	dspPtr->pci_function   = (EsifString)&fpcPtr->header.pci_function;

	/* Assign Function Pointers */
	dspPtr->get_code = get_code;
	dspPtr->get_ver_minor     = get_ver_minor;
	dspPtr->get_ver_major     = get_ver_major;
	dspPtr->get_temp_tc1      = get_temp_c1;
	dspPtr->get_percent_xform      = get_percent_xform;

	dspPtr->insert_primitive  = insert_primitive;
	dspPtr->insert_algorithm  = insert_algorithm;
	dspPtr->insert_domain     = insert_domain;
	dspPtr->insert_event      = insert_event;
	dspPtr->get_primitive     = get_primitive;
	dspPtr->get_action = get_action;
	dspPtr->get_algorithm     = get_algorithm;
	dspPtr->get_domain = get_domain;
	dspPtr->get_event_by_type = get_event_by_type;
	dspPtr->get_event_by_guid = get_event_by_guid;
	dspPtr->init_fpc_iterator = init_fpc_iterator;
	dspPtr->get_next_fpc_domain = get_next_fpc_domain;


	dspPtr->get_domain_count = get_domain_count;

	rc = esif_fpc_load(fpcPtr, dspPtr);
	if (ESIF_OK == rc) {
		ESIF_TRACE_DEBUG("FPC %s load successfully", path);
	} else {
		ESIF_TRACE_DEBUG("Unable to load FPC %s, rc %s", path, esif_rc_str(rc));
		goto exit;
	}

	/* Lock DSP Manager */
	esif_ccb_write_lock(&g_dm.lock);

	/* Simple Table Lookup For Now. Scan Table And Find First Empty Slot */
	/* Empty slot indicated by AVAILABLE state                           */
	for (i = 0; i < MAX_DSP_MANAGER_ENTRY; i++) {
		if (NULL == g_dm.dme[i].dsp_ptr) {
			break;
		}
	}

	/* If No Available Slots Return */
	if (i >= MAX_DSP_MANAGER_ENTRY) {
		esif_ccb_write_unlock(&g_dm.lock);
		ESIF_TRACE_ERROR("No free dsp manager entry is available for %s\n", file_ptr->filename);
		goto exit;
	}

	/*
	** Take Slot
	*/
	g_dm.dme[i].dsp_ptr  = dspPtr;
	g_dm.dme[i].file_ptr = file_ptr;
	g_dm.dme[i].fpc_ptr  = (fpcIsStatic ? 0 : fpcPtr);
	g_dm.dme_count++;
	dspPtr = NULL;	// Prevent deallocate on exit
	fpcPtr = NULL;	// Prevent deallocate on exit

	esif_ccb_write_unlock(&g_dm.lock);
	rc = ESIF_OK;
	ESIF_TRACE_INFO("Create entry in dsp manager successfully for %s\n", file_ptr->filename);

exit:
	IOStream_Destroy(ioPtr);
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	esif_dsp_destroy(dspPtr);
	if (!fpcIsStatic) {
		esif_ccb_free(fpcPtr);
	}
	return rc;
}


static eEsifError esif_dsp_file_scan()
{
	eEsifError rc = ESIF_OK;
	struct esif_ccb_file *ffdPtr = NULL;
	esif_ccb_file_enum_t findHandle = ESIF_INVALID_FILE_ENUM_HANDLE;
	char path[MAX_PATH]    = {0};
	char pattern[MAX_PATH] = {0};
	StringPtr namesp = ESIF_DSP_NAMESPACE;

	// 1. Load all EDP's in the DSP Configuration Namespace, if any exist
	EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_AUTO, namesp, 0, ESIFAUTOLEN);
	EsifDataPtr key       = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	EsifDataPtr value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	EsifConfigFindContext context = NULL;

	ESIF_TRACE_DEBUG("SCAN CONFIG For DSP Files NameSpace = %s, Pattern %s", namesp, pattern);
	if (nameSpace != NULL && key != NULL && value != NULL &&
		(rc = EsifConfigFindFirst(nameSpace, key, value, &context)) == ESIF_OK) {
		do {
			// Load all keys from the DataVault with an ".edp" extension
			if (key->data_len >= 5 && esif_ccb_stricmp(((StringPtr)(key->buf_ptr)) + key->data_len - 5, ".edp") == 0) {
				ffdPtr = (struct esif_ccb_file *)esif_ccb_malloc(sizeof(*ffdPtr));
				if (ffdPtr == NULL) {
					ESIF_TRACE_ERROR("Fail to allocate esif_ccb file\n");
					rc = ESIF_E_NO_MEMORY;
					break;
				}
				esif_ccb_strcpy(ffdPtr->filename, (StringPtr)key->buf_ptr, sizeof(ffdPtr->filename));
				if (esif_dsp_entry_create(ffdPtr) != ESIF_OK) {
					esif_ccb_free(ffdPtr);
				}
			}
			EsifData_Set(key, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
			EsifData_Set(value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		} while ((rc = EsifConfigFindNext(nameSpace, key, value, &context)) == ESIF_OK);

		EsifConfigFindClose(&context);
		if (rc == ESIF_E_ITERATION_DONE) {
			rc = ESIF_OK;
		}
	}
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);

	// 2. Load all EDP's from the DSP folder, if any exist, except ones already loaded from DataBank
	esif_build_path(path, MAX_PATH, ESIF_PATHTYPE_DSP, NULL, NULL);
	esif_ccb_strcpy(pattern, "*.edp", MAX_PATH);

	ESIF_TRACE_DEBUG("SCAN File System For DSP Files Path = %s, Pattern %s", path, pattern);
	/* Find the first file in the directory that matches are search */
	ffdPtr = (struct esif_ccb_file *)esif_ccb_malloc(sizeof(*ffdPtr));
	if (ffdPtr == NULL) {
		ESIF_TRACE_ERROR("Fail to allocate esif_ccb file\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	findHandle = esif_ccb_file_enum_first(path, pattern, ffdPtr);
	if (ESIF_INVALID_FILE_ENUM_HANDLE == findHandle) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	/* Process Each File */
	do {
		// Don't process the file if it the same name was already loaded from a DataVault
		if (DataBank_KeyExists(namesp, ffdPtr->filename) == ESIF_FALSE) {
			if (esif_dsp_entry_create(ffdPtr) != ESIF_OK) {
				esif_ccb_free(ffdPtr);
			}
			ffdPtr = (struct esif_ccb_file *)esif_ccb_malloc(sizeof(*ffdPtr));
		}
	} while (esif_ccb_file_enum_next(findHandle, pattern, ffdPtr));
	esif_ccb_file_enum_close(findHandle);

exit:
	if (ffdPtr != NULL) {
		esif_ccb_free(ffdPtr);
	}
	return rc;
}


/* Build DSP Table From DSP Resources */
static eEsifError esif_dsp_table_build()
{
	eEsifError rc = ESIF_OK;
	ESIF_TRACE_INFO("Build DSP Table");
	rc = esif_dsp_file_scan();
	return rc;
}


void esif_dsp_table_destroy()
{
	UInt8 i;
	ESIF_TRACE_INFO("Destroy DSP Table");
	for (i = 0; i < g_dm.dme_count; i++) {
		esif_dsp_destroy(g_dm.dme[i].dsp_ptr);
		esif_ccb_free(g_dm.dme[i].file_ptr);
		esif_ccb_free(g_dm.dme[i].fpc_ptr);
		esif_ccb_memset(&g_dm.dme[i], 0, sizeof(g_dm.dme[i]));
	}
	g_dm.dme_count = 0;
}


/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */

#ifdef ESIF_ATTR_OS_WINDOWS
#define	DEFAULT_PROC	"skl_proc"
#else
#define	DEFAULT_PROC	"lin_proc"
#endif

struct dsp_map_s {
	esif_string name;		/* participant name or NULL for Default or end-of-list */
	esif_string dsp_name;	/* DSP name */
};
static struct dsp_map_s dsp_mapping[] = {
		{"IETM",	"ipf_ietm"},
		{"TCPU",	DEFAULT_PROC},
		{"TPCH",	"ipf_pch" },
		{"TAMB",	"ipf_fgen"},
		{"TCHG",	"ipf_fgen"},
		{"TEFN",	"ipf_fgen"},
		{"TFN1",	"ipf_fan" },
		{"TFN2",	"ipf_fan" },
		{"TINL",	"ipf_fgen"},
		{"TMEM",	"ipf_cmem"},
		{"TPWR",	"ipf_pwr" },
		{"TSKN",	"ipf_fgen"},
		{"T_VR",	"ipf_fgen"},
		{"VTS1",	"ipf_virt"},
		{"VTS2",	"ipf_virt"},
		{"WIFI",	"ipf_wifi"},
		{"WGIG",	"ipf_wgig"},
		{"WPKG",	"ipf_wpkg"},
		{"WWAG",	"ipf_wwag"},
		{"WWDG",	"ipf_wwdg"},
		{"WWRF",	"ipf_wwrf"},
		{"DGFXMCP",	"ipf_mcp"},
		{"DGFXCORE","ipf_dgcr"},
		{"DGFXMEM",	"ipf_dgmm"},
		{"IDG2",	"ipf_idg2"},
		/* Deprecated */
		{"DPTFZ",	"ipf_ietm"},
		{"DPLY",	"ipf_disp"},
		{"FGEN",	"ipf_fgen"},
		{"GEN1",	"ipf_fgen"},
		{"GEN2",	"ipf_fgen"},
		{NULL,		"ipf_fgen"},
};


EsifDspPtr esif_uf_dm_select_dsp_by_code(esif_string code)
{
	UInt8 i = 0;
	EsifDspPtr foundPtr = NULL;

	for (i = 0; i < g_dm.dme_count; i++) {
		if (!strcmp(code, g_dm.dme[i].dsp_ptr->code_ptr)) {
			foundPtr = g_dm.dme[i].dsp_ptr;
			break;
		}
	}

	return foundPtr;
}

EsifString GetDSPFromList(
	EsifString name
	)
{
	EsifString selection = NULL;
	UInt32 item = 0;

	for (item = 0; dsp_mapping[item].dsp_name != NULL; item++) {
		if (dsp_mapping[item].name == NULL || esif_ccb_strcmp(dsp_mapping[item].name, name) == 0) {
			selection = dsp_mapping[item].dsp_name;
			break;
		}
	}
	return selection;
}

EsifString EsifDspMgr_SelectDsp(
	EsifDspQuery query
	)
{
	int i = 0;							// Loop index for DSP table.
	int weight = 0;						// Total weight of DSP query
	int primaryWeight = 0;				// Weight of items sufficient for selection.
	int secondaryWeight = 0;			// Weight of items allowed to sway results, but cannot stand alone.
	int heaviest = -1;					// Heaviest Row Found.
	EsifString dspNamePtr = NULL;		// DSP Name/Code To Use.
	int best = 0;						// Best Weight Possible.
	char *defaultDsp = GetDSPFromList(query.participantName); // Default DSP based on Participant Name

	/*
	** Query table for weighted match highest score wins. It is
	** unlikely that the table would change but we lock it just
	** to be sure it can't.
	*/
	
	// LOCK
	esif_ccb_read_lock(&g_dm.lock);

	for (i = 0; i < g_dm.dme_count; i++) {
		EsifDspPtr dspPtr = g_dm.dme[i].dsp_ptr;
		char busEnum[ENUM_TO_STRING_LEN] = { 0 };

		if (dspPtr == NULL) {
			continue;
		}

		esif_ccb_sprintf(ENUM_TO_STRING_LEN, busEnum, "%d", *dspPtr->bus_enum);
		
		// Primary weight indicates items that can result in a match
		primaryWeight =
			compare_and_weight_minterm(dspPtr->device_id, query.deviceId, PCI_DEVICE_ID_WEIGHT) +
			compare_and_weight_minterm(dspPtr->acpi_device, query.hid, ACPI_HID_WEIGHT) +
			compare_and_weight_minterm(dspPtr->acpi_type, query.ptype, ACPI_PTYPE_WEIGHT) +
			compare_and_weight_minterm(dspPtr->code_ptr, defaultDsp, PARTICIPANT_NAME_WEIGHT);

		// Secondary items can contribute only if at least one primary match is found
		secondaryWeight = 
			compare_and_weight_minterm(dspPtr->acpi_uid, query.uid, ACPI_UID_WEIGHT) +
			compare_and_weight_minterm(dspPtr->vendor_id, query.vendorId, PCI_VENDOR_ID_WEIGHT) +
			compare_and_weight_minterm(busEnum, query.enumerator, ENUMERATOR_TYPE_WEIGHT);

		// Items in secondary can weigh in on the results but cannot be the sole factor
		if (primaryWeight < MIN_VIABLE_DSP_WEIGHT) {
			weight = primaryWeight;
		}
		else {
			weight = primaryWeight + secondaryWeight;
		}

		ESIF_TRACE_DEBUG("DSP: %s Weight: %d\n", (esif_string) dspPtr->code_ptr, weight);

		//Keep track of row with most weight.  
		if (weight > heaviest) {
			heaviest = weight;
			dspNamePtr = dspPtr->code_ptr;
		}
	}

	// UNLOCK
	esif_ccb_read_unlock(&g_dm.lock);

	if (dspNamePtr != NULL) {
		ESIF_TRACE_DEBUG("Selected DSP: %s Score: %d of %d\n", dspNamePtr, heaviest, best);
	}
	else {
		ESIF_TRACE_ERROR("No DSP selected for %s. \n", query.participantName);
	}

	return dspNamePtr;
}

/* TRUE if item is an exact match or item is a member of "item1|item2|...|itemN" (case-insensitive) */
static Bool minterm_matches(
	esif_string list,
	esif_string item
	)
{
	size_t len = esif_ccb_strlen(item, MAX_MINTERM_LENGTH);

	while (list) {
		if ((esif_ccb_strnicmp(list, item, len) == 0) && (list[len] == 0 || list[len] == '|')) {
			return ESIF_TRUE;
		}
		if ((list = esif_ccb_strchr(list, '|')) != NULL)
			list++;
	}
	return ESIF_FALSE;
}

/* Handle each weighted Minterm */
static int compare_and_weight_minterm(
	esif_string dsp,
	esif_string qry,
enum dspSelectorWeight weight
	)
{
	int score = 0;
	
	if (0 == *dsp) {	/* DSP don't care trumps all */
		score = 0;
	}
	else if ('*' == *dsp) {	/* Wildcard DSP matches all but with lower score than exact match */
		score = ((int)weight) / 4;
	}
	else if (0 == *qry || '*' == *qry) {	/* blank == blank don't care. */
		score = 0;
	}
	else if (minterm_matches(dsp, qry)) { /* data == exact match or qry in "data|data|...|data" */
		score = (int)weight;
	}
	else {
		score = -1;	/* no match. */
	}
	ESIF_TRACE_DEBUG("DSP VALUE: %s VERSUS PARTICIPANT VALUE: %s  = SCORE: %d \n", dsp, qry, score);
	return score;
}

eEsifError EsifDspMgrInit(void)
{
	eEsifError rc = ESIF_OK;
	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_dm.lock);

	rc = esif_dsp_table_build();

	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifDspMgrExit(void)
{
	ESIF_TRACE_ENTRY_INFO();

	esif_dsp_table_destroy();
	esif_ccb_lock_uninit(&g_dm.lock);

	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
