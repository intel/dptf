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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_UF

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
#include "esif_ws_server.h"	/* Web Server */

#include "esif_lib_databank.h"

/* Native memory allocation functions for use by memtrace functions only */
#define native_malloc(siz)          malloc(siz)
#define native_realloc(ptr, siz)    realloc(ptr, siz)
#define native_free(ptr)            free(ptr)

#ifdef ESIF_ATTR_OS_WINDOWS
#include <share.h>
#include <shlobj.h>
#pragma comment (lib, "shell32")
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

int g_autocpc    = ESIF_TRUE;	// Automatically Assign DSP/CPC
int g_errorlevel = 0;			// Exit Errorlevel
int g_quit		 = ESIF_FALSE;	// Quit
int g_disconnectClient = ESIF_FALSE;// Disconnect client
int g_quit2      = ESIF_FALSE;	// Quit 2

static esif_thread_t g_webthread;  //web worker thread

// Global Shell lock to limit parse_cmd to one thread at a time
#ifdef ESIF_ATTR_SHELL_LOCK
esif_ccb_mutex_t g_shellLock;
#endif

#ifdef ESIF_ATTR_MEMTRACE
struct memtrace_s g_memtrace;
#endif

// ESIF Log File object
typedef struct EsifLogFile_s {
	esif_ccb_lock_t lock;		// Thread Lock
	esif_string		name;		// Log Name
	esif_string		filename;	// Log file name
	FILE			*handle;	// Log file handle or NULL if not open
} EsifLogFile;

static EsifLogFile g_EsifLogFile[MAX_ESIFLOG] = {0};

static esif_string g_home = NULL; // Global Home directory

// Set Pathname components based on OS & Architecture
#ifdef ESIF_ATTR_OS_WINDOWS
  #ifdef ESIF_ATTR_64BIT
    #define ESIF_DIR_EXE	"ufx64"
  #else 
    #define ESIF_DIR_EXE	"ufx86"
  #endif
#else
	#define ESIF_DIR_EXE	NULL
#endif
#define ESIF_DIR_BIN	"bin"
#define ESIF_DIR_CMD	"cmd"
#define ESIF_DIR_DSP	"dsp"
#define ESIF_DIR_LOG	"log"

eEsifError EsifLogMgrInit(void)
{
	int j;
	esif_ccb_memset(g_EsifLogFile, 0, sizeof(g_EsifLogFile));
	for (j=0; j < MAX_ESIFLOG; j++) {
		esif_ccb_lock_init(&g_EsifLogFile[j].lock);
	}
	g_EsifLogFile[ESIF_LOG_EVENTLOG].name = esif_ccb_strdup("event");
	g_EsifLogFile[ESIF_LOG_DEBUGGER].name = esif_ccb_strdup("debug");
	g_EsifLogFile[ESIF_LOG_SHELL].name    = esif_ccb_strdup("shell");
	g_EsifLogFile[ESIF_LOG_TRACE].name    = esif_ccb_strdup("trace");
	g_EsifLogFile[ESIF_LOG_UI].name       = esif_ccb_strdup("ui");
	return ESIF_OK;
}

void EsifLogMgrExit(void)
{
	int j;
	for (j=0; j < MAX_ESIFLOG; j++) {
		if (g_EsifLogFile[j].handle != NULL) {
			esif_ccb_fclose(g_EsifLogFile[j].handle);
		}
		esif_ccb_free(g_EsifLogFile[j].name);
		esif_ccb_free(g_EsifLogFile[j].filename);
		esif_ccb_lock_uninit(&g_EsifLogFile[j].lock);
	}
	esif_ccb_memset(g_EsifLogFile, 0, sizeof(g_EsifLogFile));
}

int EsifLogFile_Open(EsifLogType type, const char *filename)
{
	int rc=0;
	char fullpath[MAX_PATH]={0};

	esif_ccb_write_lock(&g_EsifLogFile[type].lock);
	if (g_EsifLogFile[type].handle != NULL)
		esif_ccb_fclose(g_EsifLogFile[type].handle);

	EsifLogFile_GetFullPath(fullpath, sizeof(fullpath), filename);
#ifdef ESIF_ATTR_OS_WINDOWS
	g_EsifLogFile[type].handle = _fsopen(fullpath, "wc", _SH_DENYWR);
	if (g_EsifLogFile[type].handle == NULL)
		rc = errno;
#else
	rc = esif_ccb_fopen(&g_EsifLogFile[type].handle, fullpath, "w");
#endif
	if (rc == 0) {
		esif_ccb_free(g_EsifLogFile[type].filename);
		g_EsifLogFile[type].filename = esif_ccb_strdup((char *)fullpath);
	}
	esif_ccb_write_unlock(&g_EsifLogFile[type].lock);
	return rc;
}

