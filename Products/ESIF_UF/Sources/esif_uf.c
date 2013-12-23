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

/* ESIF */
#include "esif_uf.h"		/* ESIF Upper Framework */

/* Managers */
#include "esif_pm.h"		/* Participant Manager */
#include "esif_uf_actmgr.h"	/* Action Manager */
#include "esif_uf_appmgr.h"	/* Application Manager */
#include "esif_uf_cfgmgr.h"	/* Config Manager */
#include "esif_uf_cnjmgr.h"	/* Conjure Manager */

/* Init */
#include "esif_dsp.h"		/* Device Support Package */

/* IPC */
#include "esif_command.h"	/* Command Interface */
#include "esif_event.h"		/* Events */
#include "esif_ipc.h"		/* IPC */
#include "esif_uf_ipc.h"	/* IPC */
#include "esif_uf_rest.h"	/* REST API */
#include "esif_ws_server.h"	/* Web Server */

/* Xform */
// #include "esif_temp.h"
// #include "esif_power.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

FILE *g_debuglog = NULL;

int g_debug      = ESIF_TRUE;	// Debug?
int g_autocpc    = ESIF_TRUE;	// Automatically Assign DSP/CPC
int g_errorlevel = 0;			// Exit Errorlevel
int g_quit       = ESIF_FALSE;	// Quit
int g_quit2      = ESIF_FALSE;	// Quit 2

#ifdef ESIF_ATTR_MEMTRACE
struct memtrace_s g_memtrace;
#endif

