/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#include "esif_ipc.h"

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 *
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified
 * against Windows SDK/DDK included headers which we have no control over.
 *
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Debug Logging Defintions */
#define INIT_DEBUG        0	/* Init Debug */
#define COMMAND_DEBUG     1	/* Command Debug */
#define DECODE_DEBUG      2	/* Decode Debug */

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_COMMAND, \
		       INIT_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_COMMAND(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_COMMAND, \
		       COMMAND_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_DECODE(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_COMMAND, \
		       DECODE_DEBUG, \
		       format, \
		       ##__VA_ARGS__)

/*
 ******************************************************************************
 * PRIVATE
 ******************************************************************************
 */

/* Set Debug Modules */
static void esif_execute_ipc_command_set_debug_modules(
	struct esif_ipc_command *command_ptr
	)
{
	/* Sanity Check */
	if (ESIF_DATA_UINT32 == command_ptr->req_data_type &&
	    0 == command_ptr->req_data_offset &&
	    sizeof(struct esif_command_debug_modules) ==
	    command_ptr->req_data_len) {
		struct esif_command_debug_modules *data_ptr =
			(struct esif_command_debug_modules *)(command_ptr + 1);

		ESIF_TRACE_DYN_COMMAND(
			"%s: ESIF_COMMAND_TYPE_SET_DEBUG_MODULES "
			"Modules: 0x%08X\n",
			ESIF_FUNC,
			data_ptr->modules);

		esif_debug_set_modules(data_ptr->modules);
		command_ptr->return_code = ESIF_OK;
	}
}


/* Set Debug Module Level */
static void esif_execute_ipc_command_set_debug_module_level(
	struct esif_ipc_command *command_ptr
	)
{
	/* Sanity Check */
	if (ESIF_DATA_STRUCTURE == command_ptr->req_data_type &&
	    0 == command_ptr->req_data_offset &&
	    sizeof(struct esif_command_set_debug_module_level) ==
	    command_ptr->req_data_len) {
		struct esif_command_set_debug_module_level *data_ptr =
			(struct esif_command_set_debug_module_level *)
			(command_ptr + 1);

		ESIF_TRACE_DYN_COMMAND(
			"%s: ESIF_COMMAND_TYPE_SET_DEBUG_MODULE_LEVEL "
			"Module: %d Level: 0x%08X\n",
			ESIF_FUNC,
			data_ptr->module,
			data_ptr->level);

		esif_debug_set_module_category(data_ptr->module,
					       data_ptr->level);
		command_ptr->return_code = ESIF_OK;
	}
}


/* Get Debug Module Level */
static void esif_execute_ipc_command_get_debug_module_level(
	struct esif_ipc_command *command_ptr
	)
{
	/* Sanity Check */
	if (ESIF_DATA_STRUCTURE == command_ptr->rsp_data_type &&
	    0 == command_ptr->rsp_data_offset &&
	    sizeof(struct esif_command_get_debug_module_level) ==
	    command_ptr->rsp_data_len) {
		struct esif_command_get_debug_module_level *data_ptr =
			(struct esif_command_get_debug_module_level *)
			(command_ptr + 1);

		data_ptr->modules = g_esif_module_mask;
		esif_ccb_memcpy(&data_ptr->levels,
				&g_esif_module_category_mask,
				sizeof(g_esif_module_category_mask));

		ESIF_TRACE_DYN_COMMAND(
			"%s: ESIF_COMMAND_TYPE_GET_DEBUG_MODULE_LEVEL "
			"modules 0x%08X\n",
			ESIF_FUNC,
			data_ptr->modules);

		command_ptr->return_code = ESIF_OK;
	}
}


/* Get Kernel Information */
static void esif_execute_ipc_command_get_kernel_info(
	struct esif_ipc_command *command_ptr
	)
{
	/* Sanity Check */
	if (ESIF_DATA_STRUCTURE == command_ptr->rsp_data_type &&
	    0 == command_ptr->rsp_data_offset &&
	    sizeof(struct esif_command_get_kernel_info) ==
	    command_ptr->rsp_data_len) {
		struct esif_command_get_kernel_info *data_ptr =
			(struct esif_command_get_kernel_info *)
			(command_ptr + 1);

		esif_ccb_strcpy(data_ptr->ver_str, ESIF_VERSION, ESIF_NAME_LEN);

		ESIF_TRACE_DYN_COMMAND(
			"%s: ESIF_COMMAND_TYPE_GET_KERNEL_INFO\n",
			ESIF_FUNC);

		command_ptr->return_code = ESIF_OK;
	}
}


/* Get Kernel Information */
static void esif_execute_ipc_command_get_memory_stats(
	struct esif_ipc_command *command_ptr
	)
{
	/* Sanity Check */
	if (ESIF_DATA_STRUCTURE == command_ptr->rsp_data_type &&
	    0 == command_ptr->rsp_data_offset &&
	    sizeof(struct esif_command_get_memory_stats) ==
	    command_ptr->rsp_data_len) {

		struct esif_command_get_memory_stats *data_ptr =
			(struct esif_command_get_memory_stats *)
			(command_ptr + 1);
		u32 reset = *(u32 *)data_ptr;

		if (reset) {
			esif_ccb_memset(&data_ptr->stats, 0,
					sizeof(struct esif_memory_stats));

			esif_ccb_write_lock(&g_memstat_lock);
			esif_ccb_memset(&g_memstat, 0,
					sizeof(struct esif_memory_stats));
			esif_ccb_write_unlock(&g_memstat_lock);
		} else {
			int i = 0;

			esif_ccb_read_lock(&g_memstat_lock);
			esif_ccb_memcpy(&data_ptr->stats, &g_memstat,
					sizeof(struct esif_memory_stats));
			esif_ccb_read_unlock(&g_memstat_lock);

			esif_ccb_read_lock(&g_mempool_lock);
			for (i = 0; i < ESIF_MEMPOOL_TYPE_MAX; i++) {
				if (NULL == g_mempool[i])
					continue;	/* Skip Unused */

				esif_ccb_strcpy(data_ptr->mempool_stat[i].name,
						g_mempool[i]->name_ptr,
						ESIF_NAME_LEN);
				data_ptr->mempool_stat[i].pool_tag    =
					g_mempool[i]->pool_tag;
				data_ptr->mempool_stat[i].object_size =
					g_mempool[i]->object_size;
				data_ptr->mempool_stat[i].alloc_count =
					g_mempool[i]->alloc_count;
				data_ptr->mempool_stat[i].free_count  =
					g_mempool[i]->free_count;
			}
			esif_ccb_read_unlock(&g_mempool_lock);

			esif_ccb_read_lock(&g_memtype_lock);
			for (i = 0; i < ESIF_MEMTYPE_TYPE_MAX; i++) {
				if (NULL == g_memtype[i])
					continue;	/* Skip Unused */

				esif_ccb_strcpy(data_ptr->memtype_stat[i].name,
						g_memtype[i]->name_ptr,
						ESIF_NAME_LEN);
				data_ptr->memtype_stat[i].pool_tag    =
					g_memtype[i]->type_tag;
				data_ptr->memtype_stat[i].alloc_count =
					g_memtype[i]->alloc_count;
				data_ptr->memtype_stat[i].free_count  =
					g_memtype[i]->free_count;
			}
			esif_ccb_read_unlock(&g_memtype_lock);
		}

		ESIF_TRACE_DYN_COMMAND(
			"%s: ESIF_COMMAND_TYPE_GET_MEMORY_STATS reset %d\n",
			ESIF_FUNC,
			reset);

		command_ptr->return_code = ESIF_OK;
	}
}


/* Get Participants */
static void esif_execute_ipc_command_get_participants(
	struct esif_ipc_command *command_ptr
	)
{
	/* Sanity Check */
	if (ESIF_DATA_STRUCTURE == command_ptr->rsp_data_type &&
	    0 == command_ptr->rsp_data_offset &&
	    sizeof(struct esif_command_get_participants) ==
	    command_ptr->rsp_data_len) {
		u8 i = 0;
		struct esif_command_get_participants *data_ptr =
			(struct esif_command_get_participants *)
			(command_ptr + 1);

		ESIF_TRACE_DYN_COMMAND(
			"%s: ESIF_COMMAND_TYPE_GET_PARTICIPANTS\n",
			ESIF_FUNC);

		for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
			struct esif_lp *lp_ptr =
				esif_lf_pm_lp_get_by_instance_id(i);
			if (NULL != lp_ptr) {
				data_ptr->participant_info[i].id = i;
				esif_ccb_strcpy(
					data_ptr->participant_info[i].name,
					lp_ptr->pi_ptr->name,
					ESIF_NAME_LEN);
				esif_ccb_strcpy(
					data_ptr->participant_info[i].desc,
					lp_ptr->pi_ptr->desc,
					ESIF_DESC_LEN);
				esif_ccb_memcpy(
					data_ptr->participant_info[i].class_guid,
					lp_ptr->pi_ptr->class_guid,
					ESIF_GUID_LEN);
				data_ptr->participant_info[i].version  =
					lp_ptr->pi_ptr->version;
				data_ptr->participant_info[i].bus_enum =
					lp_ptr->pi_ptr->enumerator;
				data_ptr->participant_info[i].state    =
					esif_lf_pm_lp_get_state(lp_ptr);
				if (NULL != lp_ptr->dsp_ptr) {
					esif_ccb_strcpy(
					    data_ptr->participant_info[i].dsp_code,
					    lp_ptr->dsp_ptr->get_code(
					    lp_ptr->dsp_ptr),
					    ESIF_DSP_NAME_LEN);
					data_ptr->participant_info[i].dsp_ver_major =
						lp_ptr->dsp_ptr->get_ver_major(
							lp_ptr->dsp_ptr);
					data_ptr->participant_info[i].
					dsp_ver_minor =
						lp_ptr->dsp_ptr->get_ver_minor(
							lp_ptr->dsp_ptr);
				}
			} else {
				data_ptr->participant_info[i].id = i;
				if (1 == i) {
					esif_ccb_strcpy(
						data_ptr->participant_info[i].desc,
						"RESERVED",
						ESIF_DESC_LEN);
				} else {
				}
			}
		}
		data_ptr->count = i;
		command_ptr->return_code = ESIF_OK;
	}
}


/* Get Participant */
static void esif_execute_ipc_command_get_participant_detail(
	struct esif_ipc_command *command_ptr
	)
{
	/* Sanity Check */
	if (ESIF_DATA_STRUCTURE == command_ptr->rsp_data_type &&
	    0 == command_ptr->rsp_data_offset &&
	    sizeof(struct esif_command_get_participant_detail) ==
	    command_ptr->rsp_data_len) {
		struct esif_command_get_participant_detail *data_ptr =
			(struct esif_command_get_participant_detail *)(
				command_ptr + 1);

		/* ID Will Be In Buffer ON Arrival */
		u32 participant_id = *(u32 *) data_ptr;

		ESIF_TRACE_DYN_COMMAND(
			"%s: ESIF_COMMAND_TYPE_PARTICIPANT_DETAIL id %d\n",
			ESIF_FUNC,
			participant_id);
		{
			struct esif_lp *lp_ptr =
			   esif_lf_pm_lp_get_by_instance_id((u8)participant_id);

			if (NULL != lp_ptr) {
				/* Participant Info */
				data_ptr->id         = participant_id;
				data_ptr->version    = lp_ptr->pi_ptr->version;
				data_ptr->enumerator =
					lp_ptr->pi_ptr->enumerator;
				esif_ccb_strcpy(data_ptr->name,
						lp_ptr->pi_ptr->name,
						ESIF_NAME_LEN);
				esif_ccb_strcpy(data_ptr->desc,
						lp_ptr->pi_ptr->desc,
						ESIF_DESC_LEN);
				esif_ccb_strcpy(data_ptr->driver_name,
						lp_ptr->pi_ptr->driver_name,
						ESIF_NAME_LEN);
				esif_ccb_strcpy(data_ptr->device_name,
						lp_ptr->pi_ptr->device_name,
						ESIF_NAME_LEN);
				esif_ccb_strcpy(data_ptr->device_path,
						lp_ptr->pi_ptr->device_path,
						ESIF_PATH_LEN);
				esif_ccb_memcpy(data_ptr->class_guid,
						lp_ptr->pi_ptr->class_guid,
						ESIF_GUID_LEN);
				data_ptr->flags = lp_ptr->pi_ptr->flags;

				/* ACPI */
				esif_ccb_strcpy(data_ptr->acpi_device,
						lp_ptr->pi_ptr->acpi_device,
						ESIF_NAME_LEN);
				esif_ccb_strcpy(data_ptr->acpi_scope,
						lp_ptr->pi_ptr->acpi_scope,
						ESIF_SCOPE_LEN);
				data_ptr->acpi_uid  = lp_ptr->pi_ptr->acpi_uid;
				data_ptr->acpi_type = lp_ptr->pi_ptr->acpi_type;

				/* PCI */
				data_ptr->pci_vendor     =
					(u16)lp_ptr->pi_ptr->pci_vendor;
				data_ptr->pci_device     =
					(u16)lp_ptr->pi_ptr->pci_device;
				data_ptr->pci_bus        =
					lp_ptr->pi_ptr->pci_bus;
				data_ptr->pci_bus_device =
					lp_ptr->pi_ptr->pci_bus_device;
				data_ptr->pci_function   =
					lp_ptr->pi_ptr->pci_function;
				data_ptr->pci_revision   =
					lp_ptr->pi_ptr->pci_revision;
				data_ptr->pci_class      =
					lp_ptr->pi_ptr->pci_class;
				data_ptr->pci_sub_class  =
					lp_ptr->pi_ptr->pci_sub_class;
				data_ptr->pci_prog_if    =
					lp_ptr->pi_ptr->pci_prog_if;

				/* LP */
				data_ptr->state        =
					esif_lf_pm_lp_get_state(lp_ptr);
				data_ptr->timer_period =
					lp_ptr->domains[0].timer_period_msec;

				if (NULL != lp_ptr->dsp_ptr) {
					/* Have DSP */
					data_ptr->have_dsp = 1;
					esif_ccb_strcpy(data_ptr->dsp_code,
							lp_ptr->dsp_ptr->get_code(
							lp_ptr->dsp_ptr),
							ESIF_DSP_NAME_LEN);
					data_ptr->dsp_ver_major =
						lp_ptr->dsp_ptr->get_ver_major(
							lp_ptr->dsp_ptr);
					data_ptr->dsp_ver_minor =
						lp_ptr->dsp_ptr->get_ver_minor(
							lp_ptr->dsp_ptr);

					if (NULL != lp_ptr->dsp_ptr->cpc_ptr) {
						/* Have CPC */
						data_ptr->have_cpc    = 1;
						data_ptr->cpc_version =
							lp_ptr->dsp_ptr->cpc_ptr->header.version;
						data_ptr->cpc_signature =
							lp_ptr->dsp_ptr->cpc_ptr->header.cpc.signature;
						data_ptr->cpc_size =
							lp_ptr->dsp_ptr->cpc_ptr->size;
						data_ptr->cpc_primitive_count =
							lp_ptr->dsp_ptr->cpc_ptr->number_of_basic_primitives;
					}	/* cpc */
				}	/* dsp */
			}	/* lp */
		}	/* block */
		command_ptr->return_code = ESIF_OK;
	}	/* if */
}


/* Dispatch */
struct esif_ipc *esif_execute_ipc_command(
	struct esif_ipc *ipc_ptr
	)
{
	struct esif_ipc_command *command_ptr =
		(struct esif_ipc_command *)(ipc_ptr + 1);
	command_ptr->return_code = ESIF_E_COMMAND_DATA_INVALID;

	if (ESIF_COMMAND_VERSION != command_ptr->version)
		return ipc_ptr;

	ESIF_TRACE_DYN_COMMAND("%s: COMMAND Received: ipc %p\n",
			       ESIF_FUNC,
			       ipc_ptr);
	ESIF_TRACE_DYN_DECODE("  Version:              %d\n"
			      "  Type:                 %s(%d)\n"
			      "  Priority:             %s(%d)\n"
			      "  Payload Length:       %d\n"
			      "  Request  Data Type:   %s(%d)\n"
			      "  Request  Data Offset: %d\n"
			      "  Request  Data Length: %d\n"
			      "  Response Data Type:   %s(%d)\n"
			      "  Response Data Offset: %d\n"
			      "  Response Data Length: %d\n",
			      (int)command_ptr->version,
			      esif_command_type_str(command_ptr->type),
			      (int)command_ptr->type,
			      esif_command_priority_str(command_ptr->priority),
			      (int)command_ptr->priority,
			      (int)command_ptr->payload_len,
			      esif_data_type_str(command_ptr->req_data_type),
			      (int)command_ptr->req_data_type,
			      (int)command_ptr->req_data_offset,
			      (int)command_ptr->req_data_len,
			      esif_data_type_str(command_ptr->rsp_data_type),
			      (int)command_ptr->rsp_data_type,
			      (int)command_ptr->rsp_data_offset,
			      (int)command_ptr->rsp_data_len);

	switch (command_ptr->type) {
	case ESIF_COMMAND_TYPE_SET_DEBUG_MODULES:
		esif_execute_ipc_command_set_debug_modules(command_ptr);
		break;

	case ESIF_COMMAND_TYPE_SET_DEBUG_MODULE_LEVEL:
		esif_execute_ipc_command_set_debug_module_level(command_ptr);
		break;

	case ESIF_COMMAND_TYPE_GET_DEBUG_MODULE_LEVEL:
		esif_execute_ipc_command_get_debug_module_level(command_ptr);
		break;

	case ESIF_COMMAND_TYPE_GET_KERNEL_INFO:
		esif_execute_ipc_command_get_kernel_info(command_ptr);
		break;

	case ESIF_COMMAND_TYPE_GET_MEMORY_STATS:
		esif_execute_ipc_command_get_memory_stats(command_ptr);
		break;

	case ESIF_COMMAND_TYPE_GET_PARTICIPANTS:
		esif_execute_ipc_command_get_participants(command_ptr);
		break;

	case ESIF_COMMAND_TYPE_GET_PARTICIPANT_DETAIL:
		esif_execute_ipc_command_get_participant_detail(command_ptr);
		break;

	default:
		ESIF_TRACE_DYN_COMMAND("%s: Unknown Command Type %d:\n",
				       ESIF_FUNC, command_ptr->type);
		break;
	}
	ESIF_TRACE_DYN_COMMAND("%s: COMMAND return result: %s(%d)\n", ESIF_FUNC,
			       esif_rc_str(
				       command_ptr->return_code),
			       command_ptr->return_code);

	/* Send To User */
	return ipc_ptr;
}


/* Init */
enum esif_rc esif_command_init(void)
{
	ESIF_TRACE_DYN_INIT("%s: Initialize Command\n", ESIF_FUNC);
	return ESIF_OK;
}


/* Exit */
void esif_command_exit(void)
{
	ESIF_TRACE_DYN_INIT("%s: Exit Command\n", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