int EsifLogFile_Close(EsifLogType type)
{
	int rc = EOF;

	esif_ccb_write_lock(&g_EsifLogFile[type].lock);
	if (g_EsifLogFile[type].handle != NULL)
		rc = esif_ccb_fclose(g_EsifLogFile[type].handle);
	g_EsifLogFile[type].handle = NULL;
	esif_ccb_write_unlock(&g_EsifLogFile[type].lock);
	return rc;
}

int EsifLogFile_IsOpen(EsifLogType type)
{
	int rc=0;
	esif_ccb_read_lock(&g_EsifLogFile[type].lock);
	rc = (g_EsifLogFile[type].handle != NULL);
	esif_ccb_read_unlock(&g_EsifLogFile[type].lock);
	return rc;
}

int EsifLogFile_Write(EsifLogType type, const char *fmt, ...)
{
	int rc = 0;
	va_list args;

	va_start(args, fmt);
	esif_ccb_write_lock(&g_EsifLogFile[type].lock);
	if (g_EsifLogFile[type].handle != NULL) {
		rc = esif_ccb_vfprintf(g_EsifLogFile[type].handle, fmt, args);
		fflush(g_EsifLogFile[type].handle);
	}
	esif_ccb_write_unlock(&g_EsifLogFile[type].lock);
	va_end(args);
	return rc;
}

int EsifLogFile_WriteArgs(EsifLogType type, const char *fmt, va_list args)
{
	int rc = 0;
	
	esif_ccb_write_lock(&g_EsifLogFile[type].lock);
	if (g_EsifLogFile[type].handle != NULL) {
		rc = esif_ccb_vfprintf(g_EsifLogFile[type].handle, fmt, args);
		fflush(g_EsifLogFile[type].handle);
	}
	esif_ccb_write_unlock(&g_EsifLogFile[type].lock);
	return rc;
}

esif_string EsifLogFile_GetFullPath(esif_string buffer, size_t buf_len, const char *filename)
{
	char *sep = strrchr((char *)filename, *ESIF_PATH_SEP);
	if (sep != NULL)
		filename = sep+1; // Ignore folders and use file.ext only

	esif_build_path(buffer, buf_len, ESIF_PATHTYPE_LOG, (esif_string)filename, NULL);
	return buffer;
}

void EsifLogFile_DisplayList(void)
{
	int j;
	for (j = 0; j < MAX_ESIFLOG; j++) {
		if (g_EsifLogFile[j].handle != NULL && g_EsifLogFile[j].name != NULL) {
			CMD_OUT("%s log: %s\n", g_EsifLogFile[j].name, (g_EsifLogFile[j].filename ? g_EsifLogFile[j].filename : "NA"));
		}
	}
}

EsifLogType EsifLogType_FromString(const char *name)
{
	EsifLogType result = (EsifLogType)0;
	if (NULL == name)
		return result;
	else if (esif_ccb_strnicmp(name, "eventlog", 5)==0)
		result = ESIF_LOG_EVENTLOG;
	else if (esif_ccb_strnicmp(name, "debugger", 5)==0)
		result = ESIF_LOG_DEBUGGER;
	else if (esif_ccb_stricmp(name, "shell")==0)
		result = ESIF_LOG_SHELL;
	else if (esif_ccb_stricmp(name, "trace")==0)
		result = ESIF_LOG_TRACE;
	else if (esif_ccb_stricmp(name, "ui")==0)
		result = ESIF_LOG_UI;
	return result;
}

#ifdef ESIF_ATTR_OS_WINDOWS
extern enum esif_rc write_to_srvr_cnsl_intfc_varg(const char *pFormat, va_list args);
# define EsifConsole_vprintf(fmt, args)		write_to_srvr_cnsl_intfc_varg(fmt, args)
#else
# define EsifConsole_vprintf(fmt, args)		vprintf(fmt, args)
#endif

