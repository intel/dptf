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

#include "esif_uf.h"	/* Upper Framework */
#include "esif_dsp.h"	/* Device Support Package */
#include "esif_pm.h"	/* Upper Participant Manager */
#include "esif_ipc.h"	/* IPC Abstraction */
#include "esif_uf_appmgr.h"
#include "esif_uf_ipc.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

extern int g_debug;
extern FILE *g_debuglog;

// TODO move these
void ipc_autoconnect ();
void ipc_disconnect ();
void esif_dispatch_event ();
int esif_send_dsp (char *filename, u8 dst_id);

extern int g_quit;
extern int g_quit2;
extern int g_autocpc;
extern esif_handle_t g_ipc_handle;

#define PATH_STR_LEN 128

static char*esif_primitive_domain_str (
	u16 domain,
	char *str,
	u8 str_len
	)
{
	u8 *ptr = (u8*)&domain;
	esif_ccb_sprintf(str_len, str, "%c%c", *(ptr + 1), *ptr);
	return str;
}


// Dispatch An Event
void esif_dispatch_event ()
{
	int r_bytes     = 0;
	int data_len    = 512;
	enum esif_rc rc = ESIF_OK;


	struct esif_ipc *ipc_ptr = esif_ipc_alloc(ESIF_IPC_TYPE_EVENT, data_len);
	if (NULL == ipc_ptr) {
		return;
	}

//
// TODO:  This needs to be in an OS abstraction layer
//
#ifdef ESIF_ATTR_OS_LINUX
	r_bytes = read(g_ipc_handle, ipc_ptr, data_len);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS

	rc = ipc_execute(ipc_ptr);
	//
	// TODO:  The return size check is a W/A until a better solution is developed to determine if an event was returned.
	// (The current LF code does not change the original data_len or fail the request if there are not events.)
	//
	r_bytes = 0;
	if ((ESIF_OK == rc) && (ipc_ptr->data_len != (u32)data_len)) {
		r_bytes = ipc_ptr->data_len;
	}
#endif

	// Have Event?
	if (r_bytes > 0) {
		struct esif_ipc_event_header *event_hdr_ptr = NULL;
		ESIF_TRACE_DEBUG("%s_%s: ipc version=%d, type=%d, len = %d, data_len=%d\n",
						 ESIF_ATTR_OS, ESIF_FUNC,
						 ipc_ptr->version,
						 ipc_ptr->type,
						 r_bytes,
						 ipc_ptr->data_len);

		event_hdr_ptr = (struct esif_ipc_event_header*)(ipc_ptr + 1);
		EsifEventProcess(event_hdr_ptr);
	}
	esif_ipc_free(ipc_ptr);
}


