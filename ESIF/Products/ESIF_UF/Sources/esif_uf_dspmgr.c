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

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// Limits
#define MAX_EDP_SIZE    0x7fffffff
#define MAX_FPC_SIZE    0x7fffffff

#define MIN_VIABLE_DSP_WEIGHT 1

enum dspSelectorWeight {
	ACPI_HID_WEIGHT = 10,
	ACPI_PTYPE_WEIGHT = 18,
	PCI_DEVICE_ID_WEIGHT = 20,
	PCI_VENDOR_ID_WEIGHT = 2,
	PARTICIPANT_TYPE_WEIGHT = 4,
	PARTICIPANT_NAME_WEIGHT = 8
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
struct esif_uf_dm g_dm = {0};


/* Allocate DSP Upper Insance */
static struct esif_up_dsp *esif_dsp_create(void)
{
	struct esif_up_dsp *dsp_ptr = NULL;
	dsp_ptr = (struct esif_up_dsp *)esif_ccb_malloc(sizeof(*dsp_ptr));
	return dsp_ptr;
}


/* Free DSP Upper Instance */
static void esif_dsp_destroy(struct esif_up_dsp *dsp_ptr)
{
	if (NULL == dsp_ptr) {
		return;
	}
	esif_ht_destroy(dsp_ptr->ht_ptr, NULL);
	esif_link_list_destroy(dsp_ptr->algo_ptr);
	esif_link_list_destroy(dsp_ptr->domain_ptr);
	esif_link_list_destroy(dsp_ptr->cap_ptr);
	esif_link_list_destroy(dsp_ptr->evt_ptr);
	esif_ccb_free(dsp_ptr);
}


/* DSP Interface */
static esif_string get_code(struct esif_up_dsp *dsp)
{
	if (dsp->code_ptr) {
		return dsp->code_ptr;
	} else {
		return "NA";
	}
}


static u8 get_ver_major(struct esif_up_dsp *dsp)
{
	if (dsp->ver_major_ptr) {
		return *dsp->ver_major_ptr;
	} else {
		return 0;
	}
}


static u8 get_ver_minor(struct esif_up_dsp *dsp)
{
	if (dsp->ver_minor_ptr) {
		return *dsp->ver_minor_ptr;
	} else {
		return 0;
	}
}


static u32 get_temp_c1(
	struct esif_up_dsp *dsp,
	const enum esif_action_type action
	)
{
	/* TODO: Assume tempC1 Is Slope */
	struct esif_fpc_algorithm *algo_ptr = dsp->get_algorithm(dsp, action);
	if (algo_ptr) {
		return algo_ptr->tempC1;
	} else {
		return 0xffffffff;
	}
}


static u32 get_percent_xform(
	struct esif_up_dsp *dsp,
	const enum esif_action_type action
	)
{
	struct esif_fpc_algorithm *algo_ptr = dsp->get_algorithm(dsp, action);
	if (algo_ptr) {
		return algo_ptr->percent_xform;
	} else {
		return 0xffffffff;
	}
}


/* Insert Primitive */
static eEsifError insert_primitive(
	struct esif_up_dsp *dsp_ptr,
	EsifFpcPrimitive *primitive_ptr
	)
{
	if (NULL == primitive_ptr) {
		return ESIF_E_NULL_PRIMITIVE;
	} else {
		return esif_ht_add_item(dsp_ptr->ht_ptr, /* Hash Table  */
			(UInt8 *)&primitive_ptr->tuple,	/* Key */
			sizeof(primitive_ptr->tuple),/* Size Of Key */
			primitive_ptr); /* Item        */
	}
}

/* Get Primitive */
static EsifFpcPrimitivePtr get_primitive(
	struct esif_up_dsp *dsp_ptr,
	const struct esif_primitive_tuple *tuple_ptr
	)
{
	EsifFpcPrimitivePtr primitive_ptr = NULL;

	primitive_ptr = (EsifFpcPrimitivePtr)
		esif_ht_get_item(dsp_ptr->ht_ptr,
			(u8 *)tuple_ptr,
			sizeof(*tuple_ptr));

	return primitive_ptr;
}


/* Get i-th Action */
static EsifFpcActionPtr get_action(
	EsifDspPtr dsp_ptr,
	EsifFpcPrimitivePtr primitive_ptr,
	UInt8 index
	)
{
	EsifFpcActionPtr action_ptr    = NULL;
	int i;

	if (NULL == dsp_ptr || NULL == primitive_ptr) {
		return NULL;
	}

	/* First Action */
	action_ptr = (EsifFpcActionPtr)(primitive_ptr + 1);

	/* Locate i-th Action We Are Looking For */
	for (i = 0; i < index; i++) {
		action_ptr = (EsifFpcActionPtr)((UInt8 *)action_ptr + action_ptr->size);
	}

	return action_ptr;
}


/* Insert Algorithm Into Linked List */
static enum esif_rc insert_algorithm(
	struct esif_up_dsp *dsp_ptr,
	struct esif_fpc_algorithm *algo_ptr
	)
{
	if (NULL == algo_ptr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	return esif_link_list_add_at_back(dsp_ptr->algo_ptr, (void *)algo_ptr);
}


/* Insert Event Into Linked List */
static enum esif_rc insert_event(
	struct esif_up_dsp *dsp_ptr,
	struct esif_fpc_event *evt_ptr
	)
{
	if (NULL == evt_ptr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	return esif_link_list_add_at_back(dsp_ptr->evt_ptr, (void *)evt_ptr);
}


/* Get Algorithm From Linked List */
static struct esif_fpc_algorithm *get_algorithm(
	struct esif_up_dsp *dsp_ptr,
	const enum esif_action_type action_type
	)
{
	struct esif_link_list *list_ptr = dsp_ptr->algo_ptr;
	struct esif_link_list_node *curr_ptr     = list_ptr->head_ptr;
	struct esif_fpc_algorithm *curr_algo_ptr = NULL;

	while (curr_ptr) {
		curr_algo_ptr = (struct esif_fpc_algorithm *)curr_ptr->data_ptr;
		if (curr_algo_ptr != NULL) {
			if (curr_algo_ptr->action_type == action_type) {
				return curr_algo_ptr;
			}
		}
		curr_ptr = curr_ptr->next_ptr;
	}
	return NULL;
}


/* Get Event From Linked List By Type */
static struct esif_fpc_event *get_event_by_type(
	struct esif_up_dsp *dsp_ptr,
	const enum esif_event_type event_type
	)
{
	struct esif_link_list *list_ptr      = dsp_ptr->evt_ptr;
	struct esif_link_list_node *curr_ptr = list_ptr->head_ptr;
	struct esif_fpc_event *curr_evt_ptr  = NULL;

	while (curr_ptr) {
		curr_evt_ptr = (struct esif_fpc_event *)curr_ptr->data_ptr;
		if (curr_evt_ptr != NULL) {
			if (curr_evt_ptr->esif_event == event_type) {
				return curr_evt_ptr;
			}
		}
		curr_ptr = curr_ptr->next_ptr;
	}
	return NULL;
}


/* Get Event From Linked List By GUID */
static struct esif_fpc_event *get_event_by_guid(
	struct esif_up_dsp *dsp_ptr,
	const esif_guid_t guid
	)
{
	struct esif_link_list *list_ptr      = dsp_ptr->evt_ptr;
	struct esif_link_list_node *curr_ptr = list_ptr->head_ptr;
	struct esif_fpc_event *curr_evt_ptr  = NULL;

	while (curr_ptr) {
		curr_evt_ptr = (struct esif_fpc_event *)curr_ptr->data_ptr;
		if (curr_evt_ptr != NULL) {
			if (memcmp(curr_evt_ptr->event_guid, guid, ESIF_GUID_LEN) == 0) {
				return curr_evt_ptr;
			}
		}
		curr_ptr = curr_ptr->next_ptr;
	}
	return NULL;
}


/* Insert Domain Into Linked List */
static enum esif_rc insert_domain(
	struct esif_up_dsp *dsp_ptr,
	struct esif_fpc_domain *domain_ptr
	)
{
	if (NULL == domain_ptr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	return esif_link_list_add_at_back(dsp_ptr->domain_ptr, (void *)domain_ptr);
}


static UInt32 get_domain_count(struct esif_up_dsp *dsp_ptr)
{
	return *dsp_ptr->domain_count;
}


/* Get i-th Domain */
static struct esif_fpc_domain *get_domain(
	struct esif_up_dsp *dsp_ptr,
	const u32 index
	)
{
	struct esif_link_list *list_ptr = dsp_ptr->domain_ptr;
	struct esif_link_list_node *curr_ptr    = list_ptr->head_ptr;
	struct esif_fpc_domain *curr_domain_ptr = NULL;
	u32 i = 0;

	while ((++i < index) && (curr_ptr != NULL)) {
		curr_ptr = curr_ptr->next_ptr;
	}
	if (curr_ptr != NULL) {
		curr_domain_ptr = (struct esif_fpc_domain *)curr_ptr->data_ptr;
	}
	return curr_domain_ptr;
}


enum esif_rc esif_fpc_load(
	struct esif_fpc *fpc_ptr,
	struct esif_up_dsp *dsp_ptr
	)
{
	struct esif_fpc_domain *domain_ptr;
	struct esif_fpc_primitive *primitive_ptr;
	struct esif_fpc_algorithm *algorithm_ptr;
	struct esif_fpc_event *event_ptr;
	enum esif_rc rc = ESIF_OK;
	unsigned long offset = 0;
	u8 *base_ptr    = (u8 *)fpc_ptr;
	u32 num_prim    = 0, i, j;

	if ((fpc_ptr == NULL) || (dsp_ptr == NULL))
	{
		ESIF_TRACE_ERROR("The fpc pointer or dsp pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (fpc_ptr->number_of_domains < 1) {
		ESIF_TRACE_WARN("No domain error, number_of_domain = %d (less than 1)\n",
			fpc_ptr->number_of_domains);
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	dsp_ptr->domain_count = &fpc_ptr->number_of_domains;

	/* Allocate Hash and List. Hash Size 31 Is Hard-Coded And Chosen By DSP Compiler */
	dsp_ptr->ht_ptr     = esif_ht_create(ESIF_DSP_HASHTABLE_SIZE);
	dsp_ptr->algo_ptr   = esif_link_list_create();
	dsp_ptr->domain_ptr = esif_link_list_create();
	dsp_ptr->cap_ptr    = esif_link_list_create();
	dsp_ptr->evt_ptr    = esif_link_list_create();

	if (!dsp_ptr->ht_ptr || !dsp_ptr->algo_ptr || !dsp_ptr->domain_ptr || !dsp_ptr->cap_ptr || !dsp_ptr->evt_ptr) {
		ESIF_TRACE_ERROR("Fail to allocate linked list or hash table\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ESIF_TRACE_DEBUG("<fpc @ %p> FPC name '%s' ver %x,%x desc '%s' size %u num_domains %d num_algorithms %d num_events %d",
					 fpc_ptr,
					 fpc_ptr->header.name,
					 fpc_ptr->header.ver_major,
					 fpc_ptr->header.ver_minor,
					 fpc_ptr->header.description,
					 fpc_ptr->size,
					 fpc_ptr->number_of_domains,
					 fpc_ptr->number_of_algorithms,
					 fpc_ptr->number_of_events);

	/* First Domain, Ok To Have Zero Primitive Of A Domain */
	domain_ptr = (struct esif_fpc_domain *)(fpc_ptr + 1);
	for (i = 0; i < fpc_ptr->number_of_domains; i++) {
		offset = (unsigned long)((u8 *)domain_ptr - base_ptr);
		ESIF_TRACE_DEBUG("<%04lu> Domain[%d] name %s size %d num_of_primitives %d  "
						 "num_of_capabilites %u (0x%x)",
						 offset, i, domain_ptr->descriptor.name, domain_ptr->size,
						 domain_ptr->number_of_primitives,
						 domain_ptr->capability_for_domain.number_of_capability_flags,
						 domain_ptr->capability_for_domain.capability_flags);

		/* Insert Domain Into Linked List */
		rc = dsp_ptr->insert_domain(dsp_ptr, domain_ptr);
		if (ESIF_OK != rc) {
			ESIF_TRACE_ERROR("Fail to insert domain #%d\n", i);
			goto exit;
		}

		/* Capability */
		for (j = 0; j < domain_ptr->capability_for_domain.number_of_capability_flags; j++) {
			offset = (unsigned long)(((u8 *)&domain_ptr->capability_for_domain) - base_ptr);
			ESIF_TRACE_DEBUG("<%04lu> Capability[%d] 0x%x", offset, j,
							 domain_ptr->capability_for_domain.capability_mask[j]);
		}

		/* First Primtive */
		primitive_ptr = (struct esif_fpc_primitive *)(domain_ptr + 1);
		for (j = 0; j < domain_ptr->number_of_primitives; j++, num_prim++) {
			offset = (unsigned long)(((u8 *)primitive_ptr) - base_ptr);
			ESIF_TRACE_DEBUG("<%04lu> Primitive[%03d]: size %3d tuple_id <%03u %03u %03u> "
							 "operation %u(%s) req_type %u(%s) res_type %u(%s) num_actions %u",
							 offset, j,
							 primitive_ptr->size,
							 primitive_ptr->tuple.id,
							 primitive_ptr->tuple.domain,
							 primitive_ptr->tuple.instance,
							 primitive_ptr->operation,
							 esif_primitive_opcode_str(primitive_ptr->operation),
							 primitive_ptr->request_type,
							 esif_data_type_str(primitive_ptr->request_type),
							 primitive_ptr->result_type,
							 esif_data_type_str(primitive_ptr->result_type),
							 primitive_ptr->num_actions);
			/* Insert Primitive Into Hash */
			rc = dsp_ptr->insert_primitive(dsp_ptr, primitive_ptr);
			if (ESIF_OK != rc) {
				ESIF_TRACE_ERROR("Fail to insert primitive (id = %d)\n", primitive_ptr->tuple.id);
				goto exit;
			}
			/* Next Primitive */
			primitive_ptr = (struct esif_fpc_primitive *)((u8 *)primitive_ptr +
														  primitive_ptr->size);
		}
		/* Next Domain */
		domain_ptr = (struct esif_fpc_domain *)((u8 *)domain_ptr + domain_ptr->size);
	}

	/* First Algorithm (Laid After The Last Domain) */
	algorithm_ptr = (struct esif_fpc_algorithm *)domain_ptr;
	for (i = 0; i < fpc_ptr->number_of_algorithms; i++) {
		offset = (unsigned long)((u8 *)algorithm_ptr - base_ptr);
		ESIF_TRACE_DEBUG("<%04lu> Algorithm[%03d]:  action_type %u(%s) temp_xform %u "
						 "tempC1 %u percent_xform %u size %u",
						 offset, i,
						 algorithm_ptr->action_type,
						 esif_action_type_str(algorithm_ptr->action_type),
						 algorithm_ptr->temp_xform,
						 algorithm_ptr->tempC1,
						 algorithm_ptr->percent_xform,
						 algorithm_ptr->size);

		/* Insert Algorithm Into Linked List */
		rc = dsp_ptr->insert_algorithm(dsp_ptr, algorithm_ptr);
		if (ESIF_OK != rc) {
			ESIF_TRACE_ERROR("Fail to insert algorithm - %s\n", esif_action_type_str(algorithm_ptr->action_type));
			goto exit;
		}

		/* Next Algorithm */
		algorithm_ptr = (struct esif_fpc_algorithm *)(algorithm_ptr + 1);
	}

	/* First Event (Laid After The Last Algorithm) */
	event_ptr = (struct esif_fpc_event *)algorithm_ptr;
	for (i = 0; i < fpc_ptr->number_of_events; i++) {
		offset = (unsigned long)((u8 *)event_ptr - base_ptr);
		ESIF_TRACE_DEBUG("<%04lu> Event [%03d] type %s(%d)\n",
						 offset, i, esif_event_type_str(event_ptr->esif_event), event_ptr->esif_event);

		/* Insert Algorithm Into Linked List */
		rc = dsp_ptr->insert_event(dsp_ptr, event_ptr);
		if (ESIF_OK != rc) {
			ESIF_TRACE_ERROR("Fail to insert event - %s\n", esif_event_type_str(event_ptr->esif_event));
			goto exit;
		}

		/* Next Event */
		event_ptr = (struct esif_fpc_event *)(event_ptr + 1);
	}

exit:
	if (fpc_ptr != NULL) {
		ESIF_TRACE_DEBUG("%u domains, %u primitives and %u algorithms %u events inserted! status %s",
						 fpc_ptr->number_of_domains, num_prim, fpc_ptr->number_of_algorithms, fpc_ptr->number_of_events,
						 esif_rc_str(rc));
	}
	return rc;
}


/* Add DSP Entry */
static eEsifError esif_dsp_entry_create(struct esif_ccb_file *file_ptr)
{
	struct esif_up_dsp *dsp_ptr = NULL;
	struct esif_fpc *fpc_ptr    = NULL;
	u32 fpc_static = ESIF_FALSE;
	eEsifError rc  = ESIF_E_UNSPECIFIED;
	UInt8 i = 0;
	char path[MAX_PATH]={0};
	UInt32 fpc_size = 0, edp_size = 0;
	size_t fpc_read = 0;
	struct edp_dir edp_dir;
	EsifDataPtr nameSpace = 0;
	EsifDataPtr key = 0;
	EsifDataPtr value     = 0;
	IOStreamPtr io_ptr    = IOStream_Create();

	if ((NULL == file_ptr) || (NULL == io_ptr)) {
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

	dsp_ptr = esif_dsp_create();
	if (NULL == dsp_ptr) {
		ESIF_TRACE_ERROR("Fail to allocate dsp entry\n");
		goto exit;
	}

	// Look for EDP file either on disk or in a DataVault (static or file), depending on priority setting
	esif_build_path(path, sizeof(path), ESIF_PATHTYPE_DSP, file_ptr->filename, NULL);
	if ((ESIF_EDP_DV_PRIORITY == 1 || !esif_ccb_file_exists(path)) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		esif_ccb_strcpy(path, file_ptr->filename, MAX_PATH);
		IOStream_SetMemory(io_ptr, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(io_ptr, path, "rb");
	}
	ESIF_TRACE_DEBUG("Fullpath: %s", path);

	if (IOStream_Open(io_ptr) != 0) {
		ESIF_TRACE_ERROR("File not found (%s)", path);
		goto exit;
	}

	/* Read FPC From EDP File */
	if (esif_ccb_strstr(&path[0], ".edp")) {
		/* EDP - Only Read The FPC Part */
		edp_size = (UInt32)IOStream_GetSize(io_ptr);
		fpc_read = IOStream_Read(io_ptr, &edp_dir, sizeof(edp_dir));
		fpc_size = edp_size - edp_dir.fpc_offset;
		if (edp_size > MAX_EDP_SIZE || fpc_size > MAX_FPC_SIZE) {
			ESIF_TRACE_ERROR("The edp or fpc file size is larger than maximum\n");
			goto exit;
		}
		IOStream_Seek(io_ptr, edp_dir.fpc_offset, SEEK_SET);
		ESIF_TRACE_DEBUG("File found (%s) size %u, FPC size %u from offset %u", path, edp_size, fpc_size, edp_dir.fpc_offset);
	} else {
		ESIF_TRACE_DEBUG("File %s does not have .fpc and .edp format!", path);
	}

	// use static DataVault buffer (if available), otherwise allocate space for our FPC file contents (which will not be freed)
	if (IOStream_GetType(io_ptr) == StreamMemory && value->buf_len == 0) {
		fpc_ptr = (struct esif_fpc *)IOStream_GetMemoryBuffer(io_ptr); 
		if (NULL == fpc_ptr) {
			ESIF_TRACE_ERROR("NULL buffer");
			goto exit;
		}
		fpc_ptr  = (struct esif_fpc *) (((BytePtr) fpc_ptr) + IOStream_GetOffset(io_ptr));
		fpc_read = fpc_size;
		ESIF_TRACE_DEBUG("Static vault size %u buf_ptr=0x%p\n", (int)fpc_read, fpc_ptr);
		fpc_static = ESIF_TRUE;
	} else {
		fpc_ptr = (struct esif_fpc *)esif_ccb_malloc(fpc_size);
		if (NULL == fpc_ptr) {
			ESIF_TRACE_ERROR("malloc failed to allocate %u bytes\n", fpc_size);
			goto exit;
		}
		ESIF_TRACE_DEBUG("File malloc size %u", fpc_size);

		// read file contents
		fpc_read = IOStream_Read(io_ptr, fpc_ptr, fpc_size);
		if (fpc_read < fpc_size) {
			ESIF_TRACE_ERROR("Read short received %u of %u bytes\n", (int)fpc_read, fpc_size);
			goto exit;
		}

		ESIF_TRACE_DEBUG("File read size %u", (int)fpc_read);
	}
	ESIF_TRACE_DEBUG("\nDecode Length:  %u", fpc_ptr->size);
	ESIF_TRACE_DEBUG("Code:           %s", fpc_ptr->header.code);
	ESIF_TRACE_DEBUG("Ver Major:      %u", fpc_ptr->header.ver_major);
	ESIF_TRACE_DEBUG("Ver Minor:      %u", fpc_ptr->header.ver_minor);
	ESIF_TRACE_DEBUG("Name:           %s", fpc_ptr->header.name);
	ESIF_TRACE_DEBUG("Description:    %s", fpc_ptr->header.description);
	ESIF_TRACE_DEBUG("Type:           %s", fpc_ptr->header.type);
	ESIF_TRACE_DEBUG("Bus Enumerator: %u", fpc_ptr->header.bus_enum);
	ESIF_TRACE_DEBUG("ACPI Device:    %s", fpc_ptr->header.acpi_device);
	ESIF_TRACE_DEBUG("ACPI Scope:     %s", fpc_ptr->header.acpi_scope);
	ESIF_TRACE_DEBUG("ACPI Type:      %s", fpc_ptr->header.acpi_type);
	ESIF_TRACE_DEBUG("PCI Vendor ID:  %s", fpc_ptr->header.pci_vendor_id);
	ESIF_TRACE_DEBUG("PCI Device ID:  %s", fpc_ptr->header.pci_device_id);
	ESIF_TRACE_DEBUG("PCI Bus:        %s", fpc_ptr->header.pci_bus);
	ESIF_TRACE_DEBUG("PCI Device:     %s", fpc_ptr->header.pci_device);
	ESIF_TRACE_DEBUG("PCI Function:   %s", fpc_ptr->header.pci_function);

	dsp_ptr->code_ptr = (EsifString)fpc_ptr->header.name;
	dsp_ptr->bus_enum = (UInt8 *)&fpc_ptr->header.bus_enum;
	dsp_ptr->type     = (EsifString)fpc_ptr->header.type;
	dsp_ptr->ver_major_ptr  = (UInt8 *)&fpc_ptr->header.ver_major;
	dsp_ptr->ver_minor_ptr  = (UInt8 *)&fpc_ptr->header.ver_minor;
	dsp_ptr->acpi_device    = (EsifString)fpc_ptr->header.acpi_device;
	dsp_ptr->acpi_scope     = (EsifString)fpc_ptr->header.acpi_scope;
	dsp_ptr->acpi_type      = (EsifString)fpc_ptr->header.acpi_type;
	dsp_ptr->vendor_id      = (EsifString)fpc_ptr->header.pci_vendor_id;
	dsp_ptr->device_id      = (EsifString)fpc_ptr->header.pci_device_id;
	dsp_ptr->pci_bus		= (EsifString)&fpc_ptr->header.pci_bus;
	dsp_ptr->pci_bus_device = (EsifString)&fpc_ptr->header.pci_device;
	dsp_ptr->pci_function   = (EsifString)&fpc_ptr->header.pci_function;

	/* Assign Function Pointers */
	dsp_ptr->get_code = get_code;
	dsp_ptr->get_ver_minor     = get_ver_minor;
	dsp_ptr->get_ver_major     = get_ver_major;
	dsp_ptr->get_temp_tc1      = get_temp_c1;
	dsp_ptr->get_percent_xform      = get_percent_xform;

	dsp_ptr->insert_primitive  = insert_primitive;
	dsp_ptr->insert_algorithm  = insert_algorithm;
	dsp_ptr->insert_domain     = insert_domain;
	dsp_ptr->insert_event      = insert_event;
	dsp_ptr->get_primitive     = get_primitive;
	dsp_ptr->get_action = get_action;
	dsp_ptr->get_algorithm     = get_algorithm;
	dsp_ptr->get_domain = get_domain;
	dsp_ptr->get_event_by_type = get_event_by_type;
	dsp_ptr->get_event_by_guid = get_event_by_guid;


	dsp_ptr->get_domain_count = get_domain_count;

	rc = esif_fpc_load(fpc_ptr, dsp_ptr);
	if (ESIF_OK == rc) {
		ESIF_TRACE_DEBUG("FPC %s load successfully", path);
	} else {
		ESIF_TRACE_DEBUG("Unable to load FPC %s, rc %s", path, esif_rc_str(rc));
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
	g_dm.dme[i].dsp_ptr  = dsp_ptr;
	g_dm.dme[i].file_ptr = file_ptr;
	g_dm.dme[i].fpc_ptr  = (fpc_static ? 0 : fpc_ptr);
	g_dm.dme_count++;
	dsp_ptr = NULL;	// Deallocate on exit
	fpc_ptr = NULL;	// Deallocate on exit

	esif_ccb_write_unlock(&g_dm.lock);
	rc = ESIF_OK;
	ESIF_TRACE_INFO("Create entry in dsp manager sucessfully for %s\n", file_ptr->filename);

exit:
	IOStream_Destroy(io_ptr);
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	esif_ccb_free(dsp_ptr);
	if (!fpc_static) {
		esif_ccb_free(fpc_ptr);
	}
	return rc;
}


static eEsifError esif_dsp_file_scan()
{
	eEsifError rc = ESIF_OK;
	struct esif_ccb_file *ffd_ptr = NULL;
	esif_ccb_file_enum_t find_handle = INVALID_HANDLE_VALUE;
	char path[MAX_PATH]    = {0};
	char pattern[MAX_PATH] = {0};
	int files = 0;
	StringPtr namesp = ESIF_DSP_NAMESPACE;
	DataVaultPtr DB  = DataBank_GetNameSpace(g_DataBankMgr, namesp);

	// 1. Load all EDP's in the DSP Configuration Namespace, if any exist
	if (DB) {
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
					ffd_ptr = (struct esif_ccb_file *)esif_ccb_malloc(sizeof(*ffd_ptr));
					esif_ccb_strcpy(ffd_ptr->filename, (StringPtr)key->buf_ptr, sizeof(ffd_ptr->filename));
					esif_dsp_entry_create(ffd_ptr);
					files++;
				}
				EsifData_Set(key, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
				EsifData_Set(value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
			} while ((rc = EsifConfigFindNext(nameSpace, key, value, &context)) == ESIF_OK);
			if (rc == ESIF_E_ITERATION_DONE) {
				rc = ESIF_OK;
			}
		}
		EsifData_Destroy(nameSpace);
		EsifData_Destroy(key);
		EsifData_Destroy(value);
	}

	// 2. Load all EDP's from the DSP folder, if any exist, except ones already loaded from DataBank
	esif_build_path(path, MAX_PATH, ESIF_PATHTYPE_DSP, NULL, NULL);
	esif_ccb_strcpy(pattern, "*.edp", MAX_PATH);

	ESIF_TRACE_DEBUG("SCAN File System For DSP Files Path = %s, Pattern %s", path, pattern);
	/* Find the first file in the directory that matches are search */
	ffd_ptr = (struct esif_ccb_file *)esif_ccb_malloc(sizeof(*ffd_ptr));
	if (ffd_ptr == NULL) {
		ESIF_TRACE_ERROR("Fail to allocate esif_ccb file\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	find_handle = esif_ccb_file_enum_first(path, pattern, ffd_ptr);
	if (INVALID_HANDLE_VALUE == find_handle) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	/* Process Each File */
	do {
		// Don't process the file if it the same name was already loaded from a DataVault
		if (DB == NULL || DataCache_GetValue(DB->cache, ffd_ptr->filename) == NULL) {
			esif_dsp_entry_create(ffd_ptr);
			ffd_ptr = (struct esif_ccb_file *)esif_ccb_malloc(sizeof(*ffd_ptr));
			files++;
		}
	} while (esif_ccb_file_enum_next(find_handle, pattern, ffd_ptr));
	esif_ccb_file_enum_close(find_handle);

exit:
	if (ffd_ptr != NULL) {
		esif_ccb_free(ffd_ptr);
	}
	return rc;
}


/* Build DSP Table From DSP Resources */
static eEsifError esif_dsp_table_build()
{
	eEsifError rc = ESIF_OK;
	ESIF_TRACE_INFO("Build DSP Table");
	esif_dsp_file_scan();
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

struct dsp_map_s {
	esif_string name;		/* participant name or NULL for Default or end-of-list */
	esif_string dsp_name;	/* DSP name or NULL for end-of-list */
};
static struct dsp_map_s dsp_mapping[] = {
		{"TFN1",	"dpf_fan" },
		{"TFN2",	"dpf_fan" },
		{"TMEM",	"dpf_cmem"},
		{"TAMB",	"dpf_fgen"},
		{"TEFN",	"dpf_fgen"},
		{"TSKN",	"dpf_fgen"},
		{"T_VR",	"dpf_fgen"},
		{"FGEN",	"dpf_fgen"},
		{"DPLY",	"dpf_disp"},
		{"TPWR",	"dpf_pwr" },
		{"WIFI",	"dpf_wifi"},
		{"WGIG",	"dpf_wgig"},
		{"WWAN",	"dpf_wwan"},
		{"TINL",	"dpf_fgen"},
		{"TCPU",	"shb_proc"},
		{"B0DB",	"dpf_proc"},
		{"B0D4",	"dpf_proc"},
		{"TPCH",	"shb_pch" },
		{"IETM",	"dpf_dptf"},
		{"DPTFZ",	"dpf_dptf"},
		{"GEN1",	"dpf_fgen"},
		{"TCHG",	"dpf_fgen"},
		{"GEN2",	"dpf_fgen"},
		{"WPKG",	"dpf_wpkg"},
		{"VTS1",	"dpf_virt"},
		{"VTS2",	"dpf_virt"},
		{NULL,		"dpf_fgen"},
};

esif_string esif_uf_dm_select_dsp(
	void *piPtr
	)
{
	esif_string selection = NULL;
	EsifParticipantIfacePtr pi_ptr = (EsifParticipantIfacePtr)piPtr;
	esif_string name = pi_ptr->name;
	u32 item = 0;

	for (item = 0; dsp_mapping[item].dsp_name != NULL; item++) {
		if (dsp_mapping[item].name == NULL || esif_ccb_strcmp(dsp_mapping[item].name, name) == 0) {
			selection = dsp_mapping[item].dsp_name;
			break;
		}
	}
	return selection;
}


struct esif_up_dsp *esif_uf_dm_select_dsp_by_code(esif_string code)
{
	UInt8 i = 0;
	struct esif_up_dsp *found_ptr = NULL;

	for (i = 0; i < g_dm.dme_count; i++) {
		if (!strcmp(code, g_dm.dme[i].dsp_ptr->code_ptr)) {
			found_ptr = g_dm.dme[i].dsp_ptr;
			break;
		}
	}

	return found_ptr;
}

EsifString GetDSPFromList(
	EsifString name
	)
{
	EsifString selection = NULL;
	u32 item = 0;

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

	/*
	** Query table for weighted match highest score wins. It is
	** unlikely that the table would change but we lock it just
	** to be sure it can't.
	*/
	
	// LOCK
	esif_ccb_read_lock(&g_dm.lock);

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char busEnum[ENUM_TO_STRING_LEN] = { 0 };
		char *defaultDsp = NULL;

		if (dsp_ptr == NULL) {
			continue;
		}

		esif_ccb_sprintf(ENUM_TO_STRING_LEN, busEnum, "%d", *dsp_ptr->bus_enum);
		
		defaultDsp = GetDSPFromList(query.participantName);

		// Primary weight indicates items that can result in a match
		primaryWeight =
			compare_and_weight_minterm(dsp_ptr->device_id, query.deviceId, PCI_DEVICE_ID_WEIGHT) +
			compare_and_weight_minterm(dsp_ptr->acpi_device, query.hid, ACPI_HID_WEIGHT) +
			compare_and_weight_minterm(dsp_ptr->acpi_type, query.ptype, ACPI_PTYPE_WEIGHT) +
			compare_and_weight_minterm(dsp_ptr->code_ptr, defaultDsp, PARTICIPANT_NAME_WEIGHT);

		// Secondary items can contribute only if at least one primary match is found
		secondaryWeight = 
			compare_and_weight_minterm(dsp_ptr->vendor_id, query.vendorId, PCI_VENDOR_ID_WEIGHT) +
			compare_and_weight_minterm(busEnum, query.participantType, PARTICIPANT_TYPE_WEIGHT);

		// Items in secondary can weigh in on the results but cannot be the sole factor
		if (primaryWeight < MIN_VIABLE_DSP_WEIGHT) {
			weight = primaryWeight;
		}
		else {
			weight = primaryWeight + secondaryWeight;
		}

		ESIF_TRACE_DEBUG("DSP: %s Weight: %d\n", (esif_string) dsp_ptr->code_ptr, weight);

		//Keep track of row with most weight.  
		if (weight > heaviest) {
			heaviest = weight;
			dspNamePtr = dsp_ptr->code_ptr;
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


/* Handle each weighted Minterm */
static int compare_and_weight_minterm(
	esif_string dsp,
	esif_string qry,
enum dspSelectorWeight weight
	)
{
	int score = 0;
	
	if (0 == *dsp || '*' == *dsp) {	/* DSP don't care trumps all */
		score = 0;
	}
	else if (0 == *qry || '*' == *qry) {	/* blank == blank don't care. */
		score = 0;
	}
	else if (0 == strcmp(dsp, qry)) {	/* data == data match. */
		score = (int) weight;
	}
	else {
		score = -1;	/* no match. */
	}
	ESIF_TRACE_DEBUG("DSP VALUE: %s VERSUS PARTICIPANT VALUE: %s  = SCORE: %d \n", dsp, qry, score);
	return score;
}

eEsifError EsifDspMgrInit()
{
	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_dm.lock);

	esif_dsp_table_build();
	EsifDspInit();

	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifDspMgrExit()
{
	ESIF_TRACE_ENTRY_INFO();

	EsifDspExit();
	esif_dsp_table_destroy();
	esif_ccb_lock_uninit(&g_dm.lock);

	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