int EsifConsole_WriteTo(u32 writeto, const char *format, ...)
{
	int rc=0;

	if (writeto & CMD_WRITETO_CONSOLE) {
		va_list args;
		va_start(args, format);
		rc = EsifConsole_vprintf(format, args);
		va_end(args);
	}
	if ((writeto & CMD_WRITETO_LOGFILE) && (g_EsifLogFile[ESIF_LOG_SHELL].handle != NULL)) {
		va_list args;
		va_start(args, format);
		esif_ccb_write_lock(&g_EsifLogFile[ESIF_LOG_SHELL].lock);
		rc = esif_ccb_vfprintf(g_EsifLogFile[ESIF_LOG_SHELL].handle, format, args);
		esif_ccb_write_unlock(&g_EsifLogFile[ESIF_LOG_SHELL].lock);
		va_end(args);
	}
	return rc;
}

int EsifConsole_WriteConsole(const char *format, va_list args)
{
	return EsifConsole_vprintf(format, args);
}

int EsifConsole_WriteLogFile(const char *format, va_list args)
{
	int rc=0;
	esif_ccb_write_lock(&g_EsifLogFile[ESIF_LOG_SHELL].lock);
	rc = esif_ccb_vfprintf(g_EsifLogFile[ESIF_LOG_SHELL].handle, format, args);
	esif_ccb_write_unlock(&g_EsifLogFile[ESIF_LOG_SHELL].lock);
	return rc;
}

/* Work Around */
static enum esif_rc get_participant_data(
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
	*(u32 *)(command_ptr + 1) = participantId;
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
	data_ptr = (struct esif_command_get_participant_detail *)(command_ptr + 1);
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
static enum esif_rc sync_lf_participants()
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

	data_ptr = (struct esif_command_get_participants *)(command_ptr + 1);
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

		size = sizeof(struct esif_ipc_event_header) +
			sizeof(struct esif_ipc_event_data_create_participant);
		event_hdr_ptr = (struct esif_ipc_event_header *)esif_ccb_malloc(size);
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

		event_cp_ptr = (struct esif_ipc_event_data_create_participant *)(event_hdr_ptr + 1);
		get_participant_data(event_cp_ptr, i);

		EsifEventProcess(event_hdr_ptr);

		esif_ccb_free(event_hdr_ptr);
	}
	ESIF_TRACE_DEBUG("\n");

exit:
	ESIF_TRACE_DEBUG("%s: rc = %s(%u)", ESIF_FUNC, esif_rc_str(rc), rc);

	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return rc;
}


void done(const void *context)
{
	CMD_OUT("Context = %p %s\n", context, (char *)context);
}

/* Build Full Pathname for a given path type including an optional filename and extention. 
 * Defaults below, where "C:\Program Files..." is either:
 *     "C:\Program Files\Intel\Intel(R) Dynamic Platform and Thermal Framework"
 *  or "C:\Program Files (x86)\Intel\Intel(R) Dynamic Platform and Thermal Framework"
 * ("+" = writeable)
 * 
 * Type	Linux					Windows
 * ----	-----------------------	----------------------------------------------------------------------------
 * HOME	/usr/share/dptf			C:\Program Files...
 *+TEMP /tmp					C:\Windows\Temp  or  C:\Users\<username>\AppData\Local\Temp
 *+DV	/etc/esif				C:\Windows\ServiceProfiles\LocalService\AppData\Intel\DPTF
 *+LOG	Same as DV				Same as DV
 *+BIN	/usr/share/dptf/bin		C:\Program Files...\bin
 *+LOCK	/var/run				Same as TEMP
 * EXE	/usr/share/dptf/ufx64	C:\Program Files...\ufx64 [or ufx86]
 * DLL	/usr/share/dptf/ufx64	C:\Program Files...\ufx64 [or ufx86]
 * DSP	/usr/share/dptf/dsp		C:\Program Files...\dsp
 * CMD	/usr/share/dptf/cmd		C:\Program Files...\cmd
 * UI	/usr/share/dptf			C:\Program Files...
 * DPTF	/usr/share/dptf			ufx64  [or ufx86]
 */
