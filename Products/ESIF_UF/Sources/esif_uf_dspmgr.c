/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

#define ESIF_TRACE_DEBUG_DISABLED

#include "esif_uf.h"			/* Upper Framework */
#include "esif_dsp.h"			/* Device Support Package */
#include "esif_hash_table.h"	/* Hash Table */
#include "esif_uf_fpc.h"		/* Full Primitive Catalog */

#include "esif_lib_esifdata.h"
#include "esif_lib_databank.h"
#include "esif_uf_cfgmgr.h"

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

/* ESIF Memory Pool */
struct esif_ccb_mempool *g_mempool[ESIF_MEMPOOL_TYPE_MAX] = {0};


/* Allocate DSP Upper Insance */
static struct esif_up_dsp*esif_dsp_create (void)
{
	struct esif_up_dsp *dsp_ptr = NULL;
	dsp_ptr = (struct esif_up_dsp*)esif_ccb_malloc(sizeof(*dsp_ptr));
	return dsp_ptr;
}


/* Free DSP Upper Instance */
static void esif_dsp_destroy (struct esif_up_dsp *dsp_ptr)
{
	if (NULL == dsp_ptr) {
		return;
	}
	esif_hash_table_destroy(dsp_ptr->ht_ptr);
	esif_link_list_destroy(dsp_ptr->algo_ptr);
	esif_link_list_destroy(dsp_ptr->domain_ptr);
	esif_link_list_destroy(dsp_ptr->cap_ptr);
	esif_link_list_destroy(dsp_ptr->evt_ptr);
	esif_ccb_free(dsp_ptr);
}


/* DSP Interface */
static esif_string get_code (struct esif_up_dsp *dsp)
{
	if (dsp->code_ptr) {
		return dsp->code_ptr;
	} else {
		return "NA";
	}
}


static u8 get_ver_major (struct esif_up_dsp *dsp)
{
	if (dsp->ver_major_ptr) {
		return *dsp->ver_major_ptr;
	} else {
		return 0;
	}
}


static u8 get_ver_minor (struct esif_up_dsp *dsp)
{
	if (dsp->ver_minor_ptr) {
		return *dsp->ver_minor_ptr;
	} else {
		return 0;
	}
}