/* Work Around */
static enum esif_rc get_participant_data (
	struct esif_ipc_event_data_create_participant *pi_ptr,
	UInt8 participantId
	)
{
	eEsifError rc = ESIF_OK;

	struct esif_command_get_participant_detail *data_ptr = NULL;
	struct esif_ipc_command *command_ptr = NULL;
	struct esif_ipc *ipc_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_participant_detail);

	ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_PARTICIPANT_DETAIL;
	command_ptr->req_data_type   = ESIF_DATA_UINT32;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 4;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	// ID For Command
	*(u32*)(command_ptr + 1) = participantId;
	rc = ipc_execute(ipc_ptr);

	if (ESIF_OK != rc) {
		goto exit;
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		rc = ipc_ptr->return_code;
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		rc = command_ptr->return_code;
		goto exit;
	}

	// our data
	data_ptr = (struct esif_command_get_participant_detail*)(command_ptr + 1);
	if (0 == data_ptr->version) {
		goto exit;
	}

	pi_ptr->id = (u8)data_ptr->id;
	pi_ptr->version = data_ptr->version;
	esif_ccb_memcpy(pi_ptr->class_guid, data_ptr->class_guid, ESIF_GUID_LEN);

	pi_ptr->enumerator = data_ptr->enumerator;
	pi_ptr->flags = data_ptr->flags;

	esif_ccb_strcpy(pi_ptr->name, data_ptr->name, ESIF_NAME_LEN);
	esif_ccb_strcpy(pi_ptr->desc, data_ptr->desc, ESIF_DESC_LEN);
	esif_ccb_strcpy(pi_ptr->driver_name, data_ptr->driver_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(pi_ptr->device_name, data_ptr->device_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(pi_ptr->device_path, data_ptr->device_path, ESIF_NAME_LEN);

	/* ACPI */
	esif_ccb_strcpy(pi_ptr->acpi_device, data_ptr->acpi_device, ESIF_NAME_LEN);
	esif_ccb_strcpy(pi_ptr->acpi_scope, data_ptr->acpi_scope, ESIF_SCOPE_LEN);
	pi_ptr->acpi_uid  = data_ptr->acpi_uid;
	pi_ptr->acpi_type = data_ptr->acpi_type;

	/* PCI */
	pi_ptr->pci_vendor     = data_ptr->pci_vendor;
	pi_ptr->pci_device     = data_ptr->pci_device;
	pi_ptr->pci_bus        = data_ptr->pci_bus;
	pi_ptr->pci_bus_device = data_ptr->pci_bus_device;
	pi_ptr->pci_function   = data_ptr->pci_function;
	pi_ptr->pci_revision   = data_ptr->pci_revision;
	pi_ptr->pci_class      = data_ptr->pci_class;
	pi_ptr->pci_sub_class  = data_ptr->pci_sub_class;
	pi_ptr->pci_prog_if    = data_ptr->pci_prog_if;

exit:

	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return rc;
}


/* Will sync any existing lower framework participatnts */
static enum esif_rc sync_lf_participants ()
{
	eEsifError rc = ESIF_OK;
	struct esif_command_get_participants *data_ptr = NULL;
	const UInt32 data_len = sizeof(struct esif_command_get_participants);
	struct esif_ipc_command *command_ptr = NULL;
	UInt8 i = 0;
	UInt32 count = 0;

	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s: SYNC", ESIF_FUNC);

	command_ptr->type = ESIF_COMMAND_TYPE_GET_PARTICIPANTS;
	command_ptr->req_data_type   = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 0;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	rc = ipc_execute(ipc_ptr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		rc = ipc_ptr->return_code;
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		rc = ipc_ptr->return_code;
		goto exit;
	}

	/* Participant Data */

	data_ptr = (struct esif_command_get_participants*)(command_ptr + 1);
	count    = data_ptr->count;

	for (i = 0; i < count; i++) {
		struct esif_ipc_event_header *event_hdr_ptr = NULL;
		struct esif_ipc_event_data_create_participant *event_cp_ptr = NULL;

		esif_ccb_time_t now;
		int size = 0;
		esif_ccb_system_time(&now);

		if (data_ptr->participant_info[i].state <= ESIF_PM_PARTICIPANT_REMOVED) {
			continue;
		}

		// printf("Add Existing LF Participant: %s\n", data_ptr->participant_info[i].name);

		size = sizeof(struct esif_ipc_event_header) +
			sizeof(struct esif_ipc_event_data_create_participant);
		event_hdr_ptr = (struct esif_ipc_event_header*)esif_ccb_malloc(size);
		if (event_hdr_ptr == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		/* Fillin Event Header */
		event_hdr_ptr->data_len = sizeof(struct esif_ipc_event_data_create_participant);
		event_hdr_ptr->dst_id   = ESIF_INSTANCE_UF;
		event_hdr_ptr->dst_domain_id = 'NA';
		event_hdr_ptr->id        = 0;
		event_hdr_ptr->priority  = ESIF_EVENT_PRIORITY_NORMAL;
		event_hdr_ptr->src_id    = i;
		event_hdr_ptr->timestamp = now;
		event_hdr_ptr->type      = ESIF_EVENT_PARTICIPANT_CREATE;
		event_hdr_ptr->version   = ESIF_EVENT_VERSION;

		event_cp_ptr = (struct esif_ipc_event_data_create_participant*)(event_hdr_ptr + 1);
		get_participant_data(event_cp_ptr, i);

		EsifEventProcess(event_hdr_ptr);

		esif_ccb_free(event_hdr_ptr);
	}
	printf("\n");

exit:
	ESIF_TRACE_DEBUG("%s: rc = %s(%u)", ESIF_FUNC, esif_rc_str(rc), rc);

	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return rc;
}


void done (const void *context)
{
	printf("Context = %p %s\n", context, (char*)context);
}


/* Build An Absoulte Path */
esif_string g_home = NULL;
esif_string esif_build_path (
	esif_string buffer,
	u32 buf_len,
	esif_string dir,
	esif_string filename
	)
{
	if (NULL == buffer) {
		goto exit;
	}

	if (NULL != dir && NULL != filename) {
		esif_ccb_sprintf(buf_len, buffer, "%s%s%s%s", g_home, dir, ESIF_PATH_SEP, filename);
	} else if (NULL == dir && NULL != filename) {
		esif_ccb_sprintf(buf_len, buffer, "%s%s", g_home, filename);
	} else if (NULL != dir && NULL == filename) {
		esif_ccb_sprintf(buf_len, buffer, "%s%s", g_home, dir);
	} else if (NULL == dir && NULL == filename) {
		esif_ccb_sprintf(buf_len, buffer, "%s", g_home);
	}
exit:
	return buffer;
}


static void*esif_web_worker_thread (void *ptr)
{
	UNREFERENCED_PARAMETER(ptr);
	esif_ws_init(g_home);

	return 0;
}


static esif_thread_t g_webthread;
static eEsifError EsifWebStart ()
{
	esif_ccb_thread_create(&g_webthread, esif_web_worker_thread, NULL);
	return ESIF_OK;
}


static void EsifWebStop ()
{
	// TODO shutdown webserver cleanly.
}


eEsifError esif_uf_init (esif_string home_dir)
{
	eEsifError rc = ESIF_OK;
#ifdef ESIF_ATTR_MEMTRACE
	u32 *memory_leak = 0;
	esif_memtrace_init();
	UNREFERENCED_PARAMETER(memory_leak);
	//memory_leak = (u32*)esif_ccb_malloc(sizeof(*memory_leak)); /* intentional memory leak for debugging */
#endif

	ESIF_TRACE_DEBUG("%s: Init Upper Framework (UF)", ESIF_FUNC);
	if (NULL != home_dir) {
		printf("%s: Home: %s\n", ESIF_FUNC, home_dir);
		g_home = esif_ccb_strdup(home_dir);
	} else {
		goto exit;
	}

	/* OS Agnostic */
	EsifCfgMgrInit();
	EsifCnjMgrInit();
	EsifUppMgrInit();
	EsifDspMgrInit();
	EsifAppMgrInit();
	EsifActMgrInit();

#ifdef  ESIF_ATTR_WEBSOCKET
	EsifWebStart();
#endif

	/* OS Specific */
	rc = esif_uf_os_init();
	if (ESIF_OK != rc) {
		goto exit;
	}

	ipc_connect();
	sync_lf_participants();

exit:
	return rc;
}


void esif_uf_exit ()
{
#ifdef  ESIF_ATTR_WEBSOCKET
	EsifWebStop();
#endif

	/* Stop Rest API */
	EsifRestStop();

	/* OS Specific */
	esif_uf_os_exit();

	/* OS Agnostic */
	EsifAppMgrExit();
	EsifActMgrExit();
	EsifDspMgrExit();
	EsifUppMgrExit();
	EsifCnjMgrExit();
	EsifCfgMgrExit();

	if (g_debuglog) {
		esif_ccb_fclose(g_debuglog);
		g_debuglog = 0;
	}
	esif_ccb_free(g_home);

#ifdef ESIF_ATTR_MEMTRACE
	ESIF_TRACE_DEBUG("UserMem: Allocs=%lu Frees=%lu\n", (unsigned long)atomic_read(&g_memtrace.allocs),
					 (unsigned long)atomic_read(&g_memtrace.frees));
	esif_memtrace_exit();
#endif
	ESIF_TRACE_DEBUG("%s: Exit Upper Framework (UF)", ESIF_FUNC);
}

/* Memory Allocation Trace Support */
#ifdef ESIF_ATTR_MEMTRACE

// TODO: Locking

void *esif_memtrace_alloc (
	void *old_ptr, 
	size_t size, 
	const char *func, 
	const char *file, 
	int line)
{
	struct memalloc_s *mem = g_memtrace.allocated;
	void *mem_ptr = 0;

	if (file) {
		const char *slash = strrchr(file, *ESIF_PATH_SEP);
		if (slash)
			file = slash+1;
	}

	if (old_ptr) {
		mem_ptr = realloc(old_ptr, size); /* native */
		if (!mem_ptr)
			goto exit;
		while (mem) {
			if (old_ptr == mem->mem_ptr) {
				mem->mem_ptr = mem_ptr;
				mem->size = size;
				mem->func = func;
				mem->file = file;
				mem->line = line;
				goto exit;
			}
			mem = mem->next;
		}
	}
	else {
		mem_ptr = malloc(size); /* native */
		if (mem_ptr) {
			esif_ccb_memset(mem_ptr, 0, size);
			atomic_inc(&g_memtrace.allocs);
		}
	}

	mem = (struct memalloc_s*)malloc(sizeof(*mem)); /* native */
	if (!mem)
		goto exit;
	esif_ccb_memset(mem, 0, sizeof(*mem));
	mem->mem_ptr = mem_ptr;
	mem->size = size;
	mem->func = func;
	mem->file = file;
	mem->line = line;
	mem->next = g_memtrace.allocated;
	g_memtrace.allocated = mem;
exit:
	return mem_ptr;
}

char* esif_memtrace_strdup (
	char *str, 
	const char *func, 
	const char *file, 
	int line)
{
	size_t len = esif_ccb_strlen(str, 0x7fffffff)+1;
	char *mem_ptr = (char*)esif_memtrace_alloc(0, len, func, file, line);
	if (NULL != mem_ptr) {
		esif_ccb_strcpy(mem_ptr, str, len);
	}
	return mem_ptr;
}

void esif_memtrace_free (void *mem_ptr)
{
	struct memalloc_s *mem = g_memtrace.allocated;
	struct memalloc_s **last = &g_memtrace.allocated;
	while (mem) {
		if (mem_ptr == mem->mem_ptr) {
			*last = mem->next;
			free(mem); /* native */
			goto exit;
		}
		last = &mem->next;
		mem = mem->next;
	}
exit:
	if (mem_ptr) {
		free(mem_ptr); /* native */
		atomic_inc(&g_memtrace.frees);
	}
	return;
}

void esif_memtrace_init ()
{
	struct memalloc_s *mem = g_memtrace.allocated;

	// Ignore any allocations made before this function was called
	while (mem) {
		struct memalloc_s *node = mem;
		mem = mem->next;
		free(node); /* native */

	}
	g_memtrace.allocated = NULL;
	atomic_ctor(&g_memtrace.allocs);
	atomic_ctor(&g_memtrace.frees);
}

#include "esif_lib_databank.h" // ESIFDV_DIR reference

void esif_memtrace_exit ()
{
	struct memalloc_s *mem = g_memtrace.allocated;
	char tracefile[MAX_PATH];
	FILE *tracelog=NULL;

	CMD_OUT("UserMem: Allocs=%lu Frees=%lu\n", (unsigned long)atomic_read(&g_memtrace.allocs), (unsigned long)atomic_read(&g_memtrace.frees));
	if (!mem)
		goto exit;

	esif_ccb_sprintf(MAX_PATH, tracefile, "%s%s", ESIFDV_DIR, "memtrace.txt");
	CMD_OUT("\n*** MEMORY LEAKS DETECTED ***\nFor details see %s\n", tracefile);
	esif_ccb_fopen(&tracelog, tracefile, "w");
	while (mem) {
		struct memalloc_s *node=mem;
		size_t i; (i);
		if (tracelog)
			fprintf(tracelog, "[%s @%s:%d]: (%lld bytes) %p\n", mem->func, mem->file, mem->line, (long long)mem->size, mem->mem_ptr);
		mem = mem->next;
		free(node->mem_ptr); /* native */
		free(node); /* native */
	}
	g_memtrace.allocated = NULL;
	if (tracelog)
		esif_ccb_fclose(tracelog);
exit:
	atomic_dtor(&g_memtrace.allocs);
	atomic_dtor(&g_memtrace.frees);
}

/************/

#endif /* ESIF_ATTR_MEMTRACE */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