esif_string esif_build_path(
	esif_string buffer, 
	size_t buf_len,
	esif_pathtype type,
	esif_string filename,
	esif_string ext)
{
	char home[MAX_PATH]={0};
	size_t len = 0;

	if (NULL == buffer) {
		return NULL;
	}

	// If g_home is undefined, use default location
	if (NULL == g_home) {
	#if defined(ESIF_ATTR_OS_WINDOWS)
		char winHome[MAX_PATH]={0};
		if (SHGetSpecialFolderPathA(0, winHome, CSIDL_PROGRAM_FILES, FALSE) == TRUE) {
			#if defined(ESIF_ATTR_64BIT)
			esif_ccb_strcat(winHome, " (x86)", sizeof(winHome));
			#endif
			esif_ccb_strcat(winHome, "\\Intel\\Intel(R) Dynamic Platform and Thermal Framework", sizeof(winHome));
		}
		if (esif_ccb_file_exists(winHome)) {
			g_home = esif_ccb_strdup(winHome);
		}
	#elif defined(ESIF_ATTR_OS_ANDROID)
		g_home = esif_ccb_strdup("."); // Placeholder
	#elif defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_CHROME)
		static char *home_default = "/usr/share/dptf";
		if (esif_ccb_file_exists(home_default)) {
			g_home = esif_ccb_strdup(home_default);
		}
	#endif
	}

	// if g_DataVaultDir is undefined, use default
	if (g_DataVaultDir[0] == 0) {
	#if defined(ESIF_ATTR_OS_WINDOWS)
		if (GetWindowsDirectoryA(g_DataVaultDir, sizeof(g_DataVaultDir)) == 0) {
			esif_ccb_strcpy(g_DataVaultDir, "C:\\Windows", sizeof(g_DataVaultDir));
		}
		esif_ccb_strcat(g_DataVaultDir, "\\ServiceProfiles\\LocalService\\AppData\\Local\\Intel\\DPTF", sizeof(g_DataVaultDir));
	#elif defined(ESIF_ATTR_OS_ANDROID)
		esif_ccb_strcpy(g_DataVaultDir, ".", sizeof(g_DataVaultDir)); // Placeholder
	#elif defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_CHROME)
		esif_ccb_strcpy(g_DataVaultDir, "/etc/dptf", sizeof(g_DataVaultDir));
	#endif
	}

	// Build Home Directory, trimming any trailing slash
	if (NULL != g_home) {
		esif_ccb_strcpy(home, g_home, buf_len);
	}
	len = esif_ccb_strlen(home, sizeof(home));
	if (len > 0 && home[len-1] == *ESIF_PATH_SEP) {
		home[len-1] = 0;
	}

	// Build Directory
	buffer[0] = 0;
	switch (type) {
	case ESIF_PATHTYPE_HOME: // Read-Only
		esif_ccb_strcpy(buffer, home, buf_len);
		break;
	
	// Read/Write Paths
	case ESIF_PATHTYPE_TEMP:
	#if defined(ESIF_ATTR_OS_WINDOWS)
		if ((len = (size_t)GetTempPathA((DWORD)buf_len, buffer)) > 1) {
			buffer[len-2] = 0; // Trim trailing slash
		}
		else {
			buffer[0] = 0;
		}
	#elif defined(ESIF_ATTR_OS_ANDROID)
		esif_ccb_strcpy(buffer, ".", buf_len); // Placeholder
	#elif defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_CHROME)
		esif_ccb_strcpy(buffer, "/tmp", buf_len);
	#endif
		break;

	case ESIF_PATHTYPE_DV:
		esif_ccb_strcpy(buffer, g_DataVaultDir, buf_len);
		break;

	case ESIF_PATHTYPE_LOG:
	#if defined(ESIF_ATTR_OS_WINDOWS)
		esif_ccb_strcpy(buffer, g_DataVaultDir, buf_len);
	#elif defined(ESIF_ATTR_OS_ANDROID)
		esif_ccb_strcpy(buffer, g_DataVaultDir, buf_len); // Placeholder
	#elif defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_CHROME)
		// This may use a different path (/var/log/...) in the future
		esif_ccb_strcpy(buffer, g_DataVaultDir, buf_len);
	#endif
		break;

	case ESIF_PATHTYPE_BIN:
		// TODO: These may need to go to writable DV dir in Windows
		esif_ccb_sprintf(buf_len, buffer, "%s%s%s", home, ESIF_PATH_SEP, ESIF_DIR_BIN);
		break;

	// Read-Only Paths
	case ESIF_PATHTYPE_EXE:
		// Binaries Use full path (ufx64/ufx86 folder) in Windows, otherwise Home directory
	#if defined(ESIF_ATTR_OS_WINDOWS)
		esif_ccb_sprintf(buf_len, buffer, "%s%s%s", home, ESIF_PATH_SEP, ESIF_DIR_EXE);
	#elif defined(ESIF_ATTR_OS_ANDROID)
		esif_ccb_strcpy(buffer, home, buf_len); // Placeholder
	#elif defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_CHROME)
		esif_ccb_strcpy(buffer, home, buf_len);
	#endif
		break;

	case ESIF_PATHTYPE_DLL:
		// Shared Libraries loaded by ESIF. Use full path (ufx64/ufx86 folder), otherwise Home directory
	#if defined(ESIF_ATTR_OS_WINDOWS)
		esif_ccb_sprintf(buf_len, buffer, "%s%s%s", home, ESIF_PATH_SEP, ESIF_DIR_EXE);
	#elif defined(ESIF_ATTR_OS_ANDROID)
		esif_ccb_strcpy(buffer, home, buf_len); // Placeholder
	#elif defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_CHROME)
		esif_ccb_strcpy(buffer, "", buf_len);
	#endif
		break;

	case ESIF_PATHTYPE_DPTF:
		// Shared Libraries loaded by DPTF app. Always pass full path to the app.
	#if defined(ESIF_ATTR_OS_WINDOWS)
		esif_ccb_sprintf(buf_len, buffer, "%s%s%s", home, ESIF_PATH_SEP, ESIF_DIR_EXE);
	#elif defined(ESIF_ATTR_OS_ANDROID)
		esif_ccb_strcpy(buffer, home, buf_len); // Placeholder
	#elif defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_CHROME)
		esif_ccb_strcpy(buffer, home, buf_len);
	#endif
		break;

	case ESIF_PATHTYPE_DSP:
		esif_ccb_sprintf(buf_len, buffer, "%s%s%s", home, ESIF_PATH_SEP, ESIF_DIR_DSP);
		break;

	case ESIF_PATHTYPE_CMD:
		esif_ccb_sprintf(buf_len, buffer, "%s%s%s", home, ESIF_PATH_SEP, ESIF_DIR_CMD);
		break;

	case ESIF_PATHTYPE_UI:
		esif_ccb_strcpy(buffer, home, buf_len);
		break;
	default:
		break;
	}

	// Append Optional filename.ext
	if (filename != NULL || ext != NULL) {
		if (buffer[0] != 0)
			esif_ccb_strcat(buffer, ESIF_PATH_SEP, buf_len);
		if (filename != NULL)
			esif_ccb_strcat(buffer, filename, buf_len);
		if (ext != NULL)
			esif_ccb_strcat(buffer, ext, buf_len);
	}
	return buffer;
}