void EsifEventProcess (struct esif_ipc_event_header *eventHdrPtr)
{
	char domain_str[8] = "";
	esif_ccb_time_t now;
	char guid_str[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guid_str);
	UNREFERENCED_PARAMETER(domain_str);

	esif_ccb_system_time(&now);

	ESIF_TRACE_DEBUG(
		"\n"
		"===================================================\n"
		"ESIF IPC EVENT HEADER: Timestamp: %llu\n"
		"                         Latency: %u msec\n"
		"===================================================\n"
		"Version:     %d\n"
		"Type:        %s(%d)\n"
		"ID:          %llu\n"
		"Timestamp:   %llu\n"
		"Priority:    %s(%d)\n"
		"Source:      %d\n"
		"Dest:        %d\n"
		"Dest Domain: %s(%04X)\n"
		"Data Size:   %u\n\n",
		(u64)now,
		(int)(now - eventHdrPtr->timestamp),
		eventHdrPtr->version,
		esif_event_type_str(eventHdrPtr->type),
		eventHdrPtr->type,
		eventHdrPtr->id,
		eventHdrPtr->timestamp,
		esif_event_priority_str(eventHdrPtr->priority),
		eventHdrPtr->priority,
		eventHdrPtr->src_id,
		eventHdrPtr->dst_id,
		esif_primitive_domain_str(eventHdrPtr->dst_domain_id, domain_str, 8),
		eventHdrPtr->dst_domain_id,
		eventHdrPtr->data_len);

	if (ESIF_EVENT_PARTICIPANT_CREATE == eventHdrPtr->type) {
		struct esif_ipc_event_data_create_participant *data_ptr = NULL;
		EsifString edp_filename_ptr = NULL;
		EsifString dsp_lookup_result_ptr = NULL;

		// Event Data
		data_ptr = (struct esif_ipc_event_data_create_participant*)(eventHdrPtr + 1);

		// Sharkbay
		if (!strcmp(data_ptr->name, "TFN1")) {
			edp_filename_ptr = (char*)"sb_tfan";
		} else if (!strcmp(data_ptr->name, "TFN2")) {
			edp_filename_ptr = (char*)"sb_tfan";
		} else if (!strcmp(data_ptr->name, "TMEM")) {
			edp_filename_ptr = (char*)"sb_tmem";
		} else if (!strcmp(data_ptr->name, "TAMB")) {
			edp_filename_ptr = (char*)"sb_temp";
		} else if (!strcmp(data_ptr->name, "TEFN")) {
			edp_filename_ptr = (char*)"sb_temp";
		} else if (!strcmp(data_ptr->name, "TSKN")) {
			edp_filename_ptr = (char*)"sb_temp";
		} else if (!strcmp(data_ptr->name, "T_VR")) {
			edp_filename_ptr = (char*)"sb_temp";
		} else if (!strcmp(data_ptr->name, "FGEN")) {
			edp_filename_ptr = (char*)"sb_fgen";
		} else if (!strcmp(data_ptr->name, "DPLY")) {
			edp_filename_ptr = (char*)"sb_dply";
		} else if (!strcmp(data_ptr->name, "TPWR")) {
			edp_filename_ptr = (char*)"sb_tpwr";
		} else if (!strcmp(data_ptr->name, "WIFI")) {
			edp_filename_ptr = (char*)"sb_wifi";
		} else if (!strcmp(data_ptr->name, "WGIG")) {
			edp_filename_ptr = (char*)"sb_wgig";
		} else if (!strcmp(data_ptr->name, "WWAN")) {
			edp_filename_ptr = (char*)"sb_wwan";
		} else if (!strcmp(data_ptr->name, "TINL")) {
			edp_filename_ptr = (char*)"sb_temp";
		} else if (!strcmp(data_ptr->name, "TPCH")) {
			edp_filename_ptr = (char*)"sb_b0_d1f_f6";
		} else if (!strcmp(data_ptr->name, "TCPU")) {
			edp_filename_ptr = (char*)"sb_b0_d4_f0";
		} else if (!strcmp(data_ptr->name, "IETM")) {
			edp_filename_ptr = (char*)"sb_ietm";
		} else if (!strcmp(data_ptr->name, "DPTFZ")) {
			edp_filename_ptr = (char*)"sb_ietm";
		} else {
			edp_filename_ptr = (char*)"sb_fgen";
		}

		// PCI
		if (1 == data_ptr->enumerator) {
			ESIF_TRACE_DEBUG(
				"==================================================\n"
				"ESIF IPC EVENT DATA CREATE PCI PARTICIPANT:\n"
				"==================================================\n"
				"Instance:       %d\n"
				"Version:        %d\n"
				"Class:          %s\n"
				"Enumerator:     %s(%u)\n"
				"Flags:          0x%08x\n"
				"Name:           %s\n"
				"Description:    %s\n"
				"Driver Name:    %s\n"
				"Device Name:    %s\n"
				"Device Path:    %s\n"
				"ACPI Scope:     %s\n"
				"PCI Vendor:     0x%04X %s\n"
				"PCI Device:     0x%04X %s\n"
				"PCI Bus:        0x%02X\n"
				"PCI Bus Device: 0x%02X\n"
				"PCI Function:   0x%02X\n"
				"PCI Revision:   0x%02X\n"
				"PCI Class:      0x%02X %s\n"
				"PCI SubClass:   0x%02X\n"
				"PCI ProgIF:     0x%02X\n\n",
				data_ptr->id,
				data_ptr->version,
				esif_guid_print((esif_guid_t*)data_ptr->class_guid, guid_str),
				esif_participant_enum_str(data_ptr->enumerator),
				data_ptr->enumerator,
				data_ptr->flags,
				data_ptr->name,
				data_ptr->desc,
				data_ptr->driver_name,
				data_ptr->device_name,
				data_ptr->device_path,
				data_ptr->acpi_scope,
				data_ptr->pci_vendor,
				esif_vendor_str(data_ptr->pci_vendor),
				data_ptr->pci_device,
				esif_device_str(data_ptr->pci_device),
				data_ptr->pci_bus,
				data_ptr->pci_bus_device,
				data_ptr->pci_function,
				data_ptr->pci_revision,
				data_ptr->pci_class,
				esif_pci_class_str(data_ptr->pci_class),
				data_ptr->pci_sub_class,
				data_ptr->pci_prog_if);
			// ACPI
		} else if (0 == data_ptr->enumerator) {
			ESIF_TRACE_DEBUG(
				"==================================================\n"
				"ESIF IPC EVENT DATA CREATE ACPI PARTICIPANT:\n"
				"==================================================\n"
				"Instance:       %d\n"
				"Version:        %d\n"
				"Class:          %s\n"
				"Enumerator:     %s(%u)\n"
				"Flags:          0x%08x\n"
				"Name:           %s\n"
				"Description:    %s\n"
				"Driver Name:    %s\n"
				"Device Name:    %s\n"
				"Device Path:    %s\n"
				"ACPI Device:    %s %s\n"
				"ACPI Scope:     %s\n",
				data_ptr->id,
				data_ptr->version,
				esif_guid_print((esif_guid_t*)&data_ptr->class_guid, guid_str),
				esif_participant_enum_str(data_ptr->enumerator),
				data_ptr->enumerator,
				data_ptr->flags,
				data_ptr->name,
				data_ptr->desc,
				data_ptr->driver_name,
				data_ptr->device_name,
				data_ptr->device_path,
				data_ptr->acpi_device,
				esif_acpi_device_str(data_ptr->acpi_device),
				data_ptr->acpi_scope);

			if (data_ptr->acpi_uid != 0xFFFFFFFF) {
				ESIF_TRACE_DEBUG("ACPI UID:       0x%x\n",
								 data_ptr->acpi_uid);
			}

			if (data_ptr->acpi_type != 0xFFFFFFFF) {
				ESIF_TRACE_DEBUG("ACPI Type:      0x%x %s\n",
								 data_ptr->acpi_type,
								 esif_domain_type_str((enum esif_domain_type)data_ptr->acpi_type));
			}

			{
				struct esif_uf_dm_query_acpi qry = {0};
					#define TEMP_LEN 12
				char temp[TEMP_LEN]    = {0};

				EsifString acpi_device = "*";
				EsifString acpi_type   = "*";
				EsifString acpi_uid    = "*";
				EsifString acpi_scope  = "*";

				int type = data_ptr->acpi_type;
				int uid  = data_ptr->acpi_uid;

				if (esif_ccb_strlen(data_ptr->acpi_device, ESIF_NAME_LEN) > 0) {
					acpi_device = data_ptr->acpi_device;
				}
				if (esif_ccb_strlen(data_ptr->acpi_scope, ESIF_SCOPE_LEN) > 0) {
					acpi_scope = data_ptr->acpi_scope;
				}

				if (type >= 0) {
					esif_ccb_sprintf(TEMP_LEN, temp, "%d", type);
					acpi_type = temp;
				}

				if (uid >= 0) {
					esif_ccb_sprintf(TEMP_LEN, temp, "%d", uid);
					acpi_uid = temp;
				}

				qry.acpi_device = acpi_device;
				qry.acpi_type   = acpi_type;
				qry.acpi_uid    = acpi_uid;
				qry.acpi_scope  = acpi_scope;

				ESIF_TRACE_DEBUG(
					"\n\nQuery WHERE Clause:\n\n"
					"acpi_device: %s\n"
					"acpi_type:   %s\n"
					"acpi_uid:    %s\n"
					"acpi_scope:  %s\n",
					acpi_device,
					acpi_type,
					acpi_uid,
					acpi_scope);

				dsp_lookup_result_ptr = esif_uf_dm_query(ESIF_UF_DM_QUERY_TYPE_ACPI, &qry);

				ESIF_TRACE_DEBUG("DSP Lookup: %s\n", dsp_lookup_result_ptr);

				if (NULL == dsp_lookup_result_ptr) {
					dsp_lookup_result_ptr = edp_filename_ptr;
				}

				ESIF_TRACE_DEBUG("\nDSP Lookup: %s\n", dsp_lookup_result_ptr);

				/* We are all in for ACPI so override our selection */
				edp_filename_ptr = dsp_lookup_result_ptr;
			}
		} else {
			ESIF_TRACE_DEBUG(
				"=======================================================\n"
				"ESIF IPC EVENT DATA CREATE PLATFORM PARTICIPANT:\n"
				"=======================================================\n"
				"Instance:       %d\n"
				"Version:        %d\n"
				"Class:          %s\n"
				"Enumerator:     %s(%u)\n"
				"Flags:          0x%08x\n"
				"Name:           %s\n"
				"Description:    %s\n"
				"Driver Name:    %s\n"
				"Device Name:    %s\n"
				"Device Path:    %s\n\n",
				data_ptr->id,
				data_ptr->version,
				esif_guid_print((esif_guid_t*)data_ptr->class_guid, guid_str),
				esif_participant_enum_str(data_ptr->enumerator),
				data_ptr->enumerator,
				data_ptr->flags,
				data_ptr->name,
				data_ptr->desc,
				data_ptr->driver_name,
				data_ptr->device_name,
				data_ptr->device_path);
		}

		if(EsifUpManagerDoesAvailableParticipantExistByName(data_ptr->name)) {
			return;
		}

		if (g_autocpc && NULL != edp_filename_ptr) {
			char edp_full_path[PATH_STR_LEN];
			esif_ccb_sprintf(PATH_STR_LEN, edp_full_path, "%s.edp",
							 esif_build_path(edp_full_path, PATH_STR_LEN, ESIF_DIR_DSP, edp_filename_ptr));

			ESIF_TRACE_DEBUG("AutoCPC Selected DSP Delivery Method EDP(CPC): %s\n", edp_full_path);
			if (0 == esif_send_dsp(edp_full_path, eventHdrPtr->src_id)) {
				EsifUpPtr up_ptr = NULL;
				ESIF_TRACE_DEBUG("\nCreate Upper Participant: %s\n", data_ptr->name);

				up_ptr = EsifUpManagerCreateParticipant(eParticipantOriginLF, &eventHdrPtr->src_id, data_ptr);
				if (up_ptr) {
					/* Associate a DSP with the participant */
					up_ptr->fDspPtr = esif_uf_dm_select_dsp_by_code(edp_filename_ptr);
					if (up_ptr->fDspPtr) {
						ESIF_TRACE_DEBUG("Hit DSP Lookup %s\n", up_ptr->fDspPtr->code_ptr);

						/* Now offer this participant to each running application */
						EsifAppMgrCreateCreateParticipantInAllApps(up_ptr);

					} else {
						ESIF_TRACE_DEBUG("Missed DSP Lookup %s\n", edp_filename_ptr);
						EsifUpManagerUnregisterParticipant(eParticipantOriginLF, up_ptr);
						up_ptr = NULL;
					}
				}
			}
		} else {
			ESIF_TRACE_DEBUG("Please manually load a CPC for this Participant\n");
			ESIF_TRACE_DEBUG("Example: dst %d then loadcpc\n\n", eventHdrPtr->src_id);
		}
	} else if (ESIF_EVENT_PARTICIPANT_UNREGISTER == eventHdrPtr->type) {
		ESIF_TRACE_DEBUG("Destroy Upper Participant: %d\n", eventHdrPtr->src_id);
		EsifUpManagerUnregisterParticipant(eParticipantOriginLF, &eventHdrPtr->src_id);
	} else {
		// Best Effor Delivery.
		EsifAppsEvent(eventHdrPtr->dst_id, eventHdrPtr->dst_domain_id, eventHdrPtr->type, NULL);
	}
}


// Event Worker Thread
// Processes ESIF Events
void*esif_event_worker_thread (void *ptr)
{
	int rc = 0;
	fd_set rfds = {0};
	struct timeval tv = {0};

	UNREFERENCED_PARAMETER(ptr);
	CMD_OUT("Start ESIF Upper Framework Shell\n");

	// Connect To Kernel IPC
	ipc_autoconnect();

	// Run Until Told To Quit
	while (!g_quit) {
		FD_ZERO(&rfds);
#ifdef ESIF_ATTR_OS_WINDOWS
		FD_SET((SOCKET)g_ipc_handle, &rfds);
#else
		FD_SET((u32)g_ipc_handle, &rfds);
#endif

		tv.tv_sec  = 0;
		tv.tv_usec = 50000;	/* 50 msec */

#ifdef ESIF_ATTR_OS_LINUX
		rc = select(g_ipc_handle + 1, &rfds, NULL, NULL, &tv);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
		// Windows does not support select/poll so we simulate here
		Sleep(50);
		rc = 1;
#endif
		if (rc > 0) {
			esif_dispatch_event();
		}
	}

	if (g_ipc_handle != ESIF_INVALID_HANDLE) {
		ipc_disconnect();
	}
	g_quit2 = 1;
	return 0;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