static u32 get_temp_c1 (
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


static u32 get_temp_c2 (
	struct esif_up_dsp *dsp,
	const enum esif_action_type action
	)
{
	/* TODO: Assume tempC2 Is Slope */
	struct esif_fpc_algorithm *algo_ptr = dsp->get_algorithm(dsp, action);
	if (algo_ptr) {
		return algo_ptr->tempC2;
	} else {
		return 0xffffffff;
	}
}


/* Insert Primitive */
static eEsifError insert_primitive (
	struct esif_up_dsp *dsp_ptr,
	EsifFpcPrimitive *primitive_ptr
	)
{
	if (NULL == primitive_ptr) {
		return ESIF_E_NULL_PRIMITIVE;
	} else {
		return esif_hash_table_put_item((UInt8*)&primitive_ptr->tuple,	/* Key */
										sizeof(struct esif_primitive_tuple),/* Size Of Key */
										primitive_ptr,						/* Item        */
										dsp_ptr->ht_ptr);						/* Hash Table  */
	}
}


static int is_same_primitive (
	const struct esif_primitive_tuple *t1_ptr,
	const struct esif_primitive_tuple *t2_ptr
	)
{
	if (NULL == t1_ptr || NULL == t2_ptr) {
		return 0;
	}
	return memcmp(t1_ptr, t2_ptr, sizeof(struct esif_primitive_tuple)) == 0;
}


static EsifFpcPrimitivePtr find_primitive_in_list (
	struct esif_link_list *list_ptr,
	const struct esif_primitive_tuple *tuple_ptr
	)
{
	EsifFpcPrimitivePtr cur_primitive_ptr = NULL;
	EsifFpcPrimitivePtr primitive_ptr     = NULL;
	struct esif_link_list_node *curr_ptr  = list_ptr->head_ptr;

	while (curr_ptr) {
		cur_primitive_ptr = (EsifFpcPrimitivePtr)curr_ptr->data_ptr;
		if (cur_primitive_ptr != NULL) {
			if (is_same_primitive(tuple_ptr, &cur_primitive_ptr->tuple)) {
				primitive_ptr = cur_primitive_ptr;
				goto end;
			}
		}
		curr_ptr = curr_ptr->next_ptr;
	}
end:
	return primitive_ptr;
}


/* Get Primtivie */
static EsifFpcPrimitivePtr get_primitive (
	struct esif_up_dsp *dsp_ptr,
	const struct esif_primitive_tuple *tuple_ptr
	)
{
	EsifFpcPrimitivePtr primitive_ptr = NULL;
	struct esif_link_list *row_ptr;

	row_ptr = esif_hash_table_get_item((u8*)tuple_ptr, sizeof(struct esif_primitive_tuple),
									   dsp_ptr->ht_ptr);
	if (NULL == row_ptr) {
		goto exit;
	}

	primitive_ptr = find_primitive_in_list(row_ptr, tuple_ptr);
	if (NULL == primitive_ptr) {
		goto exit;
	}
	return primitive_ptr;

exit:
	return NULL;
}


/* Get i-th Action */
static EsifFpcActionPtr get_action (
	EsifDspPtr dsp_ptr,
	EsifFpcPrimitivePtr primitive_ptr,
	UInt8 index
	)
{
	EsifFpcActionPtr action_ptr    = NULL;
	struct esif_link_list *row_ptr = NULL;
	int i;

	if (NULL == dsp_ptr || NULL == primitive_ptr) {
		return NULL;
	}

	row_ptr = esif_hash_table_get_item((UInt8*)&primitive_ptr->tuple,
									   sizeof(struct esif_primitive_tuple),
									   dsp_ptr->ht_ptr);

	if (NULL == row_ptr) {
		return NULL;
	}

	primitive_ptr = find_primitive_in_list(row_ptr, &primitive_ptr->tuple);
	if (NULL == primitive_ptr) {
		return NULL;
	}

	/* First Action */
	action_ptr = (EsifFpcActionPtr)((UInt8*)primitive_ptr + sizeof(EsifFpcPrimitive));

	/* Locate i-th Action We Are Looking For */
	for (i = 0; i < index; i++)
		action_ptr = (EsifFpcActionPtr)((UInt8*)action_ptr + action_ptr->size);

	return action_ptr;
}


/* Insert Algorithm Into Linked List */
static enum esif_rc insert_algorithm (
	struct esif_up_dsp *dsp_ptr,
	struct esif_fpc_algorithm *algo_ptr
	)
{
	if (NULL == algo_ptr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	esif_link_list_node_add(dsp_ptr->algo_ptr, esif_link_list_create_node((void*)algo_ptr));
	return ESIF_OK;
}


/* Insert Event Into Linked List */
static enum esif_rc insert_event (
	struct esif_up_dsp *dsp_ptr,
	struct esif_fpc_event *evt_ptr
	)
{
	if (NULL == evt_ptr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	esif_link_list_node_add(dsp_ptr->evt_ptr, esif_link_list_create_node((void*)evt_ptr));
	return ESIF_OK;
}


/* Get Algorithm From Linked List */
static struct esif_fpc_algorithm
*get_algorithm (
	struct esif_up_dsp *dsp_ptr,
	const enum esif_action_type action_type
	)
{
	struct esif_link_list *list_ptr = dsp_ptr->algo_ptr;
	struct esif_link_list_node *curr_ptr     = list_ptr->head_ptr;
	struct esif_fpc_algorithm *curr_algo_ptr = NULL;

	while (curr_ptr) {
		curr_algo_ptr = (struct esif_fpc_algorithm*)curr_ptr->data_ptr;
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
static struct esif_fpc_event
*get_event_by_type (
	struct esif_up_dsp *dsp_ptr,
	const enum esif_event_type event_type
	)
{
	struct esif_link_list *list_ptr      = dsp_ptr->evt_ptr;
	struct esif_link_list_node *curr_ptr = list_ptr->head_ptr;
	struct esif_fpc_event *curr_evt_ptr  = NULL;

	while (curr_ptr) {
		curr_evt_ptr = (struct esif_fpc_event*)curr_ptr->data_ptr;
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
static struct esif_fpc_event
*get_event_by_guid (
	struct esif_up_dsp *dsp_ptr,
	const esif_guid_t guid
	)
{
	struct esif_link_list *list_ptr      = dsp_ptr->evt_ptr;
	struct esif_link_list_node *curr_ptr = list_ptr->head_ptr;
	struct esif_fpc_event *curr_evt_ptr  = NULL;

	while (curr_ptr) {
		curr_evt_ptr = (struct esif_fpc_event*)curr_ptr->data_ptr;
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
static enum esif_rc insert_domain (
	struct esif_up_dsp *dsp_ptr,
	struct esif_fpc_domain *domain_ptr
	)
{
	if (NULL == domain_ptr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	esif_link_list_node_add(dsp_ptr->domain_ptr, esif_link_list_create_node((void*)domain_ptr));
	return ESIF_OK;
}


static UInt32 get_domain_count (struct esif_up_dsp *dsp_ptr)
{
	return *dsp_ptr->domain_count;
}


/* Get i-th Domain */
static struct esif_fpc_domain
*get_domain (
	struct esif_up_dsp *dsp_ptr,
	const u32 index
	)
{
	struct esif_link_list *list_ptr = dsp_ptr->domain_ptr;
	struct esif_link_list_node *curr_ptr    = list_ptr->head_ptr;
	struct esif_fpc_domain *curr_domain_ptr = NULL;
	u32 i;

	// printf("domain index %d\n", index);
	for (i = 0; i < index; i++) {
		curr_domain_ptr = (struct esif_fpc_domain*)curr_ptr->data_ptr;
		curr_ptr = curr_ptr->next_ptr;
		if (curr_ptr == NULL) {
			break;	// return NULL;
		}
	}
	// if( curr_domain_ptr) printf("domain = %s\n", curr_domain_ptr->descriptor.name);
	return curr_domain_ptr;
}


enum esif_rc esif_fpc_load (
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
	u8 *base_ptr    = (u8*)fpc_ptr;
	u32 num_prim    = 0, i, j;

	if (fpc_ptr->number_of_domains < 1) {
		printf("!ERR! %s: no domain error, number_of_domain = %d (less than 1)\n",
			   ESIF_FUNC, fpc_ptr->number_of_domains);
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	dsp_ptr->domain_count = &fpc_ptr->number_of_domains;

	/* Allocate Hash and List. Hash Size 31 Is Hard-Coded And Chosen By DSP Compiler */
	dsp_ptr->ht_ptr     = esif_hash_table_create(31);
	dsp_ptr->algo_ptr   = esif_link_list_create();
	dsp_ptr->domain_ptr = esif_link_list_create();
	dsp_ptr->cap_ptr    = esif_link_list_create();
	dsp_ptr->evt_ptr    = esif_link_list_create();

	if (!dsp_ptr->ht_ptr || !dsp_ptr->algo_ptr || !dsp_ptr->domain_ptr || !dsp_ptr->cap_ptr || !dsp_ptr->evt_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s <fpc @ %p> FPC name '%s' ver %x,%x desc '%s' size %u num_domains %d num_algorithms %d num_events %d",
			   ESIF_FUNC, fpc_ptr,
			   fpc_ptr->header.name,
			   fpc_ptr->header.ver_major,
			   fpc_ptr->header.ver_minor,
			   fpc_ptr->header.description,
			   fpc_ptr->size,
			   fpc_ptr->number_of_domains,
			   fpc_ptr->number_of_algorithms,
			   fpc_ptr->number_of_events);

	/* First Domain, Ok To Have Zero Primitive Of A Domain */
	domain_ptr = (struct esif_fpc_domain*)((u8*)fpc_ptr + sizeof(struct esif_fpc));
	for (i = 0; i < fpc_ptr->number_of_domains; i++) {
		offset = (unsigned long)((u8*)domain_ptr - base_ptr);
		ESIF_TRACE_DEBUG("<%04lu> Domain[%d] name %s size %d num_of_primitives %d  "
				   "num_of_capabilites %u (0x%x)",
				   offset, i, domain_ptr->descriptor.name, domain_ptr->size,
				   domain_ptr->number_of_primitives,
				   domain_ptr->capability_for_domain.number_of_capability_flags,
				   domain_ptr->capability_for_domain.capability_flags);

		/* Insert Domain Into Linked List */
		rc = dsp_ptr->insert_domain(dsp_ptr, domain_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}

		/* Capability */
		for (j = 0; j < domain_ptr->capability_for_domain.number_of_capability_flags; j++) {
			offset = (unsigned long)(((u8*)&domain_ptr->capability_for_domain) - base_ptr);
			ESIF_TRACE_DEBUG("<%04lu> Capability[%d] 0x%x", offset, j,
					   domain_ptr->capability_for_domain.capability_mask[j]);
		}

		/* First Primtive */
		primitive_ptr = (struct esif_fpc_primitive*)((u8*)domain_ptr +
													 sizeof(struct esif_fpc_domain));
		for (j = 0; j < domain_ptr->number_of_primitives; j++, num_prim++) {
			offset = (unsigned long)(((u8*)primitive_ptr) - base_ptr);
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
				goto exit;
			}
			/* Next Primitive */
			primitive_ptr = (struct esif_fpc_primitive*)((u8*)primitive_ptr +
														 primitive_ptr->size);
		}
		/* Next Domain */
		domain_ptr = (struct esif_fpc_domain*)((u8*)domain_ptr + domain_ptr->size);
	}

	/* First Algorithm (Laid After The Last Domain) */
	algorithm_ptr = (struct esif_fpc_algorithm*)domain_ptr;
	for (i = 0; i < fpc_ptr->number_of_algorithms; i++) {
		offset = (unsigned long)((u8*)algorithm_ptr - base_ptr);
		ESIF_TRACE_DEBUG("<%04lu> Algorithm[%03d]:  action_type %u(%s) temp_xform %u "
				   "tempC1 %u tempC2 %u size %u",
				   offset, i,
				   algorithm_ptr->action_type,
				   esif_action_type_str(algorithm_ptr->action_type),
				   algorithm_ptr->temp_xform,
				   algorithm_ptr->tempC1,
				   algorithm_ptr->tempC2,
				   algorithm_ptr->size);

		/* Insert Algorithm Into Linked List */
		rc = dsp_ptr->insert_algorithm(dsp_ptr, algorithm_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}

		/* Next Algorithm */
		algorithm_ptr = (struct esif_fpc_algorithm*)((u8*)algorithm_ptr +
													 sizeof(struct esif_fpc_algorithm));
	}

	/* First Event (Laid After The Last Algorithm) */
	event_ptr = (struct esif_fpc_event*)algorithm_ptr;
	for (i = 0; i < fpc_ptr->number_of_events; i++) {
		offset = (unsigned long)((u8*)event_ptr - base_ptr);
		ESIF_TRACE_DEBUG("<%04lu> Event [%03d] type %s(%d)\n",
				   offset, i, esif_event_type_str(event_ptr->esif_event), event_ptr->esif_event);

		/* Insert Algorithm Into Linked List */
		rc = dsp_ptr->insert_event(dsp_ptr, event_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}

		/* Next Event */
		event_ptr = (struct esif_fpc_event*)((u8*)event_ptr +
											 sizeof(struct esif_fpc_event));
	}

exit:
	ESIF_TRACE_DEBUG("%s: %u domains, %u primitives and %u algorithms %u events inserted! status %s",
			   ESIF_FUNC, fpc_ptr->number_of_domains, num_prim, fpc_ptr->number_of_algorithms, fpc_ptr->number_of_events,
			   esif_rc_str(rc));
	return rc;
}


/* Add DSP Entry */
static eEsifError esif_dsp_entry_create (struct esif_ccb_file *file_ptr)
{
	struct esif_up_dsp *dsp_ptr = NULL;
	struct esif_fpc *fpc_ptr    = NULL;
	u32 fpc_static = ESIF_FALSE;
	eEsifError rc  = ESIF_E_UNSPECIFIED;
	UInt8 i = 0;
	char path[MAX_PATH];
	UInt32 fpc_size = 0, edp_size = 0;
	size_t fpc_read = 0;
	struct edp_dir edp_dir;
	EsifDataPtr nameSpace = 0;
	EsifDataPtr key = 0;
	EsifDataPtr value     = 0;
	IOStreamPtr io_ptr    = IOStream_Create();

	if ((NULL == file_ptr) || (NULL == io_ptr)) {
		goto exit;
	}
	nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, ESIF_DSP_NAMESPACE, 0, ESIFAUTOLEN);
	key = EsifData_CreateAs(ESIF_DATA_STRING, file_ptr->filename, 0, ESIFAUTOLEN);
	value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

	ESIF_TRACE_DEBUG("%s: Filename: %s", ESIF_FUNC, file_ptr->filename);

	dsp_ptr = esif_dsp_create();
	if (NULL == dsp_ptr) {
		goto exit;
	}

	// Look for EDP file either on disk or in a DataVault (static or file), depending on priority setting
	esif_ccb_sprintf(MAX_PATH, path, "%s%s%s", esif_build_path(path, MAX_PATH, ESIF_DIR_DSP, NULL), ESIF_PATH_SEP, file_ptr->filename);
	if ((ESIF_EDP_DV_PRIORITY == 1 || !esif_ccb_file_exists(path)) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		esif_ccb_strcpy(path, file_ptr->filename, MAX_PATH);
		IOStream_SetMemory(io_ptr, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(io_ptr, path, "rb");
	}
	ESIF_TRACE_DEBUG("%s: Fullpath: %s", ESIF_FUNC, path);

	if (IOStream_Open(io_ptr) != 0) {
		ESIF_TRACE_DEBUG("%s: file not found (%s)", ESIF_FUNC, path);
		goto exit;
	}

	/* Read FPC From EDP File */
	if (esif_ccb_strstr(&path[0], ".edp")) {
		/* EDP - Only Read The FPC Part */
		edp_size = (UInt32)IOStream_GetSize(io_ptr);
		fpc_read = IOStream_Read(io_ptr, &edp_dir, sizeof(struct edp_dir));
		fpc_size = edp_size - edp_dir.fpc_offset;
		if (edp_size > MAX_EDP_SIZE || fpc_size > MAX_FPC_SIZE) {
			goto exit;
		}
		IOStream_Seek(io_ptr, edp_dir.fpc_offset, SEEK_SET);
		ESIF_TRACE_DEBUG("%s: file found (%s) size %u, FPC size %u from offset %u", ESIF_FUNC, path, edp_size, fpc_size, edp_dir.fpc_offset);
	} else {
		ESIF_TRACE_DEBUG("%s: file %s does not have .fpc and .edp format!", ESIF_FUNC, path, fpc_size);
	}

	// use static DataVault buffer (if available), otherwise allocate space for our FPC file contents (which will not be freed)
	if (IOStream_GetType(io_ptr) == StreamMemory && value->buf_len == 0) {
		fpc_ptr    = (struct esif_fpc*)(IOStream_GetMemoryBuffer(io_ptr) + IOStream_GetOffset(io_ptr));
		fpc_read   = fpc_size;
		ESIF_TRACE_DEBUG("%s: static vault size %u buf_ptr=0x%p\n", ESIF_FUNC, (int)fpc_read, fpc_ptr);
		fpc_static = ESIF_TRUE;
	} else {
		fpc_ptr = (struct esif_fpc*)esif_ccb_malloc(fpc_size);
		if (NULL == fpc_ptr) {
			ESIF_TRACE_DEBUG("%s: malloc failed to allocate %u bytes\n", ESIF_FUNC, fpc_size);
			goto exit;
		}
		ESIF_TRACE_DEBUG("%s: file malloc size %u", ESIF_FUNC, fpc_size);

		// read file contents
		fpc_read = IOStream_Read(io_ptr, fpc_ptr, fpc_size);
		if (fpc_read < fpc_size) {
			ESIF_TRACE_DEBUG("%s: read short received %u of %u bytes\n", ESIF_FUNC, (int)fpc_read, fpc_size);
			goto exit;
		}

		ESIF_TRACE_DEBUG("%s: file read size %u", ESIF_FUNC, (int)fpc_read);
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
	ESIF_TRACE_DEBUG("PCI Function:   -1x%02x", fpc_ptr->header.pci_function);

	dsp_ptr->code_ptr = (EsifString)fpc_ptr->header.name;
	dsp_ptr->bus_enum = (UInt8*)&fpc_ptr->header.bus_enum;
	dsp_ptr->type     = (EsifString)fpc_ptr->header.type;
	dsp_ptr->ver_major_ptr  = (UInt8*)&fpc_ptr->header.ver_major;
	dsp_ptr->ver_minor_ptr  = (UInt8*)&fpc_ptr->header.ver_minor;
	dsp_ptr->acpi_device    = (EsifString)fpc_ptr->header.acpi_device;
	dsp_ptr->acpi_scope     = (EsifString)fpc_ptr->header.acpi_scope;
	dsp_ptr->acpi_type      = (EsifString)fpc_ptr->header.acpi_type;
	dsp_ptr->vendor_id      = (EsifString)fpc_ptr->header.pci_vendor_id;
	dsp_ptr->device_id      = (EsifString)fpc_ptr->header.pci_device_id;
	dsp_ptr->pci_bus = (UInt8*)&fpc_ptr->header.pci_bus;
	dsp_ptr->pci_bus_device = (UInt8*)&fpc_ptr->header.pci_device;
	dsp_ptr->pci_function   = (UInt8*)&fpc_ptr->header.pci_function;

	/* Assign Function Pointers */
	dsp_ptr->get_code = get_code;
	dsp_ptr->get_ver_minor     = get_ver_minor;
	dsp_ptr->get_ver_major     = get_ver_major;
	dsp_ptr->get_temp_tc1      = get_temp_c1;
	dsp_ptr->get_temp_tc2      = get_temp_c2;

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
		ESIF_TRACE_DEBUG("%s: FPC %s load successfully", ESIF_FUNC, path);
	} else {
		ESIF_TRACE_DEBUG("%s: Unable to load FPC %s, rc %s",
				   ESIF_FUNC, path, esif_rc_str(rc));
	}

	/* Lock DSP Manager */
	esif_ccb_write_lock(&g_dm.lock);

	/* Simple Table Lookup For Now. Scan Table And Find First Empty Slot */
	/* Empty slot indicated by AVAILABLE state                           */
	for (i = 0; i < MAX_DSP_MANAGER_ENTRY; i++)
		if (NULL == g_dm.dme[i].dsp_ptr) {
			break;
		}


	/* If No Available Slots Return */
	if (i >= MAX_DSP_MANAGER_ENTRY) {
		esif_ccb_write_unlock(&g_dm.lock);
		goto exit;
	}

	/*
	** Take Slot
	*/
	g_dm.dme[i].dsp_ptr  = dsp_ptr;
	g_dm.dme[i].file_ptr = file_ptr;
	g_dm.dme[i].fpc_ptr = (fpc_static ? 0 : fpc_ptr);
	g_dm.dme_count++;
	dsp_ptr = NULL;	// Deallocate on exit
	fpc_ptr = NULL;	// Deallocate on exit

	esif_ccb_write_unlock(&g_dm.lock);
	rc = ESIF_OK;

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


static eEsifError esif_dsp_file_scan ()
{
	eEsifError rc = ESIF_OK;
	struct esif_ccb_file *ffd_ptr = NULL;
	esif_ccb_file_find_handle find_handle = INVALID_HANDLE_VALUE;
	char path[MAX_PATH]    = {0};
	char pattern[MAX_PATH] = {0};
	int files = 0;
	StringPtr namesp = ESIF_DSP_NAMESPACE;
	DataVaultPtr DB  = DataBank_GetNameSpace(g_DataBankMgr, namesp);

	// 1. Load all EDP's in the DSP Configuration Namespace, if any exist
	if (DB) {
		UInt32 context        = 0;
		EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_AUTO, namesp, 0, ESIFAUTOLEN);
		EsifDataPtr key       = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		EsifDataPtr value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

		ESIF_TRACE_DEBUG("%s: SCAN CONFIG For DSP Files NameSpace = %s, Pattern %s", ESIF_FUNC, (char*)nameSpace->buf_ptr, pattern);
		if ((rc = EsifConfigFindFirst(nameSpace, key, value, &context)) == ESIF_OK) {
			do {
				// Load all keys from the DataVault with an ".edp" extension
				if (key->data_len >= 5 && esif_ccb_stricmp(((StringPtr)(key->buf_ptr)) + key->data_len - 5, ".edp") == 0) {
					ffd_ptr = (struct esif_ccb_file*)esif_ccb_malloc(sizeof(*ffd_ptr));
					esif_ccb_strcpy(ffd_ptr->filename, (StringPtr)key->buf_ptr, sizeof(ffd_ptr->filename));
					esif_dsp_entry_create(ffd_ptr);
					files++;
				}
				EsifData_Set(key, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
				EsifData_Set(value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
			} while ((rc = EsifConfigFindNext(nameSpace, key, value, &context)) == ESIF_OK);
			if (rc == ESIF_E_NOT_FOUND) {
				rc = ESIF_OK;
			}
		}
		EsifData_Destroy(nameSpace);
		EsifData_Destroy(key);
		EsifData_Destroy(value);
	}

	// 2. Load all EDP's from the DSP folder, if any exist, except ones already loaded from DataBank
	esif_build_path(path, MAX_PATH, ESIF_DIR_DSP, NULL);
	esif_ccb_strcpy(pattern, "*.edp", MAX_PATH);

	ESIF_TRACE_DEBUG("%s: SCAN File System For DSP Files Path = %s, Pattern %s", ESIF_FUNC, path, pattern);
	/* Find the first file in the directory that matches are search */
	ffd_ptr = (struct esif_ccb_file*)esif_ccb_malloc(sizeof(*ffd_ptr));
	if (ffd_ptr == NULL) {
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
			ffd_ptr = (struct esif_ccb_file*)esif_ccb_malloc(sizeof(*ffd_ptr));
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
static eEsifError esif_dsp_table_build ()
{
	eEsifError rc = ESIF_OK;
	ESIF_TRACE_DEBUG("%s: Build DSP Table", ESIF_FUNC);
	esif_dsp_file_scan();
	return rc;
}


void esif_dsp_table_destroy ()
{
	UInt8 i;
	ESIF_TRACE_DEBUG("%s: Destroy DSP Table", ESIF_FUNC);
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

esif_string esif_uf_dm_select_dsp (
	eEsifParticipantOrigin origin,
	void *piPtr
	)
{
	esif_string selection = NULL;
	EsifParticipantIfacePtr pi_ptr = (EsifParticipantIfacePtr)piPtr;
	esif_string name = pi_ptr->name;

	UNREFERENCED_PARAMETER(origin);

	if (!strcmp(name, "TFN1")) {
		selection = (esif_string)"sb_tfan";
	} else if (!strcmp(name, "TFN2")) {
		selection = (esif_string)"sb_tfan";
	} else if (!strcmp(name, "TMEM")) {
		selection = (esif_string)"sb_tmem";
	} else if (!strcmp(name, "TAMB")) {
		selection = (esif_string)"sb_temp";
	} else if (!strcmp(name, "TEFN")) {
		selection = (esif_string)"sb_temp";
	} else if (!strcmp(name, "TSKN")) {
		selection = (esif_string)"sb_temp";
	} else if (!strcmp(name, "T_VR")) {
		selection = (esif_string)"sb_temp";
	} else if (!strcmp(name, "FGEN")) {
		selection = (esif_string)"sb_fgen";
	} else if (!strcmp(name, "DPLY")) {
		selection = (esif_string)"sb_dply";
	} else if (!strcmp(name, "TPWR")) {
		selection = (esif_string)"sb_tpwr";
	} else if (!strcmp(name, "WIFI")) {
		selection = (esif_string)"sb_wifi";
	} else if (!strcmp(name, "WGIG")) {
		selection = (esif_string)"sb_wgig";
	} else if (!strcmp(name, "WWAN")) {
		selection = (esif_string)"sb_wwan";
	} else if (!strcmp(name, "TINL")) {
		selection = (esif_string)"sb_temp";
	} else if (!strcmp(name, "TCPU")) {
		selection = (esif_string)"sb_b0_d4_f0";
	} else if (!strcmp(name, "TPCH")) {
		selection = (esif_string)"sb_b0_d1f_f6";
	} else if (!strcmp(name, "IETM")) {
		selection = (esif_string)"sb_ietm";
	} else {
		selection = (esif_string)"sb_fgen";
	}

#ifdef ESIF_ATTR_OS_ANDROID
	if (!strcmp(name, "IETM")) {
		selection = (esif_string)"slb_ietm";
	} else {
		selection = (esif_string)"slb_temp";
	}
#endif

	return selection;
}


struct esif_up_dsp*esif_uf_dm_select_dsp_by_code (esif_string code)
{
	UInt8 i = 0;
	struct esif_up_dsp *found_ptr = NULL;

	for (i = 0; i < g_dm.dme_count; i++)
		if (!strcmp(code, g_dm.dme[i].dsp_ptr->code_ptr)) {
			found_ptr = g_dm.dme[i].dsp_ptr;
			break;
		}

	return found_ptr;
}


/* Handle each weighted Minterm */
static int compare_and_weight_minterm (
	esif_string dsp,
	esif_string qry,
	u8 weight
	)
{
	int score = 0;
	if (0 == *dsp || '*' == *dsp) {	/* DSP don't care trumps all */
		score = 0;
	} else if (0 == *dsp && (0 == *qry || '*' == *qry)) {	/* blank == blank don't care. */
		score = 0;
	} else if (0 == strcmp(dsp, qry)) {	/* data == data match. */
		score = weight;
	} else {
		score = -1;	/* no match. */
	}
	return score;
}


/* ACPI Weighted Query */
static int weighted_acpi_eq (
	struct esif_up_dsp *dsp_ptr,
	struct esif_uf_dm_query_acpi *qry_ptr
	)
{
	int weight  = 0;
	int minterm = 0;

	minterm = compare_and_weight_minterm(dsp_ptr->acpi_device, qry_ptr->acpi_device, 8);
	if (minterm == -1) {
		goto no_match;
	}
	weight += minterm;

	minterm = compare_and_weight_minterm(dsp_ptr->acpi_type, qry_ptr->acpi_type, 4);
	if (minterm == -1) {
		goto no_match;
	}
	weight += minterm;

	minterm = compare_and_weight_minterm("", qry_ptr->acpi_uid, 2);
	if (minterm == -1) {
		goto no_match;
	}
	weight += minterm;

	minterm = compare_and_weight_minterm(dsp_ptr->acpi_scope, qry_ptr->acpi_scope, 1);
	if (minterm == -1) {
		goto no_match;
	}
	weight += minterm;

	return weight;

no_match:
	return -1;
}


/* PLAT Weighted Query */
static int weighted_plat_eq (
	struct esif_up_dsp *dsp_ptr,
	struct esif_uf_dm_query_plat *qry_ptr
	)
{
	int weight  = 0;
	int minterm = 0;

	minterm = compare_and_weight_minterm(dsp_ptr->acpi_device, qry_ptr->plat_type, 2);
	if (minterm == -1) {
		goto no_match;
	}
	weight += minterm;

	minterm = compare_and_weight_minterm("", qry_ptr->plat_name, 1);
	if (minterm == -1) {
		goto no_match;
	}
	weight += minterm;

	return weight;

no_match:
	return -1;
}


/*
** Query the upper framework dsp manager to find the BEST DSP match for the query
** critieria provided. This query is based on a weighted N Minterm boolean equation.
** The DSP may indicate don't cares by using blanks.  Query's may indicate don't
** haves with blanks as well.
*/
EsifString esif_uf_dm_query (
	enum esif_uf_dm_query_type query_type,
	void *qry_ptr
	)
{
	int i = 0;							// Loop index for DSP table.
	int weight   = 0;						// Current weight of DSP table row.
	int heaviest = -1;					// Heaviest Row Found.
	EsifString dsp_code_ptr = NULL;		// DSP Name/Code To Use.
	int best     = 0;						// Best Weight Possible.

	/*
	** Query table for weighted match highest score wins. It is
	** unlikely that the table would chnage but we lock it just
	** to be sure it can't.
	*/

	// LOCK
	esif_ccb_read_lock(&g_dm.lock);

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;

		// Skip NULL and Non Query Type Rows
		if (dsp_ptr == NULL || *dsp_ptr->bus_enum != query_type) {
			continue;
		}

		switch (query_type) {
		case ESIF_UF_DM_QUERY_TYPE_ACPI:
			best   = 16 + 8 + 4 + 2 + 1;// 5 minterms
			weight = weighted_acpi_eq(dsp_ptr, (struct esif_uf_dm_query_acpi*)qry_ptr);
			break;

		// TBD
		case ESIF_UF_DM_QUERY_TYPE_PCI:
			break;

		// TBD
		case ESIF_UF_DM_QUERY_TYPE_PLATFORM:
			best   = 2 + 1;
			weight = weighted_plat_eq(dsp_ptr, (struct esif_uf_dm_query_plat*)qry_ptr);
			break;
		}

		printf("%s: DSP: %s Weight: %d\n", ESIF_FUNC, (esif_string)dsp_ptr->code_ptr, weight);

		/*
		** Keep track of row with most weight known as heaviest.  Note that once we
		** find the heaviest row anoher row must be heavier if it is the same we use
		** the first row.  An optimization code be to check for a perfect match / weight
		** and bail out early.  For now we scan the entire table an return the best/weighted
		** DSP as our query result.
		*/
		if (weight > heaviest) {
			heaviest     = weight;
			dsp_code_ptr = dsp_ptr->code_ptr;
		}
	}

	// UNLOCK
	esif_ccb_read_unlock(&g_dm.lock);

	if (dsp_code_ptr != NULL) {
		printf("%s: Selected DSP: %s Score: %d of %d\n", ESIF_FUNC, dsp_code_ptr, heaviest, best);
	} else {
		printf("%s: No DSP selected; DME count must be 0\n", ESIF_FUNC);
	}
	return dsp_code_ptr;
}


eEsifError EsifDspMgrInit ()
{
	ESIF_TRACE_DEBUG("%s: Init DSP Manager(DSPMGR)", ESIF_FUNC);
	esif_ccb_lock_init(&g_dm.lock);

	/* Initialize Hash For FPC Entry */
	esif_link_list_init();
	esif_hash_table_init();
	esif_dsp_table_build();
	EsifDspInit();
	return ESIF_OK;
}


void EsifDspMgrExit ()
{
	EsifDspExit();
	esif_dsp_table_destroy();
	esif_hash_table_exit();
	esif_link_list_exit();
	esif_ccb_lock_uninit(&g_dm.lock);
	ESIF_TRACE_DEBUG("%s: Exit DSP Manager (DSPMGR)", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