// WebSocket Server Implemented?
#ifdef ESIF_ATTR_WEBSOCKET

static void *esif_web_worker_thread(void *ptr)
{
	UNREFERENCED_PARAMETER(ptr);
	esif_ws_init();
	return 0;
}

eEsifError EsifWebStart()
{
	
	if (!EsifWebIsStarted()) {
		esif_ccb_thread_create(&g_webthread, esif_web_worker_thread, NULL);
	}
	return ESIF_OK;
}

void EsifWebStop()
{
	if (EsifWebIsStarted()) {
		esif_ws_exit(&g_webthread);
	}
}

int EsifWebIsStarted()
{
	extern atomic_t g_ws_threads;
	return (atomic_read(&g_ws_threads) > 0);
}

void EsifWebSetIpaddrPort(const char *ipaddr, u32 port)
{
	esif_ws_set_ipaddr_port(ipaddr, port);
}

#else

eEsifError EsifWebStart()
{
	return ESIF_E_NOT_IMPLEMENTED;
}
void EsifWebStop()
{
}
int EsifWebIsStarted()
{
	return 0;
}
void EsifWebSetIpaddrPort(const char *ipaddr, u32 port)
{
	UNREFERENCED_PARAMETER(ipaddr);
	UNREFERENCED_PARAMETER(port);
}
#endif

eEsifError esif_uf_init(esif_string home_dir)
{
	eEsifError rc = ESIF_OK;

#ifdef ESIF_ATTR_MEMTRACE
	esif_memtrace_init();	/* must be called first */
#endif
#ifdef ESIF_ATTR_SHELL_LOCK
	esif_ccb_mutex_init(&g_shellLock);
#endif

	ESIF_TRACE_DEBUG("%s: Init Upper Framework (UF)", ESIF_FUNC);

	esif_ccb_mempool_init_tracking();

	// If home_dir specified, use it, otherwise use OS-specific default
	if (NULL != home_dir) {
		size_t len = esif_ccb_strlen(home_dir, MAX_PATH);
		if (len > 0) {
			esif_ccb_free(g_home);
			g_home = esif_ccb_strdup(home_dir);
			if (g_home[len-1] == *ESIF_PATH_SEP) {
				g_home[len-1] = 0; // trim trailing slash
			}
		}
	} 
	else {
		char home[MAX_PATH]={0};
		esif_build_path(home, sizeof(home), ESIF_PATHTYPE_HOME, NULL, NULL);
	}
	CMD_OUT("Home: %s\n", g_home);


	/* OS Agnostic */
	EsifLogMgrInit();
	EsifCfgMgrInit();
	EsifCnjMgrInit();
	EsifUppMgrInit();
	EsifDspMgrInit();
	EsifActMgrInit();

	/* Web Server optionally started by shell scripts in esif_init */

	/* OS Specific */
	rc = esif_uf_os_init();
	if (ESIF_OK != rc) {
		goto exit;
	}

	ipc_connect();
	sync_lf_participants();

	/* Start App Manager after all dependent components started
	 * This does not actually start any apps.
	 */
	EsifAppMgrInit();

exit:
	return rc;
}


void esif_uf_exit()
{
	/* Stop event thread */
	g_quit = ESIF_TRUE;

	/* Stop web server if it is running */
	EsifWebStop();

	/* Stop all Apps before dependent components they may be using */
	EsifAppMgrExit();

	/* OS Specific */
	esif_uf_os_exit();

	/* OS Agnostic - Call in reverse order of Init */
	EsifActMgrExit();
	EsifDspMgrExit();
	EsifUppMgrExit();
	EsifCnjMgrExit();
	EsifCfgMgrExit();
	EsifLogMgrExit();

	esif_ccb_free(g_home);

	esif_ccb_mempool_uninit_tracking();

	// Re-Initialize necessary global variables in case ESIF restarted
	g_esif_started = ESIF_FALSE;

	ESIF_TRACE_DEBUG("%s: Exit Upper Framework (UF)", ESIF_FUNC);

#ifdef ESIF_ATTR_SHELL_LOCK
	esif_ccb_mutex_uninit(&g_shellLock);
#endif
#ifdef ESIF_ATTR_MEMTRACE
	esif_memtrace_exit();	/* must be called last */
#endif
}


/* Memory Allocation Trace Support */
#ifdef ESIF_ATTR_MEMTRACE

void *esif_memtrace_alloc(
	void *old_ptr,
	size_t size,
	const char *func,
	const char *file,
	int line
	)
{
	struct memalloc_s *mem = NULL;
	struct memalloc_s **last = NULL;
	void *mem_ptr = NULL;

	esif_ccb_write_lock(&g_memtrace.lock);
	mem = g_memtrace.allocated;
	last = &g_memtrace.allocated;

	if (file) {
		const char *slash = strrchr(file, *ESIF_PATH_SEP);
		if (slash) {
			file = slash + 1;
		}
	}

	if (old_ptr) {
		mem_ptr = native_realloc(old_ptr, size);

		// realloc(ptr, size) leaves ptr unaffected if realloc fails and size is nonzero
		if (!mem_ptr && size > 0) {
			goto exit;
		}
		while (mem) {
			if (old_ptr == mem->mem_ptr) {
				// realloc(ptr, 0) behaves like free(ptr)
				if (size == 0) {
					*last = mem->next;
					native_free(mem);
				} else {
					mem->mem_ptr = mem_ptr;
					mem->size    = size;
					mem->func    = func;
					mem->file    = file;
					mem->line    = line;
				}
				goto exit;
			}
			last = &mem->next;
			mem = mem->next;
		}
	} else {
		mem_ptr = native_malloc(size);
		if (mem_ptr) {
			esif_ccb_memset(mem_ptr, 0, size);
			atomic_inc(&g_memtrace.allocs);
		}
	}

	mem = (struct memalloc_s *)native_malloc(sizeof(*mem));
	if (!mem) {
		goto exit;
	}
	esif_ccb_memset(mem, 0, sizeof(*mem));
	mem->mem_ptr = mem_ptr;
	mem->size    = size;
	mem->func    = func;
	mem->file    = file;
	mem->line    = line;
	mem->next    = g_memtrace.allocated;
	g_memtrace.allocated = mem;
exit:
	esif_ccb_write_unlock(&g_memtrace.lock);
	return mem_ptr;
}


char *esif_memtrace_strdup(
	char *str,
	const char *func,
	const char *file,
	int line
	)
{
	size_t len    = esif_ccb_strlen(str, 0x7fffffff) + 1;
	char *mem_ptr = (char *)esif_memtrace_alloc(0, len, func, file, line);
	if (NULL != mem_ptr) {
		esif_ccb_strcpy(mem_ptr, str, len);
	}
	return mem_ptr;
}


void esif_memtrace_free(void *mem_ptr)
{
	struct memalloc_s *mem   = NULL;
	struct memalloc_s **last = NULL;

	esif_ccb_write_lock(&g_memtrace.lock);
	mem  = g_memtrace.allocated;
	last = &g_memtrace.allocated;

	while (mem) {
		if (mem_ptr == mem->mem_ptr) {
			*last = mem->next;
			native_free(mem);
			goto exit;
		}
		last = &mem->next;
		mem  = mem->next;
	}
exit:
	esif_ccb_write_unlock(&g_memtrace.lock);
	if (mem_ptr) {
		native_free(mem_ptr);
		atomic_inc(&g_memtrace.frees);
	}
}


void esif_memtrace_init()
{
	struct memalloc_s *mem = NULL;

	esif_ccb_lock_init(&g_memtrace.lock);
	esif_ccb_write_lock(&g_memtrace.lock);
	mem = g_memtrace.allocated;

	// Ignore any allocations made before this function was called
	while (mem) {
		struct memalloc_s *node = mem;
		mem = mem->next;
		native_free(node);
	}
	g_memtrace.allocated = NULL;
	esif_ccb_write_unlock(&g_memtrace.lock);
}

void esif_memtrace_exit()
{
	struct memalloc_s *mem = NULL;
	char tracefile[MAX_PATH] = {0};
	FILE *tracelog = NULL;

	esif_ccb_write_lock(&g_memtrace.lock);
	mem = g_memtrace.allocated;
	g_memtrace.allocated = NULL;
	esif_ccb_write_unlock(&g_memtrace.lock);

	CMD_OUT("MemTrace: Allocs=" ATOMIC_FMT " Frees=" ATOMIC_FMT "\n", atomic_read(&g_memtrace.allocs), atomic_read(&g_memtrace.frees));
	if (!mem) {
		goto exit;
	}

	esif_build_path(tracefile, sizeof(tracefile), ESIF_PATHTYPE_LOG, "memtrace.txt", NULL);
	CMD_OUT("\n*** MEMORY LEAKS DETECTED ***\nFor details see %s\n", tracefile);
	esif_ccb_fopen(&tracelog, tracefile, "w");
	while (mem) {
		struct memalloc_s *node = mem;
		if (tracelog) {
			fprintf(tracelog, "[%s @%s:%d]: (%lld bytes) %p\n", mem->func, mem->file, mem->line, (long long)mem->size, mem->mem_ptr);
		}
		mem = mem->next;
		native_free(node->mem_ptr);
		native_free(node);
	}
	if (tracelog) {
		esif_ccb_fclose(tracelog);
	}
exit:
	esif_ccb_lock_uninit(&g_memtrace.lock);
}


/************/

#endif /* ESIF_ATTR_MEMTRACE */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

