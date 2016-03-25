/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID ESIF_TRACEMODULE_SHELL

#include "esif_uf_version.h"

const char *g_esif_shell_version = ESIF_UF_VERSION;

extern char *g_esif_etf_version;

char g_esif_kernel_version[64] = "NA";

/* ESIF */
#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_shell.h"	/* Shell / Command Line Interface */
#include "esif_uf_test.h"	/* Test Framework */
#include "esif_uf_ipc.h"	/* IPC */

/* Managers */
#include "esif_pm.h"		/* Upper Participant Manager */
#include "esif_uf_appmgr.h"	/* Application Manager */
#include "esif_uf_actmgr.h"	/* Action Manager */
#include "esif_uf_cnjmgr.h"	/* Conjure Manager */
#include "esif_uf_cfgmgr.h"	/* Config Manager */
#include "esif_uf_eventmgr.h"

#include "esif_uf_primitive.h"

#include "esif_command.h"	/* Commands */
#include "esif_ipc.h"		/* IPC Abstraction */
#include "esif_primitive.h"	/* Primitive */
#include "esif_cpc.h"		/* Compact Primitive Catalog */
#include "esif_uf_logging.h"
#include "esif_uf_ccb_thermalapi.h"
#include "esif_uf_loggingmgr.h"

// SDK
#include "esif_sdk_capability_type.h" /* For Capability Id Description*/

// friend classes
#define _DATABANK_CLASS

#include "esif_lib_databank.h"

#include "esif_dsp.h"
#include "esif_uf_fpc.h"
#include "esif_uf_ccb_system.h"
#include "esif_uf_app.h"
#include "esif_uf_tableobject.h"
extern struct esif_uf_dm g_dm;

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// Shell strings limited to current size of output buffer
#define ESIF_SHELL_STRLEN(s)        esif_ccb_strlen(s, OUT_BUF_LEN)

// Alias Dispatcher
char *esif_shell_exec_dispatch(esif_string line, esif_string output);

#define FILE_READ         "rb"
#define FILE_WRITE        "w"
#define PATH_STR_LEN      MAX_PATH
#define REG_EXPR_STR_LEN  256

// Limits
#define MAX_CPC_SIZE        0x7ffffffe
#define MAX_PRIMITIVES      0x7ffffffe
#define MAX_ALGORITHMS      0x7ffffffe
#define MAX_DOMAINS         ESIF_DOMAIN_MAX
#define MAX_EVENTS          0x7ffffffe
#define MAX_ARGV            32
#define MAX_REPEAT			0x7ffffffe
#define MAX_REPEAT_DELAY	0x7ffffffe

/* Friends */
extern EsifAppMgr g_appMgr;
extern EsifCnjMgr g_cnjMgr;
extern EsifUppMgr g_uppMgr;

int g_dst   = 0;

// StopWatch
struct timeval g_timer = {0};

enum output_format g_format = FORMAT_TEXT;
int g_shell_enabled = 0;	// user shell enabled?
int g_cmdshell_enabled = 1;	// "!cmd" type shell commands enabled (if shell enabled)?

static UInt8 g_isRest = 0;

//
// NOT Declared In Header Only This Module Should Use These
//
int g_binary_buf_size = 4096;	// Buffer Size
extern int g_errorlevel;	// Exit Errorlevel
extern int g_quit;			// Quit Application?
extern int g_disconnectClient;	// Disconnect shell client
int g_repeat = 1;		// Repeat N Times
int g_repeat_delay = 0;		// Repeat Delay In msec
extern int g_timestamp;		// Timestamp on / off?
char g_os[64];

int g_soe = 1;

// Global Shell lock to limit parse_cmd to one thread at a time
static esif_ccb_mutex_t g_shellLock;

// ESIF Global Shell Output Buffer
static char *g_outbuf = NULL;				// Dynamically created and can grow
UInt32 g_outbuf_len = OUT_BUF_LEN_DEFAULT;	// Current (or Default) Size of ESIF Shell Output Buffer

struct esif_data_binary_bst_package {
	union esif_data_variant battery_state;
	union esif_data_variant battery_present_rate;
	union esif_data_variant battery_remaining_capacity;
	union esif_data_variant battery_present_voltage;
};



// For use with ltrim()
#define PREFIX_ALGORITHM_TYPE	20	// ESIF_ALGORITHM_TYPE_
#define PREFIX_ACTION_TYPE		12	// ESIF_ACTION_
#define PREFIX_DATA_TYPE		15	// ESIF_DATA_TYPE_
#define PREFIX_EVENT_GROUP		17	// ESIF_EVENT_GROUP_
#define PREFIX_EVENT_TYPE		11	// ESIF_EVENT_

// Return str with the first prefix characters removed if str length > prefix
static char *ltrim(char *str, size_t prefix)
{
	return ((esif_ccb_strlen(str, prefix + 2) > prefix) ? (str + prefix) : (str));
}

// low-level vsprintf to an auto-growing buffer, starting at offset
static int esif_shell_vsprintfto(
	size_t *buf_len,
	char **buffer,
	size_t offset,
	const char *format,
	va_list args
	)
{
	int result = 0;
	int len = esif_ccb_vscprintf(format, args);
	size_t buf_needed = offset + len + 1;

	if (buf_needed > *buf_len) {
		char *new_buffer = esif_ccb_realloc(*buffer, buf_needed);
		if (new_buffer != NULL) {
			*buffer = new_buffer;
			*buf_len = buf_needed;
		}
	}
	if (*buffer) {
		result = esif_ccb_vsprintf(*buf_len - offset, *buffer + offset, format, args);
	}
	return result;
}

// sprintf to an auto-growing string
static int esif_shell_sprintf(
	size_t *buf_len,
	char **buffer,
	const char *format,
	...
	)
{
	int result;
	va_list args;
	va_start(args, format);
	result = esif_shell_vsprintfto(buf_len, buffer, 0, format, args);
	va_end(args);
	return result;
}

// sprintf_concat to an auto-growing string
static int esif_shell_sprintf_concat(
	size_t *buf_len,
	char **buffer,
	const char *format,
	...
	)
{
	int result;
	va_list args;
	va_start(args, format);
	result = esif_shell_vsprintfto(buf_len, buffer, esif_ccb_strlen(*buffer, *buf_len), format, args);
	va_end(args);
	return result;
}

// Use our own strtok() function, which understands "quoted strings with spaces"
#define ESIF_SHELL_STRTOK_SEP   " \t\r\n"
static char *g_line_context;
#undef  esif_ccb_strtok
#define esif_ccb_strtok(str, sep, ctxt) esif_shell_strtok(str, sep, ctxt)

static char *esif_shell_strtok(
	char *str,
	char *seps,
	char **context
	)
{
	char *result  = 0;
	char quote[2] = {0};
	if (!str) {
		str = *context;
	}
	if (str) {
		while (*str && strchr(seps, *str)) {
			str++;
		}

		if (*str) {
			// Allow "Quoted Strings" or 'Quoted Strings' with embedded spaces or other separators
			if (*str == '\"' || *str == '\'') {
				quote[0] = *str;
				seps     = quote;
				str++;
			}
			result = str;
			str    = strpbrk(str, seps);
			if (str) {
				*str++   = 0;
				*context = str;
			} else {
				*context = result + ESIF_SHELL_STRLEN(result);
			}
		}
	}
	return result;
}


// Shell ATOI
unsigned int esif_atoi(const esif_string str)
{
	unsigned int val = 0;
	if (NULL == str) {
		return 0;
	}

	if (!strncmp(str, "0x", 2)) {
		esif_ccb_sscanf(str + 2, "%x", &val);
	} else {
		esif_ccb_sscanf(str, "%d", (int *)&val);
	}

	return val;
}


UInt64 esif_atoi64(const esif_string str)
{
	UInt64 val = 0;
	if (NULL == str) {
		return 0;
	}

	if (!strncmp(str, "0x", 2)) {
		esif_ccb_sscanf(str + 2, "%llx", &val);
	} else {
		esif_ccb_sscanf(str, "%lld", (Int64 *) &val);
	}

	return val;
}


/* Need Header */
char *esif_str_replace(
	char *orig,
	char *rep,
	char *with
	)
{
	char *result;	// the return string
	char *ins;		// the next insert point
	char *tmp;		// varies
	size_t tmp_len   = 0;
	size_t len_rep   = 0;	// length of rep
	size_t len_with  = 0;	// length of with
	size_t len_front = 0;	// distance between rep and end of last rep
	int count;		// number of replacements

	if (NULL == orig) {
		return NULL;
	}
	len_rep = (rep ? ESIF_SHELL_STRLEN(rep) : 0);

	if (NULL == rep || 0 == len_rep) {
		return NULL;
	}
	ins = strstr(orig, rep);

	if (NULL == ins) {
		return NULL;
	}

	if (NULL == with) {
		with = (char *)"";
	}
	len_with = ESIF_SHELL_STRLEN(with);

	/* Count the number of replacement instances */
	for (count = 0; NULL != (tmp = strstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}

	if (0 == count) {
		return NULL;
	}

	/*
	** first time through the loop, all the variable are set correctly
	** from here on,
	**    tmp points to the end of the result string
	**    ins points to the next occurrence of rep in orig
	**    orig points to the remainder of orig after "end of rep"
	*/
	tmp_len = ESIF_SHELL_STRLEN(orig) + (len_with - len_rep) * count + 1;

	tmp     = result = (char *)esif_ccb_malloc(tmp_len);

	if (NULL == result) {
		return NULL;
	}

	while (count--) {
		ins = strstr(orig, rep);/* Find the replacement in origin */

		len_front = ins - orig;	/* Calculate bytes before replacement */
		esif_ccb_strcpy(tmp, orig, len_front+1); /* Copy entire string */
		tmp += len_front;					/* Move To Repalement Loc */

		/* Copy the replacement @ the replace locaiton */
		esif_ccb_strcpy(tmp, with, len_with+1);
		tmp  += len_with;

		orig += len_front + len_rep;// move to next "end of rep"
	}

	/*
	** Append what we have left of orig to the end.
	*/
	esif_ccb_strcat(result, orig, tmp_len);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// INITIALIZE
///////////////////////////////////////////////////////////////////////////////

// Init Shell
eEsifError esif_uf_shell_init()
{
	esif_ccb_mutex_init(&g_shellLock);
	if ((g_outbuf = esif_ccb_malloc(OUT_BUF_LEN)) == NULL) {
		return ESIF_E_NO_MEMORY;
	}
	return ESIF_OK;
}

// Uninit Shell
void esif_uf_shell_exit()
{
	esif_ccb_mutex_uninit(&g_shellLock);
	esif_ccb_free(g_outbuf);
	g_outbuf = NULL;
}

// Exclusively Lock Shell
void esif_uf_shell_lock()
{
	esif_ccb_mutex_lock(&g_shellLock);
}

// Unlock Lock Shell
void esif_uf_shell_unlock()
{
	esif_ccb_mutex_unlock(&g_shellLock);
}

// Resize ESIF Shell Buffer if necessary
char *esif_shell_resize(size_t buf_len)
{
	if (buf_len > OUT_BUF_LEN) {
		char *buf_ptr = esif_ccb_realloc(g_outbuf, buf_len);
		if (buf_ptr != NULL) {
			g_outbuf = buf_ptr;
			g_outbuf_len = (UInt32)buf_len;
		}
	}
	return g_outbuf;
}

// Init
void esif_init()
{
	// Run Autostart script on first execution
	ESIF_TRACE_ENTRY_INFO();
	if (g_esif_started == ESIF_FALSE) {
		char command[MAX_LINE] = {0};
		char filepath[MAX_PATH] = {0};

		CMD_OUT("Start ESIF Upper Framework Shell\n");

		// Execute "start" script file in cmd directory, if one exists
		if (esif_build_path(filepath, sizeof(filepath), ESIF_PATHTYPE_CMD, "start", NULL) != NULL && esif_ccb_file_exists(filepath)) {
			esif_ccb_strcpy(command, "load start", sizeof(command));
		}
		// Use startup script in startup.dv datavault, if it exists
		else if (DataBank_KeyExists(g_DataBankMgr, "startup", "start")) {
			esif_ccb_strcpy(command, "config exec @startup start", sizeof(command));
		}
		// Use startup script in Default datavault, if it exists
		else if (DataBank_KeyExists(g_DataBankMgr, g_DataVaultDefault, "start")) {
			esif_ccb_strcpy(command, "config exec start", sizeof(command));
		}
		// Otherwise Use default startup script, if any
		else {
			esif_ccb_strcpy(command, "autoexec", sizeof(command));
		}

		// Execute Startup Script, if one was found
		if (command[0]) {
			parse_cmd(command, ESIF_FALSE);
		}
		g_esif_started = ESIF_TRUE;
	}

	// Build OS Display String
	esif_ccb_sprintf(sizeof(g_os), g_os, "%s %s %s", ESIF_ATTR_OS, ESIF_PLATFORM_TYPE, ESIF_BUILD_TYPE);
	g_os[0] = (char)toupper(g_os[0]);

	// Display Banner if Shell Enabled. Intial Prompt will follow
	if (g_shell_enabled) {
		CMD_OUT("\n\nEEEEEEEEEE   SSSSSSSSSS   IIIIIIIII   FFFFFFFFFF\n"
					"EEE          SSS             III      FFF\n"
					"EEE          SSS             III      FFF\n"
					"EEEEEEEEEE   SSSSSSSSSS      III      FFFFFFFFFF\n"
					"EEE                 SSS      III      FFF\n"
					"EEE                 SSS      III      FFF     OS:      %s\n"
					"EEEEEEEEEE   SSSSSSSSSS   IIIIIIIII   FFF     Version: %s\n\n",
				g_os, g_esif_shell_version);
	}
	else {
		CMD_OUT("ESIF Shell Disabled\n");
	}
	ESIF_TRACE_EXIT_INFO();
}


// Exit
void esif_uf_subsystem_exit()
{
}


///////////////////////////////////////////////////////////////////////////////
// PARSED COMMANDS
///////////////////////////////////////////////////////////////////////////////


static char *esif_shell_cmd_dspquery(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	EsifDspQuery query = { 0 };
	EsifString dspName = NULL;
	char participantName[MAX_ACPI_DEVICE_STRING_LENGTH] = { 0 };
	char vendorId[MAX_VENDOR_ID_STRING_LENGTH] = { 0 };
	char deviceId[MAX_DEVICE_ID_STRING_LENGTH] = { 0 };
	char participantEnum[ENUM_TO_STRING_LEN] = { 0 };
	char participantPType[ENUM_TO_STRING_LEN] = { 0 };
	char hid[MAX_ACPI_DEVICE_STRING_LENGTH] = { 0 };
	char uid[MAX_ACPI_UID_STRING_LENGTH] = { 0 };
	int opt = 1;

	if (argc > opt) {
		esif_ccb_strcpy(participantName, argv[opt++], sizeof(participantName));
	}

	if (argc > opt) {
		esif_ccb_strcpy(vendorId, argv[opt++], sizeof(vendorId));
	}

	if (argc > opt) {
		esif_ccb_strcpy(deviceId, argv[opt++], sizeof(deviceId));
	}

	if (argc > opt) {
		esif_ccb_strcpy(participantEnum, argv[opt++], sizeof(participantEnum));
	}

	if (argc > opt) {
		esif_ccb_strcpy(participantPType, argv[opt++], sizeof(participantPType));
	}

	if (argc > opt) {
		esif_ccb_strcpy(hid, argv[opt++], sizeof(hid));
	}

	if (argc > opt) {
		esif_ccb_strcpy(uid, argv[opt++], sizeof(uid));
	}

	query.vendorId = vendorId;
	query.deviceId = deviceId;
	query.enumerator = participantEnum;
	query.hid = hid;
	query.uid = uid;
	query.ptype = participantPType;
	query.participantName = participantName;

	dspName = EsifDspMgr_SelectDsp(query);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", (dspName ? dspName : ESIF_NOT_AVAILABLE));
	return output;
}


static char *esif_shell_cmd_dsps(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	u8 i = 0;
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (g_format == FORMAT_XML) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<result>\n");

		for (i = 0; i < g_dm.dme_count; i++) {
			struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
			char version[8];

			if (NULL == dsp_ptr) {
				continue;
			}

			esif_ccb_sprintf(8, version, "%u.%u",
							 *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  <dsp>\n");
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "    <id>%u</id>\n"
							 "    <enum>%u</enum>\n"
							 "    <package>%s</package>\n"
							 "    <name>%s</name>\n"
							 "    <guid>%s</guid>\n"
							 "    <version>%s</version>\n"
							 "    <acpiHID>%s</acpiHID>\n"
							 "    <acpiUID>%s</acpiUID>\n"
							 "    <acpiType>%s</acpiType>\n"
							 "    <acpiScope>%s</acpiScope>\n"
							 "    <pciVendor>%s</pciVendor>\n"
							 "    <pciDevice>%s</pciDevice>\n"
							 "    <pciBus>%u</pciBus>\n"
							 "    <pciBusDevice>%u</pciBusDevice>\n"
							 "    <pciFunction>%u</pciFunction>\n"
							 "    <pciRevision></pciRevision>\n"
							 "    <pciClass></pciClass>\n"
							 "    <pciSubClass></pciSubClass>\n"
							 "    <pciProgIf></pciProgIf>\n",
							 i,
							 *dsp_ptr->bus_enum,
							 dsp_ptr->code_ptr,
							 dsp_ptr->code_ptr + 3,
							 dsp_ptr->type,
							 version,
							 dsp_ptr->acpi_device,
							 "",
							 dsp_ptr->acpi_type,
							 dsp_ptr->acpi_scope,
							 dsp_ptr->vendor_id,
							 dsp_ptr->device_id,
							 *dsp_ptr->pci_bus,
							 *dsp_ptr->pci_bus_device,
							 *dsp_ptr->pci_function);

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  </dsp>\n");
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</result>\n");

		return output;
	}


	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nLoaded Device Support Packages (DSP):\n"
					 "Count:  %u\n\n",
					 g_dm.dme_count);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "ACPI Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: = (HID(8) & TYPE(4) & UID(2) & SCOPE(1))\n"
					 "Minterms: 4\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION HID      TYPE UID SCOPE\n"
					 "-- ------------ ------- -------- ---- --- ------------------------------\n");

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr || *dsp_ptr->bus_enum != 0) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-7s %-8s %-4s %-3s %-30s\n", 
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->acpi_device,
						 dsp_ptr->acpi_type,
						 "",
						 dsp_ptr->acpi_scope);
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "\nPCI Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: (VENDOR(128)& DEVICE(64)& BS(32)& DV(16)& FN(8)& RV(4)& SC(2)& PI(1))\n"
					 "Minterms: 8\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION VENDOR DEVICE BS DV FN RV SC PI\n"
					 "-- ------------ ------- ------ ------ -- -- -- -- -- --\n");

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr || *dsp_ptr->bus_enum != 1) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-7s %-6s %-6s %2s %2s %2s %2s %2s %2s\n",
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->vendor_id,
						 dsp_ptr->device_id,
						 "",
						 "",
						 "",
						 "",
						 "",
						 "");
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "\nConjure Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: (GUID(1))\n"
					 "Minterms: 1\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION GUID\n"
					 "-- ------------ ------- ------------------------------------\n");

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr	/* || *dsp_ptr->bus_enum != 3 */) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-7s %-36s\n", 
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->type);
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "\nPlatform Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: (GUID(1))\n"
					 "Minterms: 1\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION GUID\n"
					 "-- ------------ ------- ------------------------------------\n");

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr || *dsp_ptr->bus_enum != 2) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-7s %-36s\n", 
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->type);
	}


	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	return output;
}


static char *esif_shell_cmd_conjures(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	u8 i = 0;
	
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nLoaded Conjures:\n\n"
					 "ID Name         Description                         Type   Version     \n"
					 "-- ------------ ----------------------------------- ------ ------------\n"

					 );
	/* Enumerate Conjures */
	for (i = 0; i < ESIF_MAX_CONJURES; i++) {
		EsifCnjPtr a_conjure_ptr    = &g_cnjMgr.fEnrtries[i];

		if (NULL == a_conjure_ptr->fLibNamePtr) {
			continue;
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, 
					"%02u %-12s %-35s %-6s %d\n", i,
						 a_conjure_ptr->fLibNamePtr,
						 (esif_string)a_conjure_ptr->fInterface.desc,
						 "plugin",
						 a_conjure_ptr->fInterface.cnjVersion);
	}
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	return output;
}

// Kernel Actions (Static/dynamic)
static char *esif_shell_cmd_actionsk(EsifShellCmdPtr shell)
{
	struct esif_ipc_command *command_ptr = NULL;
	u32 data_len = sizeof(struct esif_command_get_actions);
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	struct esif_command_get_actions *data_ptr = NULL;
	struct esif_action_info *act_ptr = NULL;
	u32 count = 0;
	char *output = shell->outbuf;
	u32 i = 0;

	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"esif_ipc_alloc_command failed for %u bytes\n",
			data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_ACTIONS;
	command_ptr->req_data_type = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len = 0;
	command_ptr->rsp_data_type = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len = data_len;

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"IPC error code = %s(%d)\n",
			esif_rc_str(ipc_ptr->return_code),
			ipc_ptr->return_code);
		goto exit;
	}


	/*
	* If the buffer is too small, reallocate based on the number of actions that are present and retry
	*/
	if (ESIF_E_NEED_LARGER_BUFFER == command_ptr->return_code) {
		data_ptr = (struct esif_command_get_actions *)(command_ptr + 1);
		count = data_ptr->available_count;
		if (0 == count) {
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"Invalid action count (0) returned for ESIF_E_NEED_LARGER_BUFFER\n");
			goto exit;
		}
		data_len = sizeof(struct esif_command_get_actions) + ((count - 1) * sizeof(*data_ptr->action_info));

		esif_ipc_free(ipc_ptr);

		ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

		if (NULL == ipc_ptr || NULL == command_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN,
				output,
				"esif_ipc_alloc_command failed for %u bytes\n",
				data_len);
			goto exit;
		}

		command_ptr->type = ESIF_COMMAND_TYPE_GET_ACTIONS;
		command_ptr->req_data_type = ESIF_DATA_VOID;
		command_ptr->req_data_offset = 0;
		command_ptr->req_data_len = 0;
		command_ptr->rsp_data_type = ESIF_DATA_STRUCTURE;
		command_ptr->rsp_data_offset = 0;
		command_ptr->rsp_data_len = data_len;

		ipc_execute(ipc_ptr);
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"ipc error code = %s(%d)\n",
			esif_rc_str(ipc_ptr->return_code),
			ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"Command error code = %s(%d)\n",
			esif_rc_str(command_ptr->return_code),
			command_ptr->return_code);
		goto exit;
	}

	// out data
	data_ptr = (struct esif_command_get_actions *)(command_ptr + 1);
	count = data_ptr->returned_count;

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(
			OUT_BUF_LEN,
			output,
			"\nKERNEL ACTIONS:\n\n"
			"ID Name                                Type   \n"
			"-- ----------------------------------- -------\n");
	}
	else {// XML
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<actionsk>\n");
	}

	for (i = 0; i < count; i++) {
		act_ptr = &data_ptr->action_info[i];
		char *type_ptr = "dynamic";

		if (ESIF_FALSE == act_ptr->dynamic_action){
			type_ptr = "static";
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN,
				output,
				"%-2d %-35s %s\n",
				act_ptr->action_type,
				esif_action_type_str(act_ptr->action_type),
				type_ptr);
		}
		else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"<action>\n"
				"  <id>%d</id>\n"
				"  <name>%s</name>\n"
				"  <type>%s</type>\n"
				"</action>\n",
				act_ptr->action_type,
				esif_action_type_str(act_ptr->action_type),
				type_ptr);
		}
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	}
	else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</actionsk>\n");
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


static char *esif_shell_cmd_actionsu(EsifShellCmdPtr shell)
{
	char *output = shell->outbuf;
	eEsifError iterRc = ESIF_OK;
	ActMgrIterator actIter = {0};
	EsifActPtr curActPtr = NULL;

	iterRc = EsifActMgr_InitIterator(&actIter);
	if (iterRc != ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error getting data\n");
		goto exit;
	}

	if (g_format == FORMAT_XML) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<result>\n");

		iterRc = EsifActMgr_GetNexAction(&actIter, &curActPtr);
		while (ESIF_OK == iterRc) {
			if (curActPtr != NULL) {
				char *type_ptr = "dynamic";

				if (!EsifAct_IsPlugin(curActPtr)) {
					type_ptr = "static";
				}

				esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
								 "<action>\n"
								 "    <id>%u</id>\n"
								 "    <name>%s</name>\n"
								 "    <type>%s</type>\n"
								 "  </action>\n",
								 EsifAct_GetType(curActPtr),
								 EsifAct_GetName(curActPtr),
								 type_ptr);
			}
			iterRc = EsifActMgr_GetNexAction(&actIter, &curActPtr);
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</result>\n");
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nUSER ACTIONS:\n\n"
					 "ID Name         Description                         Type    Version\n"
					 "-- ------------ ----------------------------------- ------- -------\n"
					 );

	iterRc = EsifActMgr_GetNexAction(&actIter, &curActPtr);
	while (ESIF_OK == iterRc) {
		if (curActPtr != NULL) {
			char *type_ptr = "dynamic";

			if (!EsifAct_IsPlugin(curActPtr)) {
				type_ptr = "static";
			}
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-35s %-7s %7u\n",
							 EsifAct_GetType(curActPtr),
							 EsifAct_GetName(curActPtr),
							 EsifAct_GetDesc(curActPtr),
							 type_ptr,
							 EsifAct_GetVersion(curActPtr));
		}
		iterRc = EsifActMgr_GetNexAction(&actIter, &curActPtr);
	}
exit:
	EsifAct_PutRef(curActPtr);
	return output;
}


static char *esif_shell_cmd_actions(EsifShellCmdPtr shell)
{
	char *actionskString = NULL;
	char *output = shell->outbuf;

	// Get and save the actionsk output
	esif_shell_exec_dispatch("actionsk", output);
	actionskString = esif_ccb_strdup(output);
		
	output = esif_shell_exec_dispatch("actionsu", output);
	if (g_format == FORMAT_XML) {
		goto exit;
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, actionskString);

exit:
	esif_ccb_free(actionskString);
	return output;
}


static char *esif_shell_cmd_upes(EsifShellCmdPtr shell)
{
	char *output = shell->outbuf;
	eEsifError iterRc = ESIF_OK;
	ActMgrIterator actIter = {0};
	EsifActPtr curActPtr = NULL;

	iterRc = EsifActMgr_InitIterator(&actIter);
	if (iterRc != ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error getting data\n");
		goto exit;
	}

	if (g_format == FORMAT_XML) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<result>\n");

		iterRc = EsifActMgr_GetNexAction(&actIter, &curActPtr);
		while (ESIF_OK == iterRc) {
			if ((curActPtr != NULL) && EsifAct_IsPlugin(curActPtr)) {

				esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
								 "<action>\n"
								 "    <id>%u</id>\n"
								 "    <action_type_str>%s</action_type_str>\n"
								 "    <name>%s</name>\n"
								 "    <desc>%s</desc>\n"
								 "    <version>%u</version>\n"
								 "</action>\n",
								 EsifAct_GetType(curActPtr),
								 esif_action_type_str(EsifAct_GetType(curActPtr)),
								 EsifAct_GetName(curActPtr),
								 EsifAct_GetDesc(curActPtr),
								 EsifAct_GetVersion(curActPtr));
			}
			iterRc = EsifActMgr_GetNexAction(&actIter, &curActPtr);
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</result>\n");
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nUSER PARTICIPANT EXTENSIONS:\n\n"
					 "ID Name         Description                         Version\n"
					 "-- ------------ ----------------------------------- -------\n"
					 );

	iterRc = EsifActMgr_GetNexAction(&actIter, &curActPtr);
	while (ESIF_OK == iterRc) {
			if ((curActPtr != NULL) && EsifAct_IsPlugin(curActPtr)) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-35s %7u\n",
							 EsifAct_GetType(curActPtr),
							 EsifAct_GetName(curActPtr),
							 EsifAct_GetDesc(curActPtr),
							 EsifAct_GetVersion(curActPtr));
		}
		iterRc = EsifActMgr_GetNexAction(&actIter, &curActPtr);
	}
exit:
	EsifAct_PutRef(curActPtr);
	return output;
}


static char *esif_shell_cmd_apps(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	u8 i = 0;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nRUNNING APPLICATIONS:\n\n"
					 "ID Name         Description                         Type   Version      Events    \n"
					 "-- ------------ ----------------------------------- ------ ------------ ----------\n");
	/* Enumerate Applications */
	for (i = 0; i < ESIF_MAX_APPS; i++) {
		u8 j = 0;
		EsifAppPtr a_app_ptr = &g_appMgr.fEntries[i];
		void* appHandle = NULL;
		char desc[ESIF_DESC_LEN];
		char version[ESIF_DESC_LEN];

		ESIF_DATA(data_desc, ESIF_DATA_STRING, desc, ESIF_DESC_LEN);
		ESIF_DATA(data_version, ESIF_DATA_STRING, version, ESIF_DESC_LEN);

		if (NULL == a_app_ptr->fLibNamePtr) {
			continue;
		}

		appHandle = a_app_ptr->fHandle;
		a_app_ptr->fInterface.fAppGetDescriptionFuncPtr(&data_desc);
		a_app_ptr->fInterface.fAppGetVersionFuncPtr(&data_version);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-35s %-6s %-12s 0x%016llx\n", i,
						 a_app_ptr->fLibNamePtr,
						 (esif_string)data_desc.buf_ptr,
						 "plugin",
						 (esif_string)data_version.buf_ptr,
						 EsifEventMgr_GetEventMask(appHandle, 0, EVENT_MGR_DOMAIN_D0));

		/* Now temproary dump particiapnt events here */
		for (j = 0; j < MAX_PARTICIPANT_ENTRY; j++) {
			u8 k = 0;
			if (a_app_ptr->fParticipantData[j].fAppParticipantHandle == NULL) {
				continue;
			}
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s = 0x%016llx Participant Registered Events\n", 
							 EsifUp_GetName(a_app_ptr->fParticipantData[j].fUpPtr),
							 EsifEventMgr_GetEventMask(appHandle, j, EVENT_MGR_DOMAIN_D0));

			for (k = 0; k < MAX_DOMAIN_ENTRY; k++) {
				if (a_app_ptr->fParticipantData[j].fDomainData[k].fAppDomainHandle == NULL) {
					continue;
				}
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  D%d = 0x%016llx Domain Registered Events\n", 
								 k,
								 EsifEventMgr_GetEventMask(appHandle,
									j,
									domain_str_to_short(a_app_ptr->fParticipantData[j].fDomainData[k].fQualifier)));
			}
		}
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %s %s    0xffffffffffffffff\n",
					 g_appMgr.fEntryCount,
					 "esif         ESIF Shell                          static",
					 g_esif_shell_version);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", 
					 "\nHint To manage / connnect use appselect name e.g. appselect esif\n\n");
	return output;
}


static char *esif_shell_cmd_appstart(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *libName  = 0;
	EsifAppPtr a_app_ptr = NULL;
	u8 i = 0;

	/* Parse The Library Nme */
	if (argc < 2) {
		return NULL;
	}
	libName = argv[1];

	/* Check to see if the app is already running only one instance per app allowed */
	if (NULL != g_appMgr.GetAppFromName(&g_appMgr, libName)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Application %s Already Started.\n", libName);
		goto exit;
	}

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		if (NULL == g_appMgr.fEntries[i].fLibNamePtr) {
			break;
		}
	}

	if (ESIF_MAX_APPS == i) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Maximum Applicaitons Reached %u.\n", i);
		goto exit;
	}

	esif_ccb_write_lock(&g_appMgr.fLock);
	g_appMgr.fEntryCount++;
	esif_ccb_sprintf(OUT_BUF_LEN, output, "Application Sandbox %u Selected.\n", i);

	a_app_ptr = &g_appMgr.fEntries[i];
	a_app_ptr->fLibNamePtr = (esif_string)esif_ccb_strdup(libName);
	rc = EsifAppStart(a_app_ptr);

	if (ESIF_OK != rc) {
		/* Cleanup */
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Start App: %s [%s (%d)].\n", libName, esif_rc_str(rc), rc);
		esif_ccb_free(a_app_ptr->fLibNamePtr);
		memset(a_app_ptr, 0, sizeof(*a_app_ptr));
	} else {
		/* Proceed */
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Started App: %s Instance %u Max %u Running %u\n\n",
						 libName, i, ESIF_MAX_APPS, g_appMgr.fEntryCount);
	}
	esif_ccb_write_unlock(&g_appMgr.fLock);

exit:
	return output;
}


static char *esif_shell_cmd_appstop(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *libName  = 0;
	EsifAppPtr a_app_ptr = NULL;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}
	libName  = argv[1];

	esif_ccb_write_lock(&g_appMgr.fLock);

	a_app_ptr = g_appMgr.GetAppFromName(&g_appMgr, libName);
	if (NULL == a_app_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find App: %s.\n", libName);
		goto exit;
	}

	rc = EsifAppStop(a_app_ptr);
	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Stop App: %s.\n", libName);
	} else {
		/* Proceed */
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Stopped App: %s\n\n", libName);
		g_appMgr.fEntryCount--;
	}
exit:
	esif_ccb_write_unlock(&g_appMgr.fLock);
	return output;
}


static char *esif_shell_cmd_actionstart(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	char *libName  = 0;
	eEsifError rc = ESIF_OK;

	if (argc < 2) {
		return NULL;
	}

	/* Parse The Library Name */
	libName = argv[1];

	rc = EsifActMgr_StartUpe(libName);

	switch(rc){
	case ESIF_OK:
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Action %s started\n\n", libName);
		break;
	case ESIF_E_ACTION_ALREADY_STARTED:
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Action %s already started\n\n", libName);
		break;
	default:
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Failed to start action %s, error = %s(%d)\n\n", libName, esif_rc_str(rc), rc);
		break;
	}
	return output;
}


static char *esif_shell_cmd_actionstop(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	eEsifError rc = ESIF_OK;
	char *libName  = 0;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}
	libName = argv[1];

	rc = EsifActMgr_StopUpe(libName);

	switch(rc){
	case ESIF_OK:
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Action %s stopped\n\n", libName);
		break;
	case ESIF_E_ACTION_NOT_IMPLEMENTED:
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Action %s not found\n\n", libName);
		break;
	default:
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Failed to stop action %s, error = %s(%d)\n\n", libName, esif_rc_str(rc), rc);
		break;
	}
	return output;
}


static char *esif_shell_cmd_appselect(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *libName  = 0;

	EsifAppPtr a_app_ptr = NULL;
	struct esif_data data_banner;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}
	libName = argv[1];

	/* Works But Ugly */
	if (!strcmp(libName, "esif")) {
		g_appMgr.fSelectedAppPtr = NULL;
		esif_init();
		goto exit;
	}

	a_app_ptr = g_appMgr.GetAppFromName(&g_appMgr, libName);
	if (NULL == a_app_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find App: %s.\n", libName);
		goto exit;
	}

	data_banner.type     = ESIF_DATA_STRING;
	data_banner.buf_ptr  = esif_ccb_malloc(1024);
	data_banner.buf_len  = 1024;
	data_banner.data_len = 0;

	rc = a_app_ptr->fInterface.fAppGetBannerFuncPtr(a_app_ptr->fHandle, &data_banner);
	if (ESIF_OK != rc) {
		goto exit;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s", (esif_string)data_banner.buf_ptr);
	esif_ccb_free(data_banner.buf_ptr);

	g_appMgr.fSelectedAppPtr = a_app_ptr;

exit:
	return output;
}


// Execute Default Start Script, if any
static char *esif_shell_cmd_autoexec(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	int optarg = 1;
	char *command = NULL;

	// autoexec ["shell command"] [...]
	while (optarg < argc) {
		parse_cmd(argv[optarg++], ESIF_FALSE);
	}
	if ((g_DataVaultStartScript != NULL) && ((command = esif_ccb_strdup(g_DataVaultStartScript)) != NULL)) {
		parse_cmd(command, ESIF_FALSE);
		esif_ccb_free(command);
	}
	output = NULL;
	return output;
}

// Stop On Error
static char *esif_shell_cmd_soe(EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char **argv   = shell->argv;
	char *output  = shell->outbuf;
	char *soe_str = 0;

	if (argc < 2) {
		return NULL;
	}
	soe_str = argv[1];

	if (!strcmp(soe_str, "on")) {
		g_soe = 1;
	} else {
		g_soe = 0;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "stop on error = %d\n", g_soe);
	return output;
}


// Debug Level
static char *esif_shell_cmd_debuglvl(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc *ipc_ptr = NULL;
	struct esif_command_set_debug_module_level data;
	const u32 data_len = sizeof(struct esif_command_set_debug_module_level);
	struct esif_ipc_command *command_ptr = NULL;

	if (argc < 2) {
		return NULL;
	}

	// debuglvl <tracelevel>
	if (argc < 3) {
		data.module = (u32)(-1);
		data.level  = esif_atoi(argv[1]);
		if (data.level > ESIF_TRACELEVEL_DEBUG) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "invalid tracelevel\n");
			return NULL;
		}
	}
	// debuglvl <module> <categorymask>
	else {
		data.module = esif_atoi(argv[1]);
		data.level  = esif_atoi(argv[2]);
	}

	ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_SET_DEBUG_MODULE_LEVEL;
	command_ptr->req_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = data_len;
	command_ptr->rsp_data_type   = ESIF_DATA_VOID;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = 0;

	// Execute Command multiple times for "debuglvl -1 <categorymask>"
	if (argc >= 3 && data.module == (u32)(-1)) {
		u32 mod=0;
		for (mod = 0; mod < ESIF_DEBUG_MOD_MAX; mod++) {
			data.module = mod;
			esif_ccb_memcpy((command_ptr + 1), &data, data_len);
			ipc_execute(ipc_ptr);
		}
		data.module = (u32)(-1);
	}
	else {
		esif_ccb_memcpy((command_ptr + 1), &data, data_len);
		ipc_execute(ipc_ptr);
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	if (argc < 3)
		esif_ccb_sprintf(OUT_BUF_LEN, output, "kernel tracelevel = %d\n", data.level);
	else
		esif_ccb_sprintf(OUT_BUF_LEN, output, "kernel module = %d, category = 0x%08X\n", data.module, data.level);

exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// Debug Set
static char *esif_shell_cmd_debugset(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc *ipc_ptr = NULL;
	struct esif_command_debug_modules data;
	const u32 data_len = sizeof(struct esif_command_debug_modules);
	struct esif_ipc_command *command_ptr = NULL;

	if (argc < 2) {
		return NULL;
	}
	data.modules = esif_atoi(argv[1]);

	ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_SET_DEBUG_MODULES;
	command_ptr->req_data_type   = ESIF_DATA_UINT32;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = data_len;
	command_ptr->rsp_data_type   = ESIF_DATA_VOID;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = 0;

	// Command
	esif_ccb_memcpy((command_ptr + 1), &data, data_len);
	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), command_ptr->return_code);
		goto exit;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "modules = 0x%08X\n", data.modules);
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}

// Select Destination By ID
char *esif_shell_cmd_dst(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_dst = (u8)esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "destination participant = %d selected\n", g_dst);
	return output;
}


// Select Destination By Name
char *esif_shell_cmd_dstn(EsifShellCmdPtr shell)
{
	int argc         = shell->argc;
	char **argv      = shell->argv;
	char *output     = shell->outbuf;
	char *name_str   = 0;
	EsifUpPtr upPtr = NULL;

	if (argc < 2) {
		return NULL;
	}
	name_str = argv[1];

	upPtr   = EsifUpPm_GetAvailableParticipantByName(name_str);

	if (upPtr != NULL) {
		g_dst = EsifUp_GetInstance(upPtr);

		esif_ccb_sprintf(OUT_BUF_LEN, output, "destination %s participant = %d selected\n", name_str, g_dst);

		EsifUp_PutRef(upPtr);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "destination %s participant = %d not found\n", name_str, g_dst);
	}

	return output;
}


// Echo
static char *esif_shell_cmd_echo(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *sep    = " ";
	int j = 1;

	// "echo ? arg arg ..." causes each argument to appear on separate line
	if (argc > 1 && strcmp(argv[1], "?") == 0) {
		sep = "\n";
		j++;
	}
	for ( ; j < argc; j++) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s%s", argv[j], (j + 1 < argc ? sep : "\n"));
	}

	return output;
}


// Format
static char *esif_shell_cmd_format(EsifShellCmdPtr shell)
{
	int argc         = shell->argc;
	char **argv      = shell->argv;
	char *output     = shell->outbuf;
	char *format_str = 0;

	if (argc < 2) {
		return NULL;
	}

	format_str = argv[1];

	if (!strcmp(format_str, "xml")) {
		g_format = FORMAT_XML;
	} else {
		g_format = FORMAT_TEXT;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "format requested=%s format=%d\n", format_str, g_format);
	return output;
}


static void dump_binary_object(u8 *byte_ptr, u32 byte_len);
static void dump_table_hdr(struct esif_table_hdr *tab);
static void dump_binary_data(u8 *byte_ptr, u32 byte_len);

// Get From File
static char *esif_shell_cmd_getf(EsifShellCmdPtr shell)
{
	int argc         = shell->argc;
	char **argv      = shell->argv;
	char *output     = shell->outbuf;
	char *filename   = 0;
	char full_path[PATH_STR_LEN] = {0};
	FILE *fp_ptr     = NULL;
	size_t file_size = 0;
	size_t file_read = 0;
	u8 *buf_ptr      = NULL;
	char *suffix     = &argv[0][4];	// _b or _bd
	int dump         = 0;

	UNREFERENCED_PARAMETER(suffix);
	if (argv[0][ESIF_SHELL_STRLEN(argv[0]) - 1] == 'd') {	// getf_bd
		dump = 1;
	}

	if (argc < 2) {
		return NULL;
	}
	filename = argv[1];
	esif_build_path(full_path, sizeof(full_path), ESIF_PATHTYPE_BIN, filename, NULL);

	fp_ptr = esif_ccb_fopen(full_path, (char *)FILE_READ, NULL);
	if (NULL != fp_ptr) {
		struct stat file_stat = {0};
		esif_ccb_stat(full_path, &file_stat);
		file_size = file_stat.st_size;
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: file not found (%s)\n", ESIF_FUNC, full_path);
		goto exit;
	}

	// allocate space
	buf_ptr = (u8 *)esif_ccb_malloc(file_size);
	if (NULL == buf_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: malloc failed to allocate %u bytes\n",
						 ESIF_FUNC, (u32)file_size);
		goto exit;
	}

	// read file contents
	file_read = fread(buf_ptr, 1, file_size, fp_ptr);
	if (file_read < file_size) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: read short received %u of %u bytes\n",
						 ESIF_FUNC, (int)file_read, (u32)file_size);
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "BIN Dump/Decode For %s Size %u:\n", full_path, (u32)file_size);

	if (0 == dump) {
		dump_binary_object(buf_ptr, (u32)file_size);
	} else {
		dump_binary_data(buf_ptr, (u32)file_size);
	}

exit:
	if (NULL != fp_ptr) {
		fclose(fp_ptr);
	}

	if (NULL != buf_ptr) {
		esif_ccb_free(buf_ptr);
	}
	return output;
}


// Get Primitive
static char *esif_shell_cmd_getp(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;

	int optargc     = 0;	// Simulates Command Line Argument Count
	char *optargv[MAX_ARGV];// Simulates Command Line Arguments
	int opt     = 1;

	u32 id      = 0;
	char *qualifier_str = "D0";
	u8 instance = 255;
	char full_path[PATH_STR_LEN]={0};
	char desc[32];
	u8 *data_ptr  = NULL;
	u16 qualifier = EVENT_MGR_DOMAIN_D0;

	struct esif_data request = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data response;

	char *suffix = "";
	enum esif_data_type type = ESIF_DATA_VOID;
	u32 buf_size = 0;
	int dump     = 0;
	u32 add_data;

	if (argc <= opt) {
		return NULL;
	}

	// Deduce suffix, data type, and dump options based on command
	if (esif_ccb_stricmp(argv[0], "getp") == 0) {
		suffix   = "";
		type     = ESIF_DATA_AUTO;
		buf_size = ESIF_DATA_ALLOCATE;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_u32") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_UINT32;
		buf_size = 4;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_t") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_TEMPERATURE;
		buf_size = 4;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_pw") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_POWER;
		buf_size = 4;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_s") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_STRING;
		buf_size = 128;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_t") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_UINT32;
		buf_size = g_binary_buf_size;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_b") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_BINARY;
		buf_size = g_binary_buf_size;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_bd") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_BINARY;
		buf_size = g_binary_buf_size;
		dump     = 1;
	} else if (esif_ccb_stricmp(argv[0], "getp_bs") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_BINARY;
		buf_size = g_binary_buf_size;
		dump     = 2;
	}

	// Primitive ID
	id = esif_atoi(argv[opt++]);

	// Qualifier
	if (opt < argc) {
		qualifier_str = argv[opt++];
	}
	qualifier = domain_str_to_short(qualifier_str);

	// Instance ID
	if (opt < argc) {
		instance = (u8)esif_atoi(argv[opt++]);
	}

	// Optional, Additional Argument, Integer Only For Now
	if (opt < argc && isdigit(argv[opt][0])) {
		add_data        = esif_atoi(argv[opt++]);

		request.type    = ESIF_DATA_UINT32;
		request.buf_len = sizeof(add_data);
		request.buf_ptr = (void *)&add_data;
	}

	// For File Dump
	if (2 == dump) {
		char *filename = 0;
		if (argc <= opt) {
			return NULL;
		}
		filename = argv[opt++];
		esif_build_path(full_path, sizeof(full_path), ESIF_PATHTYPE_BIN, filename, ".bin");
	}

	// Create a pseudo argc/argv to pass to Test functions
	esif_ccb_memcpy(&optargv[0], &argv[opt], sizeof(char *) * (argc - opt));
	optargc += argc - opt;

	// Setup Response
	response.type = type;
	if (ESIF_DATA_ALLOCATE != buf_size) {
		data_ptr = (u8 *)esif_ccb_malloc(buf_size);
		if (NULL == data_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "esif_ccb_malloc failed for %u bytes\n", buf_size);
			goto exit;
		}
		response.buf_ptr  = (void *)data_ptr;
		response.buf_len  = buf_size;
		response.data_len = 0;
	} else {
		response.buf_len  = ESIF_DATA_ALLOCATE;
		response.buf_ptr  = NULL;
		response.data_len = 0;
	}

	// Verify this is a GET
	if (EsifPrimitiveVerifyOpcode(
			(u8)g_dst,
			id,
			qualifier_str,
			instance,
			ESIF_PRIMITIVE_OP_GET) == ESIF_FALSE) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Primitive (%d) not a GET: %s\n", id, esif_primitive_str(id));
		rc = ESIF_E_INVALID_REQUEST_TYPE;
		goto exit;
	}

	rc = EsifExecutePrimitive(
			(u8)g_dst,
			id,
			qualifier_str,
			instance,
			&request,
			&response);

	data_ptr = (u8 *)response.buf_ptr;

	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s getp%s(%03u.%s.%03d)",
					 esif_primitive_str((enum esif_primitive_type)id), suffix, id, qualifier_str, instance);

	if (ESIF_E_NEED_LARGER_BUFFER == rc) {
		//
		// PRIMITIVE Error Code NEED Larger Buffer
		//
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 " error code = %s(%d) HAVE %u bytes NEED ATLEAST %u bytes\n",
						 esif_rc_str(rc),
						 rc,
						 response.buf_len,
						 response.data_len);
		g_errorlevel = -(ESIF_E_NEED_LARGER_BUFFER);
		goto exit;
	} else if (ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT == rc) {
		//
		// Not An Error Just A Warning
		//
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " info code = %s(%d)\n",
						 esif_rc_str(rc), rc);

		goto exit;
	} else if (ESIF_OK != rc) {
		//
		// PRIMITIVE Error Code
		//
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " error code = %s(%d)\n", esif_rc_str(rc), rc);
		g_errorlevel = -(rc);
		goto exit;
	}

	//
	// WE have data so now process and format for output
	//

	/* Assign Respond Data Type Description */
	type = response.type;
	switch (type) {
	case ESIF_DATA_UINT32:
	case ESIF_DATA_UINT64:
		esif_ccb_strcpy(&desc[0], "", sizeof(desc));
		break;

	case ESIF_DATA_TEMPERATURE:
		esif_ccb_strcpy(&desc[0], "Degrees C", sizeof(desc));
		break;

	case ESIF_DATA_POWER:
		esif_ccb_strcpy(&desc[0], "MilliWatts", sizeof(desc));
		break;

	case ESIF_DATA_STRING:
		esif_ccb_strcpy(&desc[0], "ASCII", sizeof(desc));
		break;

	case ESIF_DATA_BINARY:
		esif_ccb_strcpy(&desc[0], "BINARY", sizeof(desc));
		break;

	case ESIF_DATA_TABLE:
		esif_ccb_strcpy(&desc[0], "TABLE", sizeof(desc));
		break;

	case ESIF_DATA_PERCENT:
		esif_ccb_strcpy(&desc[0], "Centi-Percent", sizeof(desc));
		break;

	case ESIF_DATA_FREQUENCY:
		esif_ccb_strcpy(&desc[0], "Frequency", sizeof(desc));
		break;

	case ESIF_DATA_TIME:
		esif_ccb_strcpy(&desc[0], "Time (ms)", sizeof(desc));
		break;

	default:
		esif_ccb_strcpy(&desc[0], "", sizeof(desc));
		break;
	}

	if ((ESIF_DATA_UINT32 == type) ||
		(ESIF_DATA_TEMPERATURE == type) ||
		(ESIF_DATA_POWER == type) ||
		(ESIF_DATA_TIME == type) ||
		(ESIF_DATA_PERCENT == type)) {
		// Our Data
		u32 val = *(u32 *)(response.buf_ptr);
		if (optargc > 1) {
			// Have Testable Data
			eEsifTestErrorType rc = EsifTestPrimitive(id, qualifier, instance, val, optargc, optargv);

			if (ESIF_TEST_OK == rc) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " value = %u(0x%08x): %s\n",
								 val, val, ESIF_TEST_PASSED);
			} else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " value = %u(0x%08x): %s %s(%d)\n",
								 val, val, ESIF_TEST_FAILED, EsifTestErrorStr(rc), rc);
				g_errorlevel = rc;
			}
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		} else {
			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " value = 0x%08x %u %s\n",
								 val, val, desc);
			} else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
								 "<result>\n"
								 "    <value>%u</value>\n"
								 "    <valueDesc>%s</valueDesc>\n",
								 val, desc);
			}
		}
	} else if (ESIF_DATA_STRING == type) {
		char *str_ptr = (char *)(response.buf_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " string(%u of %u) = %s\n",
						 response.data_len,
						 response.buf_len,
						 str_ptr);
	} else if (ESIF_DATA_UINT64 == type ||
			   ESIF_DATA_FREQUENCY == type) {
		u64 val = *(u64 *)(response.buf_ptr);
		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " value = 0x%016llx %llu %s\n",
							 val, val, desc);
		} else {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "<result>\n"
							 "    <value>%llu</value>\n"
							 "    <valueDesc>%s</valueDesc>\n",
							 val, desc);
		}
	} else {// Binary Dump
		u8 *byte_ptr = (u8 *)(response.buf_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " Binary Data(%u of %u):",
						 response.data_len,
						 response.buf_len);

		// Have Testable Data
		if (optargc > 1) {
			eEsifTestErrorType rc = EsifTestPrimitiveBinary(id, qualifier, instance, byte_ptr,
															(u16)response.data_len, optargc, optargv);

			if (ESIF_TEST_OK == rc) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " %s\n", ESIF_TEST_PASSED);
			} else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " %s %s(%d)\n", ESIF_TEST_FAILED, EsifTestErrorStr(rc), rc);
				g_errorlevel = rc;
			}
		} else {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " %s\n", ESIF_TEST_SKIPPED);
		}

		switch (dump) {
		case 0:
			if (ESIF_DATA_TABLE == type) {
				dump_table_hdr((struct esif_table_hdr *)byte_ptr);
			} else {
				dump_binary_object(byte_ptr, response.data_len);
			}
			break;
		case 1:
			dump_binary_data(byte_ptr, response.data_len);
			break;
		case 2:
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " Binary Data To File: %s (%u of %u):\n",
							 full_path,
							 response.data_len,
							 response.buf_len);
			FILE *fp_ptr = NULL;

			if ((fp_ptr = esif_ccb_fopen(full_path, (char *)"wb", NULL)) != NULL) {
				fwrite(byte_ptr, 1, response.data_len, fp_ptr);
				fclose(fp_ptr);
			}
			break;
		default:
			break;
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	}

	if (FORMAT_XML == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</result>\n");
	}
exit:
	esif_ccb_free(data_ptr);
	return output;
}

static char *esif_shell_cmd_ufpoll(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	int pollInterval = 0;
	char **argv = shell->argv;
	char *output = shell->outbuf;

	// ufpoll [status]
	if (argc < 2 || esif_ccb_stricmp(argv[1], "status") == 0) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Upper framework polling is: %s\n", (EsifUFPollStarted() ? "started" : "stopped"));
	}
	// ufpoll start
	else if (esif_ccb_stricmp(argv[1], "start") == 0) {
		if (argc > 2) {
			if ((int) esif_atoi(argv[2]) >= ESIF_UFPOLL_PERIOD_MIN) {
				pollInterval = (esif_ccb_time_t) esif_atoi(argv[2]);
			}
			else {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid polling period specified (minimum is %d ms).\n", ESIF_UFPOLL_PERIOD_MIN);
				goto exit;
			}
		}
		/* Calling start when it is already started is allowed...it will update the poll period */
		EsifUFPollStart(pollInterval);
	}
	// ufpoll stop
	else if (esif_ccb_stricmp(argv[1], "stop") == 0) {
			EsifUFPollStop();
	}
exit:
	return output;
}

// FPC Info
static char *esif_shell_cmd_infofpc(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char cpc_domain_str[8] = "";
	char *cpc_pattern = "";
	char *edp_filename     = 0;
	char edp_name[PATH_STR_LEN]={0};
	char edp_full_path[PATH_STR_LEN]={0};
	EsifFpcPtr fpcPtr = NULL;
	EsifFpcDomainPtr domainPtr;
	EsifFpcPrimitivePtr primitivePtr;
	EsifFpcActionPtr fpcActionPtr;
	EsifFpcAlgorithmPtr algorithmPtr;
	EsifFpcEventPtr eventPtr;
	u32 fpc_size = 0;
	u8 *base_ptr = NULL;
	u32 num_prim = 0, i, j, k;
	IOStreamPtr io_ptr    = IOStream_Create();
	EsifDataPtr nameSpace = 0;
	EsifDataPtr key = 0;
	EsifDataPtr value     = 0;
	struct edp_dir edp_dir;
	size_t r_bytes;
	size_t fpc_read;
	u32 offset; /* For debug only, not used for functional code */
	u32 edp_size;


	if (argc < 2 || io_ptr == NULL) {
		IOStream_Destroy(io_ptr);
		return NULL;
	}

	// parse filename no extension
	edp_filename = argv[1];

	// parse option regular expression
	if (argc > 2) {
		cpc_pattern = argv[2];
	}

	// Look for EDP file either on disk or in a DataVault (static or file), depending on priority setting
	esif_ccb_sprintf(PATH_STR_LEN, edp_name, "%s.edp", edp_filename);
	nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, ESIF_DSP_NAMESPACE, 0, ESIFAUTOLEN);
	key       = EsifData_CreateAs(ESIF_DATA_STRING, edp_name, 0, ESIFAUTOLEN);
	value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

	if (nameSpace == NULL || key == NULL || value == NULL) {
		goto exit;
	}
	esif_build_path(edp_full_path, sizeof(edp_full_path), ESIF_PATHTYPE_DSP, edp_name, NULL);

	if (!esif_ccb_file_exists(edp_full_path) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		esif_ccb_strcpy(edp_full_path, edp_name, PATH_STR_LEN);
		IOStream_SetMemory(io_ptr, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(io_ptr, edp_full_path, (char *)FILE_READ);
	}

	if (IOStream_Open(io_ptr) == 0) {
		edp_size = (UInt32)IOStream_GetSize(io_ptr);
		r_bytes  = IOStream_Read(io_ptr, &edp_dir, sizeof(struct edp_dir));
		if (!esif_verify_edp(&edp_dir, r_bytes)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid EDP Header: Signature=%4.4s Version=%d\n", (char *)&edp_dir.signature, edp_dir.version);
			goto exit;
		}
		fpc_size = edp_size - edp_dir.fpc_offset;
		if (fpc_size > MAX_CPC_SIZE) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: fpc_size too large (%u), edp_size %d\n",
							 ESIF_FUNC, fpc_size, edp_size);
			goto exit;
		}
		IOStream_Seek(io_ptr, edp_dir.fpc_offset, SEEK_SET);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: file not found (%s)\n", ESIF_FUNC, edp_full_path);
		goto exit;
	}

	// Find FPC and read out
	IOStream_Seek(io_ptr, edp_dir.fpc_offset, SEEK_SET);

	// allocate space for our FPC file contents
	fpcPtr = (EsifFpcPtr)esif_ccb_malloc(fpc_size);
	if (NULL == fpcPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: FPC malloc failed to allocate %u bytes for fpc\n",
						 ESIF_FUNC, fpc_size);
		goto exit;
	}
	base_ptr = (u8 *)fpcPtr;

	// read FPC file contents
	fpc_read = IOStream_Read(io_ptr, fpcPtr, fpc_size);
	if (fpc_read < fpc_size) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: FPC read short received %u of %u bytes\n",
						 ESIF_FUNC, (int)fpc_read, fpc_size);
		goto exit;
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "FPC File Info For %s Size %u\n\n",
						 edp_filename, fpc_size);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "FPC Source:     %s\n"
						 "FPC Size:       %d\n"
						 "FPC Name:       %s\n"
						 "FPC Version:    %x.%x\n"
						 "FOC Desc:       %s\n"
						 "FPC Primitives: %d\n"
						 "FPC Algorithms: %d\n"
						 "FPC Domains:    %d\n",
						 (IOStream_GetType(io_ptr) == StreamMemory ? "DataVault" : "File"),
						 fpcPtr->size,
						 fpcPtr->header.name,
						 fpcPtr->header.ver_major, fpcPtr->header.ver_minor,
						 fpcPtr->header.description,
						 fpcPtr->number_of_domains,
						 fpcPtr->number_of_algorithms,
						 fpcPtr->number_of_events);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<fpcinfo>\n"
						 "  <size>%d<size>\n"
						 "  <fpcSource>%s\n"
						 "  <fpcSize>%d</fpcSize>\n"
						 "  <fpcName>%s</fpcName>\n"
						 "  <fpcVersion>%x.%x<fpcVersion>\n"
						 "  <fpcDesc>%s<fpcDesc>\n"
						 "  <fpcPrimitives>%d<fpcPrimitives>\n"
						 "  <fpcAlgorithms>%d<fpcAlgorithms>\n"
						 "  <fpcDomains>%d</fpcDomains>\n",
						 fpc_size,
						 (IOStream_GetType(io_ptr) == StreamMemory ? "DataVault" : "File"),
						 fpcPtr->size,
						 fpcPtr->header.name,
						 fpcPtr->header.ver_major, fpcPtr->header.ver_minor,
						 fpcPtr->header.description,
						 fpcPtr->number_of_domains,
						 fpcPtr->number_of_algorithms,
						 fpcPtr->number_of_events);
	}


	/* First Domain, Ok To Have Zero Primitive Of A Domain */
	domainPtr = (EsifFpcDomainPtr)(fpcPtr + 1);
	for (i = 0; i < esif_ccb_min(fpcPtr->number_of_domains, MAX_DOMAINS); i++) {
		int show = 0;
		char temp_buf[1024]    = "";
		char qualifier_str[64] = "";

		offset = (unsigned long)((u8 *)domainPtr - base_ptr);

		if (!strcmp(cpc_pattern, "")) {
			show = 1;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(1024, temp_buf,
							 "\nDOMAINS:\n\n"
							 "Name     Description          Domain   Priority Capability Type               \n"
							 "-------- -------------------- -------- -------- ---------- -----------------------------\n");

			esif_ccb_sprintf_concat(1024, temp_buf,
							 "%-8s %-20s %-8s %08x %08x   %s(%d)\n",
							 domainPtr->descriptor.name,
							 domainPtr->descriptor.description,
							 esif_primitive_domain_str(domainPtr->descriptor.domain, qualifier_str, 64),
							 domainPtr->descriptor.priority,
							 domainPtr->capability_for_domain.capability_flags,
							 esif_domain_type_str(domainPtr->descriptor.domainType),
							 domainPtr->descriptor.domainType);


			if (1 == show) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
			}
		} else {	// FORMAT_XML
			esif_ccb_sprintf(1024, temp_buf,
							 "<domain>%s</domain>\n"
							 "<domainSize>%u</domainSize>\n"
							 "<numPrimitive>%u</numPrimitive>\n"
							 "<numCapability>%d</numCapability>\n"
							 "<capability>%x</capability>\n",
							 domainPtr->descriptor.name,
							 domainPtr->size,
							 domainPtr->number_of_primitives,
							 domainPtr->capability_for_domain.number_of_capability_flags,
							 domainPtr->capability_for_domain.capability_flags);
			if (1 == show) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
			}
		}


		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\nCAPABILITY:\n\n");
		}


		/* Capability */
		for (j = 0; j < domainPtr->capability_for_domain.number_of_capability_flags; j++) {
			offset = (unsigned long)(((u8 *)&domainPtr->capability_for_domain) - base_ptr);

			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf(1024, temp_buf,
								 "Capability[%d] 0x%x\n",
								 j, domainPtr->capability_for_domain.capability_mask[j]);
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			} else {	// FORMAT_XML
				esif_ccb_sprintf(1024, temp_buf,
								 "<Capability>%x</Capability>\n",
								 domainPtr->capability_for_domain.capability_mask[j]);
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			}
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(1024, temp_buf, "\nPRIMITIVES AND ACTIONS:\n\n");
		}
		if (1 == show) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
		}

		/* First Primtive */
		primitivePtr = (EsifFpcPrimitivePtr)(domainPtr + 1);
		for (j = 0; j < domainPtr->number_of_primitives; j++, num_prim++) {
			offset = (unsigned long)(((u8 *)primitivePtr) - base_ptr);

			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf(1024, temp_buf,
								 "%03d.%s.%03d %s actions %d %s req %s rsp %s\n",
								 primitivePtr->tuple.id,
								 esif_primitive_domain_str(primitivePtr->tuple.domain, cpc_domain_str, 8),
								 primitivePtr->tuple.instance,
								 esif_primitive_opcode_str((enum esif_primitive_opcode)primitivePtr->operation),
								 primitivePtr->num_actions,
								 esif_primitive_str((enum esif_primitive_type)primitivePtr->tuple.id),
								 esif_data_type_str(primitivePtr->request_type),
								 esif_data_type_str(primitivePtr->result_type));

				// TODO REGEXP Here For Now Simple Pattern Match
				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				// Show Row?
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			} else {	// FORMAT_XML
				esif_ccb_sprintf(1024, temp_buf,
								 "    <primitive>\n"
								 "      <id>%d</id>\n"
								 "      <primitiveStr>%s</primitiveStr>\n"
								 "      <domain>%s</domain>\n"
								 "      <instance>%d</instance>\n"
								 "      <opcode>%d</opcode>\n"
								 "      <opcodeStr>%s</opcodeStr>\n"
								 "      <actionCount>%d</actionCount>\n"
								 "    </primitive>\n",
								 primitivePtr->tuple.id,
								 esif_primitive_str((enum esif_primitive_type)primitivePtr->tuple.id),
								 esif_primitive_domain_str(primitivePtr->tuple.domain, cpc_domain_str, 8),
								 primitivePtr->tuple.instance,
								 primitivePtr->operation,
								 esif_primitive_opcode_str((enum esif_primitive_opcode)primitivePtr->operation),
								 primitivePtr->num_actions);

				// TODO REGEXP Here For Now Simple Pattern Match
				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				// Show Row?
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			}

			/* First Action */
			fpcActionPtr = (EsifFpcActionPtr)(primitivePtr + 1);
			for (k = 0; k < primitivePtr->num_actions; k++) {
				offset = (unsigned long)(((u8 *)fpcActionPtr) - base_ptr);

				if (FORMAT_TEXT == g_format) {
					esif_ccb_sprintf(1024, temp_buf,
									 "  %s param_valid %1d:%1d:%1d:%1d:%1d param %x:%x:%x:%x:%x\n",
									 esif_action_type_str(fpcActionPtr->type),
									 fpcActionPtr->param_valid[0],
									 fpcActionPtr->param_valid[1],
									 fpcActionPtr->param_valid[2],
									 fpcActionPtr->param_valid[3],
									 fpcActionPtr->param_valid[4],
									 fpcActionPtr->param_offset[0],
									 fpcActionPtr->param_offset[1],
									 fpcActionPtr->param_offset[2],
									 fpcActionPtr->param_offset[3],
									 fpcActionPtr->param_offset[4]);

					// TODO REGEXP Here For Now Simple Pattern Match
					if (0 == show && strstr(temp_buf, cpc_pattern)) {
						show = 1;
					}
					if (1 == show) {
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
					}
				} else {// FORMAT_XML
					esif_ccb_sprintf(1024, temp_buf,
									 "   <action>\n"
									 "      <type>%d</type>\n"
									 "      <typeStr>%s</typeStr>\n"
									 "      <paramValid>%1d:%1d:%1d:%1d:%1d</paramValid>\n"
									 "      <param[0]>%x</param[0]>\n"
									 "      <param[1]>%x</param[1]>\n"
									 "      <param[2]>%x</param[2]>\n"
									 "      <param[3]>%x</param[3]>\n"
									 "      <param[4]>%x</param[4]>\n"
									 "   </action>\n",
									 fpcActionPtr->type,
									 esif_action_type_str(fpcActionPtr->type),
									 fpcActionPtr->param_valid[0],
									 fpcActionPtr->param_valid[1],
									 fpcActionPtr->param_valid[2],
									 fpcActionPtr->param_valid[3],
									 fpcActionPtr->param_valid[4],
									 fpcActionPtr->param_offset[0],
									 fpcActionPtr->param_offset[1],
									 fpcActionPtr->param_offset[2],
									 fpcActionPtr->param_offset[3],
									 fpcActionPtr->param_offset[4]);
					// TODO REGEXP Here For Now Simple Pattern Match
					if (0 == show && strstr(temp_buf, cpc_pattern)) {
						show = 1;
					}
					if (1 == show) {
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
					}
				}
				/* Next Action */
				fpcActionPtr = (EsifFpcActionPtr)((u8 *)fpcActionPtr + fpcActionPtr->size);
			}
			/* Next Primitive */
			primitivePtr = (EsifFpcPrimitivePtr)((u8 *)primitivePtr + primitivePtr->size);
		}
		/* Next Domain */
		domainPtr = (EsifFpcDomainPtr)((u8 *)domainPtr + domainPtr->size);
	}


	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "ALGORITHMS:\n\n"
						 "Action Type                tempxform         tc1      tc2      powerxform         timexform\n"

						 "------ ------------------- ----------------- -------- -------- ----------------- ----------------\n");
	}

	/* First Algorithm (Laid After The Last Domain) */
	algorithmPtr = (EsifFpcAlgorithmPtr)domainPtr;
	for (i = 0; i < fpcPtr->number_of_algorithms && fpcPtr->number_of_algorithms <= MAX_ALGORITHMS; i++) {
		offset = (unsigned long)((u8 *)algorithmPtr - base_ptr);

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%-2d     %-19s %-17s %08x %08x %-17s %-17s\n",
							 algorithmPtr->action_type,
							 esif_action_type_str(algorithmPtr->action_type),
							 ltrim(esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->temp_xform)), PREFIX_ALGORITHM_TYPE),
							 algorithmPtr->tempC1,
							 algorithmPtr->percent_xform,
							 ltrim(esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->power_xform)), PREFIX_ALGORITHM_TYPE),
							 ltrim(esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->time_xform)), PREFIX_ALGORITHM_TYPE));
		} else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "<algorithmType>%s</algorithmType>\n"
							 "   <tempXform>%s</tempXform>\n"
							 "   <tempC1>%d</tempC1>\n"
							 "   <percent_xform>%d</percent_xform>\n"
							 "   <powerXform>%s</powerXform>\n"
							 "   <timeXform>%s</timeXform>\n",
							 esif_action_type_str(algorithmPtr->action_type),
							 esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->temp_xform)),
							 algorithmPtr->tempC1,
							 algorithmPtr->percent_xform,
							 esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->power_xform)),
							 esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->time_xform)));
		}

		/* Next Algorithm */
		algorithmPtr = (EsifFpcAlgorithmPtr)(algorithmPtr + 1);
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "Mapped Events:\n\n"
						 "Alias Event Identification             Group    Event Data           Event Description\n"
						 "----- -------------------------------- -------- -------------------- --------------------------------------\n");
	}	// FORMAT_TEXT

	/* First Event (Laid After The Last Algorithm) */
	eventPtr = (EsifFpcEventPtr)algorithmPtr;
	for (i = 0; i < fpcPtr->number_of_events && i < MAX_EVENTS; i++) {
		offset = (unsigned long)((u8 *)eventPtr - base_ptr);

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%5s ",
							 eventPtr->event_name);

			for (j = 0; j < 16; j++) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02x", eventPtr->event_key[j]);
			}

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 " %-8s %-20s %s(%d)\n",
							 ltrim(esif_event_group_enum_str(eventPtr->esif_group), PREFIX_EVENT_GROUP),
							 esif_data_type_str(eventPtr->esif_group_data_type),
							 ltrim(esif_event_type_str(eventPtr->esif_event), PREFIX_EVENT_TYPE),
							 eventPtr->esif_event);
		} else {// FORMAT_XML
			char key[64] = "";

			for (j = 0; j < 16; j++) {
				esif_ccb_sprintf_concat(64, key, "%02x", eventPtr->event_key[j]);
			}

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "<eventName>%s</eventName>\n"
							 "   <key>%s</key>\n"
							 "   <group>%d</group>\n"
							 "   <groupString>%s</groupString>\n"
							 "   <groupData>%d</groupData>\n"
							 "   <groupDataString>%s</groupDataString>\n"
							 "   <eventNum>%d</evenNum>\n"
							 "   <event>%s</event>\n",
							 eventPtr->event_name,
							 key,
							 eventPtr->esif_group,
							 esif_event_group_enum_str(eventPtr->esif_group),
							 eventPtr->esif_group_data_type,
							 esif_data_type_str(eventPtr->esif_group_data_type),
							 eventPtr->esif_event,
							 esif_event_type_str(eventPtr->esif_event));
		}

		/* Next Event */
		eventPtr = (EsifFpcEventPtr)(eventPtr + 1);
	}

exit:
	// Cleanup
	esif_ccb_free(fpcPtr);
	IOStream_Destroy(io_ptr);
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	return output;
}


// CPC Info
static char *esif_shell_cmd_infocpc(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char cpc_domain_str[8] = "";
	char *cpc_pattern = "";
	char *edp_filename     = 0;
	char edp_name[PATH_STR_LEN];
	char edp_full_path[PATH_STR_LEN];
	u32 cpc_size    = 0;
	size_t cpc_read = 0;
	u8 *byte_ptr    = NULL;
	struct esif_lp_cpc *cpc_ptr = NULL;
	struct esif_cpc_primitive *primitivePtr = NULL;
	struct esif_cpc_action *actionPtr = NULL;
	u32 i = 0, j = 0;
	IOStreamPtr io_ptr    = IOStream_Create();
	EsifDataPtr nameSpace = 0;
	EsifDataPtr key = 0;
	EsifDataPtr value     = 0;

	if (argc < 2 || io_ptr == NULL) {
		IOStream_Destroy(io_ptr);
		return NULL;
	}

	// parse filename no extension
	edp_filename = argv[1];

	// parse option regular expression
	if (argc > 2) {
		cpc_pattern = argv[2];
	}

	// Look for EDP file either on disk or in a DataVault (static or file), depending on priority setting
	esif_ccb_sprintf(PATH_STR_LEN, edp_name, "%s.edp", edp_filename);
	nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, ESIF_DSP_NAMESPACE, 0, ESIFAUTOLEN);
	key       = EsifData_CreateAs(ESIF_DATA_STRING, edp_name, 0, ESIFAUTOLEN);
	value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

	if (nameSpace == NULL || key == NULL || value == NULL) {
		goto exit;
	}
	esif_build_path(edp_full_path, sizeof(edp_full_path), ESIF_PATHTYPE_DSP, edp_name, NULL);

	if (!esif_ccb_file_exists(edp_full_path) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		esif_ccb_strcpy(edp_full_path, edp_name, PATH_STR_LEN);
		IOStream_SetMemory(io_ptr, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(io_ptr, edp_full_path, (char *)FILE_READ);
	}
	if (IOStream_Open(io_ptr) == 0) {
		struct edp_dir edp_dir;
		size_t r_bytes;

		/* FIND CPC within EDP file */
		r_bytes  = IOStream_Read(io_ptr, &edp_dir, sizeof(struct edp_dir));
		if (!esif_verify_edp(&edp_dir, r_bytes)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid EDP Header: Signature=%4.4s Version=%d\n", (char *)&edp_dir.signature, edp_dir.version);
			goto exit;
		}
		cpc_size = edp_dir.fpc_offset - edp_dir.cpc_offset;
		if (cpc_size > MAX_CPC_SIZE) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: cpc_size too large (%u)\n", ESIF_FUNC, cpc_size);
			goto exit;
		}
		IOStream_Seek(io_ptr, edp_dir.cpc_offset, SEEK_SET);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: file not found (%s)\n", ESIF_FUNC, edp_full_path);
		goto exit;
	}

	// allocate space for our CPC file contents
	cpc_ptr = (struct esif_lp_cpc *)esif_ccb_malloc(cpc_size);
	if (NULL == cpc_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: malloc failed to allocate %u bytes\n",
						 ESIF_FUNC, cpc_size);
		goto exit;
	}

	// read file contents
	cpc_read = IOStream_Read(io_ptr, cpc_ptr, cpc_size);
	if (cpc_read < cpc_size) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: read short received %u of %u bytes\n",
						 ESIF_FUNC, (int)cpc_read, cpc_size);
		goto exit;
	}

	// check signature to make sure this is a CPC file
	if (cpc_ptr->header.cpc.signature != *(unsigned int *)ESIF_CPC_SIGNATURE) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: signature validation failure not a cpc file\n",
						 ESIF_FUNC);
		goto exit;
	}

	// parse and dump
	byte_ptr = (u8 *)&cpc_ptr->header.cpc.signature;
	if (cpc_ptr->number_of_basic_primitives > MAX_PRIMITIVES ||
		cpc_ptr->number_of_algorithms > MAX_ALGORITHMS ||
		cpc_ptr->number_of_domains > MAX_DOMAINS ||
		cpc_ptr->number_of_events > MAX_EVENTS) {
		goto exit;
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "CPC File Info For %s Size %u:\n\n",
						 edp_filename, cpc_size);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "CPC Source:     %s\n"
						 "CPC Size:       %d\n"
						 "CPC Primitives: %d\n"
						 "CPC Algorithms: %d\n"
						 "CPC Domains:    %d\n"
						 "CPC EventMaps:  %d\n"
						 "CPC Signature:  %c%c%c%c\n"
						 "CPC Version:    %d\n"
						 "CPC Flags:      %08x\n"
						 "DSP Version:    %d\n"
						 "DSP Code:       %s\n"
						 "DSP Content:    v%d.%d\n"
						 "DSP Flags:      %08x\n\n",
						 (IOStream_GetType(io_ptr) == StreamMemory ? "DataVault" : "File"),
						 cpc_ptr->size,
						 cpc_ptr->number_of_basic_primitives,
						 cpc_ptr->number_of_algorithms,
						 cpc_ptr->number_of_domains,
						 cpc_ptr->number_of_events,
						 *byte_ptr, *(byte_ptr + 1), *(byte_ptr + 2), *(byte_ptr + 3),
						 cpc_ptr->header.cpc.version,
						 cpc_ptr->header.cpc.flags,
						 cpc_ptr->header.version,
						 cpc_ptr->header.code,
						 cpc_ptr->header.ver_major,
						 cpc_ptr->header.ver_minor,
						 cpc_ptr->header.flags);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<cpcinfo>\n"
						 "  <size>%d</size>\n"
						 "  <primitives>%d</primitives>\n"
						 "  <cpcSignature>%c%c%c%c</cpcSignature>\n"
						 "  <cpcVersion>%d</cpcVersion>\n"
						 "  <cpcFlags>%08x</cpcFlags>\n"
						 "  <dspVersion>%d</dspVersion>\n"
						 "  <dspCode>%s</dspCode>\n"
						 "  <dspContentVer>%d.%d</dspContentVer>\n"
						 "  <dspFlags>%08x</dspFlags>\n",
						 cpc_ptr->size,
						 cpc_ptr->number_of_basic_primitives,
						 *byte_ptr, *(byte_ptr + 1), *(byte_ptr + 2), *(byte_ptr + 3),
						 cpc_ptr->header.cpc.version,
						 cpc_ptr->header.cpc.flags,
						 cpc_ptr->header.version,
						 cpc_ptr->header.code,
						 cpc_ptr->header.ver_major, cpc_ptr->header.ver_minor,
						 cpc_ptr->header.flags);
	}

	if (FORMAT_XML == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  <primitives>\n");
	}

	primitivePtr = (struct esif_cpc_primitive *)(cpc_ptr + 1);
	for (i = 0; i < cpc_ptr->number_of_basic_primitives; i++) {
		int show = 0;
		char temp_buf[1024];

		if (!strcmp(cpc_pattern, "")) {
			show = 1;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(1024, temp_buf, "%03d.%s.%03d %s actions %d %s\n",
							 primitivePtr->tuple.id,
							 esif_primitive_domain_str(primitivePtr->tuple.domain, cpc_domain_str, 8),
							 primitivePtr->tuple.instance,
							 esif_primitive_opcode_str((enum esif_primitive_opcode)primitivePtr->operation),
							 primitivePtr->number_of_actions,
							 esif_primitive_str((enum esif_primitive_type)primitivePtr->tuple.id));
			// TODO REGEXP Here For Now Simple Pattern Match
			if (0 == show && strstr(temp_buf, cpc_pattern)) {
				show = 1;
			}
			// Show Row?
			if (1 == show) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
			}

			actionPtr = (struct esif_cpc_action *)(primitivePtr + 1);
			for (j = 0; j < primitivePtr->number_of_actions; j++, actionPtr++) {
				if (!strcmp(cpc_pattern, "")) {
					show = 1;
				}
				esif_ccb_sprintf(1024, temp_buf, " %s param_valid %1d:%1d:%1d:%1d:%1d param %x:%x:%x:%x:%x\n",
								 esif_action_type_str(actionPtr->type),
								 actionPtr->param_valid[0],
								 actionPtr->param_valid[1],
								 actionPtr->param_valid[2],
								 actionPtr->param_valid[3],
								 actionPtr->param_valid[4],
								 actionPtr->param[0],
								 actionPtr->param[1],
								 actionPtr->param[2],
								 actionPtr->param[3],
								 actionPtr->param[4]);

				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			}
		} else {// FORMAT_XML
			esif_ccb_sprintf(1024, temp_buf,
							 "    <primitive>\n"
							 "      <id>%d</id>\n"
							 "      <primitiveStr>%s</primitiveStr>\n"
							 "      <domain>%s</domain>\n"
							 "      <instance>%d</instance>\n"
							 "      <opcode>%d</opcode>\n"
							 "      <opcodeStr>%s</opcodeStr>\n"
							 "      <actionCount>%d</actionCount>\n"
							 "    </primitive>\n",
							 primitivePtr->tuple.id,
							 esif_primitive_str((enum esif_primitive_type)primitivePtr->tuple.id),
							 esif_primitive_domain_str(primitivePtr->tuple.domain, cpc_domain_str, 8),
							 primitivePtr->tuple.instance,
							 primitivePtr->operation,
							 esif_primitive_opcode_str((enum esif_primitive_opcode)primitivePtr->operation),
							 primitivePtr->number_of_actions);
			// TODO REGEXP Here For Now Simple Pattern Match
			if (0 == show && strstr(temp_buf, cpc_pattern)) {
				show = 1;
			}
			// Show Row?
			if (1 == show) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
			}

			actionPtr = (struct esif_cpc_action *)(primitivePtr + 1);
			for (j = 0; j < primitivePtr->number_of_actions; j++, actionPtr++) {
				if (!strcmp(cpc_pattern, "")) {
					show = 1;
				}
				esif_ccb_sprintf(1024, temp_buf,
								 "   <action>\n"
								 "      <type>%d</type>\n"
								 "      <typeStr>%s</typeStr>\n"
								 "      <numValidParam>%d</numValidParam>\n"
								 "      <paramValid>%1d:%1d:%1d:%1d:%1d</paramValid>\n"
								 "      <param0>%x</param0>\n"
								 "      <param1>%x</param1>\n"
								 "      <param2>%x</param2>\n"
								 "      <param3>%x</param3>\n"
								 "      <param4>%x</param4>\n"
								 "   </action>\n",
								 actionPtr->type,
								 esif_action_type_str(actionPtr->type),
								 actionPtr->num_valid_params,
								 actionPtr->param_valid[0],
								 actionPtr->param_valid[1],
								 actionPtr->param_valid[2],
								 actionPtr->param_valid[3],
								 actionPtr->param_valid[4],
								 actionPtr->param[0],
								 actionPtr->param[1],
								 actionPtr->param[2],
								 actionPtr->param[3],
								 actionPtr->param[4]);
				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			}
		}

		// Next Primitive In CPC Set
		primitivePtr = (struct esif_cpc_primitive *)((u8 *)primitivePtr + primitivePtr->size);
	}

	if (FORMAT_XML == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  </primitives>\n");
	}

	if (FORMAT_TEXT == g_format) {
		u8 algo_index   = 0;
		u8 domainIndex = 0;
		u8 event_index  = 0;
		EsifFpcAlgorithmPtr algo_ptr = (EsifFpcAlgorithmPtr)primitivePtr;
		EsifFpcDomainPtr domainPtr  = NULL;
		struct esif_cpc_event *eventPtr    = NULL;

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "ALGORITHMS:\n\n"
						 "Action Description           tempxform tc1      tc2      pxform timexform\n"
						 "------ --------------------- --------- -------- -------- ------ ---------\n");

		for (algo_index = 0; algo_index < cpc_ptr->number_of_algorithms; algo_index++) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%-2d     %-21s %s(%d) %08x %08x %s(%d) %s(%d)\n",
							 algo_ptr->action_type,
							 esif_action_type_str(algo_ptr->action_type),
							 esif_algorithm_type_str((enum esif_algorithm_type)(algo_ptr->temp_xform)),
							 algo_ptr->temp_xform,
							 algo_ptr->tempC1,
							 algo_ptr->percent_xform,
							 esif_algorithm_type_str((enum esif_algorithm_type)(algo_ptr->power_xform)),
							 algo_ptr->power_xform,
							 esif_algorithm_type_str((enum esif_algorithm_type)(algo_ptr->time_xform)),
							 algo_ptr->time_xform);
			algo_ptr++;
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "DOMAINS:\n\n"
						 "Name     Description          Domain   Priority Capability Type               \n"
						 "-------- -------------------- -------- -------- ---------- -------------------\n");


		domainPtr = (EsifFpcDomainPtr)algo_ptr;
		for (domainIndex = 0; domainIndex < cpc_ptr->number_of_domains; domainIndex++) {
			char qualifier_str[64];

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%-8s %-20s %-8s %08x %08x   %s(%d)\n",
							 domainPtr->descriptor.name,
							 domainPtr->descriptor.description,
							 esif_primitive_domain_str(domainPtr->descriptor.domain, qualifier_str, 64),
							 domainPtr->descriptor.priority,
							 domainPtr->capability_for_domain.capability_flags,
							 esif_domain_type_str(domainPtr->descriptor.domainType), domainPtr->descriptor.domainType);

			// esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s\n", esif_guid_print((esif_guid_t*) &domainPtr->descriptor.guid, qualifier_str));
			domainPtr++;
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "Mapped Events:\n\n"
						 "Alias Event Identification             Group    Event Data           Event Description\n"
						 "----- -------------------------------- -------- -------------------- --------------------------------------\n");

		eventPtr = (struct esif_cpc_event *)domainPtr;
		for (event_index = 0; event_index < cpc_ptr->number_of_events; event_index++) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%5s ",
							 eventPtr->name);

			for (i = 0; i < 16; i++) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02x", eventPtr->event_key[i]);
			}

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 " %-8s %-20s %s\n", 
							 ltrim(esif_event_group_enum_str(eventPtr->esif_group), PREFIX_EVENT_GROUP),
							 esif_data_type_str(eventPtr->esif_group_data_type),
							 ltrim(esif_event_type_str(eventPtr->esif_event), PREFIX_EVENT_TYPE));

			eventPtr++;
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	} else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</cpcinfo>\n");
	}
exit:
	// Cleanup
	esif_ccb_free(cpc_ptr);
	IOStream_Destroy(io_ptr);
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	return output;
}


// Load
static char *esif_shell_cmd_load(EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char **argv    = shell->argv;
	char *output   = shell->outbuf;
	char *filename = 0;
	char outline[MAX_LINE]={0};
	int outline_len;
	char *ptr;
	int opt       = 1;
	char load_full_path[PATH_STR_LEN]={0};
	int cat       = 0;
	char *checkDV = 0;	// Default Namespace to check ("_dsp", "_cmd", etc), if any
	IOStreamPtr io_ptr = IOStream_Create();

	UNREFERENCED_PARAMETER(output);

	if (argc <= opt || io_ptr == NULL) {
		IOStream_Destroy(io_ptr);
		return NULL;
	}

	// Deduce cat and path options based on command
	if (esif_ccb_stricmp(argv[0], "cat") == 0) {
		cat  = 1;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_CMD, NULL, NULL);
	} 
	else if (esif_ccb_stricmp(argv[0], "cattst") == 0) {
		checkDV = ESIF_DSP_NAMESPACE;
		cat     = 1;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_DSP, NULL, NULL);
	}
	else if (esif_ccb_stricmp(argv[0], "load") == 0) {
		cat  = 0;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_CMD, NULL, NULL);
	}
	else if (esif_ccb_stricmp(argv[0], "loadtst") == 0) {
		checkDV = ESIF_DSP_NAMESPACE;
		cat     = 0;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_DSP, NULL, NULL);
	}
	else if (esif_ccb_stricmp(argv[0], "proof") == 0) {
		cat  = 2;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_CMD, NULL, NULL);
	}
	else if (esif_ccb_stricmp(argv[0], "prooftst") == 0) {
		checkDV = ESIF_DSP_NAMESPACE;
		cat     = 2;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_DSP, NULL, NULL);
	}

	// load file $1 $2 $3 $4 ...
	// Filenameexit
	filename = argv[opt++];

	// Look for Script on disk first then in DataVault (static or file)
	if (checkDV && !esif_ccb_file_exists(filename)) {
		EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, checkDV, 0, ESIFAUTOLEN);
		EsifDataPtr key       = EsifData_CreateAs(ESIF_DATA_STRING, filename, 0, ESIFAUTOLEN);
		EsifDataPtr value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		if (nameSpace != NULL && key != NULL && value != NULL && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
			esif_ccb_strcpy(load_full_path, filename, PATH_STR_LEN);
			IOStream_SetMemory(io_ptr, (BytePtr)value->buf_ptr, value->data_len);
		}
		EsifData_Destroy(nameSpace);
		EsifData_Destroy(key);
		EsifData_Destroy(value);
	}
	if (IOStream_GetType(io_ptr) == StreamNull) {
		esif_ccb_sprintf_concat(PATH_STR_LEN, load_full_path, "%s%s", ESIF_PATH_SEP, filename);
		IOStream_SetFile(io_ptr, load_full_path, "r");
	}
	if (IOStream_Open(io_ptr) == EOK) {
		int run = 1;
		while (run) {
			if (IOStream_GetLine(io_ptr, outline, MAX_LINE) == NULL) {
				break;
			}

			if (0 == cat || 2 == cat) {	/* Load Or Prove */
				// Replace Tokens  ... $1, $2, $3...
				int tok;
				for (tok = 1; tok <= 9 && tok <= argc - opt; tok++) {
					char tokstr[3] = {'$', ((char)tok + '0'), 0};	// "$1".."$9"
					char *replaced = esif_str_replace(outline, tokstr, argv[tok + opt - 1]);
					if (replaced) {
						esif_ccb_strcpy(outline, replaced, MAX_LINE);
						esif_ccb_free(replaced);
					}
				}

				// Replace %dst% with current destination
				if (strstr(outline, "$dst$")) {
					char dst_str[16];
					char *replaced = NULL;
					esif_ccb_sprintf(16, dst_str, "%d", g_dst);
					replaced = esif_str_replace(outline, (char *)"$dst$", dst_str);
					if (replaced) {
						esif_ccb_strcpy(outline, replaced, MAX_LINE);
						esif_ccb_free(replaced);
					}
				}
			}

			if (cat) {	// proof or cat NOT Load
				CMD_OUT("%s", outline);
			} else {
				// LOAD
				ptr = outline;
				while (*ptr != '\0') {
					if (*ptr == '\r' || *ptr == '\n' || *ptr == '#') {
						*ptr = '\0';
					}
					ptr++;
				}

				outline_len = (int)(ptr - outline);
				if (!((esif_ccb_strnicmp(outline, "rem",
										 3) == 0) && (outline_len > 3) &&
					  ((outline[3] == '\0') || (outline[3] == ' ') || (outline[3] == '\n') || (outline[3] == '\t')))) {
					parse_cmd(outline, ESIF_FALSE);
				}
				*output = 0;
			}
		}
		IOStream_Close(io_ptr);
	}
	IOStream_Destroy(io_ptr);
	return output;
}


// Log
static char *esif_shell_cmd_log(EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char **argv    = shell->argv;
	char *output   = shell->outbuf;
	int arg        = 1;
	char *subcmd   = NULL;
	char *filename = NULL;
	char fullpath[MAX_PATH]={0};
	char *logid    = "shell";
	EsifLogType logtype = ESIF_LOG_SHELL;

	if (argc > 1) {
		subcmd = argv[arg++];
	}

	// Get optional [logid] (use shell log if none specified)
	if (argc >= 3) {
		char *id = argv[arg];
		EsifLogType newtype = EsifLogType_FromString(id);

		// Assume no logid if invalid name
		if (newtype != ESIF_LOG_EVENTLOG || esif_ccb_strnicmp(id, "eventlog", 5)==0) {
			logtype = newtype;
			logid = id;
			arg++;
		}
	}

	// log
	// log list
	if (argc <= 1 || esif_ccb_stricmp(subcmd, "list")==0) {
		EsifLogFile_DisplayList();
	}
	// log scan [pattern]
	else if (esif_ccb_stricmp(subcmd, "scan")==0) {
		esif_ccb_file_enum_t find_handle = INVALID_HANDLE_VALUE;
		struct esif_ccb_file ffd = {0};
		char file_path[MAX_PATH] = {0};
		char *log_pattern = "*.log";

		if (argc > arg) {
			log_pattern = argv[arg++];
		}
		esif_build_path(file_path, sizeof(file_path), ESIF_PATHTYPE_LOG, NULL, NULL);

		// Find matching files and return in Text or XML format
		if (g_format==FORMAT_XML) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s", "<logs>\n");
		}
		if ((find_handle = esif_ccb_file_enum_first(file_path, log_pattern, &ffd)) != INVALID_HANDLE_VALUE) {
			do {
				if (esif_ccb_strcmp(ffd.filename, ".")==0 || esif_ccb_strcmp(ffd.filename, "..")==0) {
					continue; // Ignore . and ..
				}
				if (g_format==FORMAT_XML) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "<name>%s</name>\n", ffd.filename);
				}
				else {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s\n", ffd.filename);
				}
			} while (esif_ccb_file_enum_next(find_handle, log_pattern, &ffd));
			esif_ccb_file_enum_close(find_handle);
		}
		if (g_format==FORMAT_XML) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", "</logs>\n");
		}
	}
	// log close [logid]
	else if (esif_ccb_stricmp(subcmd, "close")==0) {
		if (!EsifLogFile_IsOpen(logtype)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log not open\n", logid);
		}
		else {
			EsifLogFile_Close(logtype);
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log closed\n", logid);
		}
	}
	// log <filename>
	// log open [logid] <filename> [append]
	else if (argc == 2 || (argc > arg && esif_ccb_stricmp(subcmd, "open")==0)) {
		char *fname = NULL;
		int append = ESIF_FALSE;
		if (argc > 2)
			filename = argv[arg++];
		else if (esif_ccb_stricmp(subcmd, "open") == 0)
			return NULL;
		else
			filename = subcmd;

		// Replace %DATETIME% in filename with yyyy-mm-dd-hhmmss
		if (esif_ccb_strchr(filename, '%') != 0) {
			char datetime[20] = {0};
			time_t now = time(NULL);
			struct tm time = {0};
			if (esif_ccb_localtime(&time, &now) == 0) {
				esif_ccb_sprintf(sizeof(datetime), datetime, "%04d-%02d-%02d-%02d%02d%02d",
					time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
				fname = esif_str_replace(filename, "%DATETIME%", datetime);
				filename = fname;
			}
		}

		if (argc > arg && esif_ccb_stricmp(argv[arg], "append")==0) {
			append = ESIF_TRUE;
		}

		EsifLogFile_Open(logtype, filename, append);
		EsifLogFile_GetFullPath(fullpath, sizeof(fullpath), filename);
		if (!EsifLogFile_IsOpen(logtype)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log: ERROR opening %s\n", logid, fullpath);
		}
		else {
			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log opened: %s\n", logid, fullpath);
			}
			else {
				esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<result>\n"
						 "  <logid>%s</logid>\n"
						 "  <logfilename>%s</logfilename>\n"
						 "  </result>",logid, fullpath);
			}
		}
		esif_ccb_free(fname);
	}
	// log write [logid] "string"
	else if (argc > arg && (esif_ccb_stricmp(subcmd, "write")==0 || esif_ccb_stricmp(subcmd, "msg")==0)) {
		char *msg = argv[arg++];

		if (!EsifLogFile_IsOpen(logtype)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log not opened\n", logid);
		}
		else {
			EsifLogFile_Write(logtype, "%s\n", msg);
		}
	}
	// unknown subcmd
	else {
		return NULL;
	}
	return output;
}

// nolog
static char *esif_shell_cmd_nolog(EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char **argv    = shell->argv;
	char *output   = shell->outbuf;
	int new_argc = 2;
	char *new_argv[MAX_ARGV]={"log", "close"};

	// log close
	shell->argc = new_argc;
	shell->argv = new_argv;
	output = esif_shell_cmd_log(shell);
	shell->argc = argc;
	shell->argv = argv;
	return output;
}

// Memstats
static char *esif_shell_cmd_memstats(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc *ipc_ptr = NULL;
	u32 i = 0;
	struct esif_command_get_memory_stats *data_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_memory_stats);
	struct esif_ipc_command *command_ptr = NULL;
	u32 reset = 0;

	if (argc > 1 && !strcmp(argv[1], "reset")) {
		reset = 1;
	}

	ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_MEMORY_STATS;
	command_ptr->req_data_type   = ESIF_DATA_UINT32;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 4;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	// Command
	*(u32 *)(command_ptr + 1) = reset;
	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// Our data
	data_ptr = (struct esif_command_get_memory_stats *)(command_ptr + 1);
	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nGlobal Memory Stats: \n"
						 "-----------------------\n"
						 "MemAllocs: %u\n"
						 "MemFrees:  %u\n"
						 "MemInuse:  %u\n\n",
						 data_ptr->stats.allocs,
						 data_ptr->stats.frees,
						 data_ptr->stats.allocs - data_ptr->stats.frees);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<memstats>\n"
						 "  <global>\n"
						 "    <memAllocs>%u</memAllocs>\n"
						 "    <memFrees>%u</memFrees>\n"
						 "    <memInuse>%u</memInuse>\n"
						 "    <memPoolObjFrees>%u</memPoolObjFrees>\n"
						 "    <memPoolAllocs>%u</memPoolAllocs>\n"
						 "    <memPoolFrees>%u</memPoolFrees>\n"
						 "    <memPoolInuse>%u</memPoolInuse>\n"
						 "    <memPoolObjAllocs>%u</memPoolObjAllocs>\n"
						 "    <memPoolObjFrees>%u</memPoolObjFrees>\n"
						 "    <memPoolObjInuse>%u</memPoolObjInuse>\n"
						 "  </global>\n",
						 data_ptr->stats.allocs,
						 data_ptr->stats.frees,
						 data_ptr->stats.allocs - data_ptr->stats.frees,
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolAllocs,
						 data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolAllocs - data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolObjAllocs,
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolObjAllocs - data_ptr->stats.memPoolObjFrees);
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "Memory Pools:\n"
						 "Name                      Tag  Size Allocs       Frees        Inuse        Bytes    \n"
						 "------------------------- ---- ---- ------------ ------------ ------------ ---------\n");
	} else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  <mempools>\n");
	}

	for (i = 0; i < ESIF_MEMPOOL_TYPE_MAX; i++) {
		char tag[8] = {0};
		esif_ccb_memcpy(tag, &data_ptr->mempool_stat[i].pool_tag, 4);
		if (0 == data_ptr->mempool_stat[i].pool_tag) {
			continue;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%-25s %s %-4d %-12u %-12u %-12u %-9u\n",
							 data_ptr->mempool_stat[i].name,
							 tag,
							 data_ptr->mempool_stat[i].object_size,
							 data_ptr->mempool_stat[i].alloc_count,
							 data_ptr->mempool_stat[i].free_count,
							 data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count,
							 (data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count) *
							 data_ptr->mempool_stat[i].object_size);
		} else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "    <mempool>\n"
							 "        <name>%s</name>\n"
							 "        <tag>%s</tag>\n"
							 "        <size>%d</size>\n"
							 "        <allocs>%d</allocs>\n"
							 "        <frees>%d</frees>\n"
							 "        <inuse>%d</inuse>\n"
							 "        <bytes>%d</bytes>\n"
							 "    </mempool>\n",
							 data_ptr->mempool_stat[i].name,
							 tag,
							 data_ptr->mempool_stat[i].object_size,
							 data_ptr->mempool_stat[i].alloc_count,
							 data_ptr->mempool_stat[i].free_count,
							 data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count,
							 (data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count) *
							 data_ptr->mempool_stat[i].object_size);
		}
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "\n"
						 "MemPoolObjFrees:  %u\n"
						 "MemPoolAllocs:    %u\n"
						 "MemPoolFrees:     %u\n"
						 "MemPoolInuse:     %u\n"
						 "MemPoolObjAllocs: %u\n"
						 "MemPoolObjFrees:  %u\n"
						 "MemPoolObjInuse:  %u\n\n",
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolAllocs,
						 data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolAllocs - data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolObjAllocs,
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolObjAllocs - data_ptr->stats.memPoolObjFrees);
	} else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
			"  </mempools>\n"
			"</memstats>\n");
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// Participant
static char *esif_shell_cmd_participantk(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_command_get_part_detail *data_ptr = NULL;
	struct esif_ipc_command *command_ptr = NULL;
	struct esif_ipc *ipc_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_part_detail);
	u32 participant_id = 0;

	if (argc < 2) {
		return NULL;
	}

	participant_id = esif_atoi(argv[1]);

	ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
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
	*(u32 *)(command_ptr + 1) = participant_id;
	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// our data
	data_ptr = (struct esif_command_get_part_detail *)(command_ptr + 1);
	if (0 == data_ptr->version) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: participant not available\n", ESIF_FUNC);
		goto exit;
	}

	// Parse
	if (FORMAT_TEXT == g_format) {
		char guid_str[ESIF_GUID_PRINT_SIZE];
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nInstance:          %d\n"
						 "Version:           %d\n"
						 "Enumerator:        %d\n"
						 "Name:              %s\n"
						 "Desc:              %s\n"
						 "Driver Name:       %s\n"
						 "Device Name:       %s\n"
						 "Device Path:       %s\n"
						 "Class:             %s\n"
						 "Flags:             0x%08x\n"
						 "Status:            %s(%d)\n"
						 "Work Timer Period: %d\n\n",
						 data_ptr->id,
						 data_ptr->version,
						 data_ptr->enumerator,
						 data_ptr->name,
						 data_ptr->desc,
						 data_ptr->driver_name,
						 data_ptr->device_name,
						 data_ptr->device_path,
						 esif_guid_print((esif_guid_t *)data_ptr->class_guid, guid_str),
						 data_ptr->flags,
						 esif_lp_state_str((enum esif_lp_state)data_ptr->state),
						 data_ptr->state,
						 data_ptr->timer_period);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "ACPI Attributes\n"
						 "--------------------------------------------------------------------\n"
						 "Device:      %s\n"
						 "Scope:       %s\n"
						 "Unique ID:   %s\n"
						 "Type:        0x%08x\n\n",
						 data_ptr->acpi_device,
						 data_ptr->acpi_scope,
						 data_ptr->acpi_uid,
						 data_ptr->acpi_type);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "PCI Attributes\n"
						 "--------------------------------------------------------------------\n"
						 "Vendor:      0x%08x\n"
						 "Device:      0x%08x\n"
						 "Bus:         0x%02x\n"
						 "Bus Device:  0x%02x\n"
						 "Function:    0x%02x\n"
						 "Revision:    0x%02x\n"
						 "Class:       0x%02x\n"
						 "SubClass:    0x%02x\n"
						 "ProgIF:      0x%02x\n\n",
						 data_ptr->pci_vendor,
						 data_ptr->pci_device,
						 data_ptr->pci_bus,
						 data_ptr->pci_bus_device,
						 data_ptr->pci_function,
						 data_ptr->pci_revision,
						 data_ptr->pci_class,
						 data_ptr->pci_sub_class,
						 data_ptr->pci_prog_if);

		if (data_ptr->capability > 0) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "Participant Capabilities: 0x%08x                                  \n"
							 "--------------------------------------------------------------------\n",
							 data_ptr->capability);

			if (data_ptr->capability & ESIF_CAPABILITY_ACTIVE_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "ACTIVE_CONTROL\t\tHas Fan Features\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_CTDP_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "CTDP_CONTROL\t\tHas CPU CTDP Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_CORE_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "CORE_CONTROL\t\t\tHas CPU/Core Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_CORE_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "CORE_CONTROL\t\t\tHas CPU/Core Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_DISPLAY_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "DISPLAY_CONTROL\t\t\tHas Display Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_PERF_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "PERF_CONTROL\t\tHas Performance Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_POWER_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "POWER_CONTROL\t\tHas RAPL Power Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_POWER_STATUS) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "POWER_STATUS\t\t\t\tHas RAPL Power Feature\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_TEMP_STATUS) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "TEMP_STATUS\t\tHas Temp Sensor\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_TEMP_THRESHOLD) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "TEMP_THRESHOLD\t\tHas Temp Thresholds\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_UTIL_STATUS) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "UTIL_STATUS\t\t\tReports Device Utilization\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_PIXELCLOCK_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "PIXELCLOCK_CONTROL\t\t\tHas Pixel Clock Control\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_PIXELCLOCK_STATUS) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "PIXELCLOCK_STATUS\t\t\tHas Pixel Clock Status\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_RFPROFILE_STATUS) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "RFPROFILE_STATUS\t\t\tHas RF Profile Status\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_RFPROFILE_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "RFPROFILE_CONTROL\t\t\tHas RF Profile Control\n");
			}

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		}

		if (1 == data_ptr->have_dsp) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "Device Support Package(DSP):                  \n"
							 "------------------------------------------------\n"
							 "Code:         %s\n"
							 "Content Ver:  %d.%d\n"
							 "Type:         %s\n\n",
							 data_ptr->dsp_code,
							 data_ptr->dsp_ver_major,
							 data_ptr->dsp_ver_minor,
							 "Compact Primitive Catalog (CPC)");
		}
		
		if (1 == data_ptr->have_cpc) {
			u8 *byte_ptr = (u8 *)&data_ptr->cpc_signature;
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "Compact Primitive Catalog(CPC):               \n"
							 "------------------------------------------------\n"
							 "Version:          %d\n"
							 "Signature:        %08X (%c%c%c%c)\n"
							 "Size:             %d\n"
							 "Primitive Count:  %d\n\n",
							 data_ptr->cpc_version,
							 data_ptr->cpc_signature,
							 *byte_ptr, *(byte_ptr + 1), *(byte_ptr + 2), *(byte_ptr + 3),
							 data_ptr->cpc_size,
							 data_ptr->cpc_primitive_count);
		}
	} else {// FORMAT_XML
		char sig_str[10] = {0};
		u16 capability_str_len    = 1024;
		char capability_str[1024] = {0};
		char guid_str[ESIF_GUID_PRINT_SIZE];
		esif_ccb_strcpy(sig_str, "NA", 10);

		if (data_ptr->cpc_signature > 0) {
			u8 *ptr = (u8 *)&data_ptr->cpc_signature;
			esif_ccb_sprintf(10, sig_str, "%c%c%c%c", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3));
		}

		if (data_ptr->capability & ESIF_CAPABILITY_TEMP_STATUS) {
			esif_ccb_sprintf_concat(capability_str_len, capability_str, "TEMP_SENSOR,");
		}
		if (data_ptr->capability & ESIF_CAPABILITY_ACTIVE_CONTROL) {
			esif_ccb_sprintf_concat(capability_str_len, capability_str, "COOLING_DEVICE,");
		}
		if (data_ptr->capability & ESIF_CAPABILITY_POWER_STATUS) {
			esif_ccb_sprintf_concat(capability_str_len, capability_str, "RAPL_DEVICE");
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<participant>\n"
						 "  <id>%d</id>\n"
						 "  <name>%s</name>\n"
						 "  <desc>%s</desc>\n"
						 "  <class>%s</class>\n"
						 "  <version>%d</version>\n"
						 "  <status>%d</status>\n"
						 "  <statusStr>%s</statusStr>\n"
						 "  <timer>%d</timer>\n"
						 "  <capabilities>%08x</capabilities>\n"
						 "  <capabilitiesStr>%s</capabilitiesStr>\n"
						 "  <dspCode>%s</dspCode>\n"
						 "  <dspContentVersion>%d.%d</dspContentVersion>\n"
						 "  <dspType>%s</dspType>\n"
						 "  <cpcVersion>%d</cpcVersion>\n"
						 "  <cpcSignature>0x%08x</cpcSignature>\n"
						 "  <cpcSignatureStr>%s</cpcSignatureStr>\n"
						 "  <cpcSize>%d</cpcSize>\n"
						 "  <cpcPrimitiveCount>%d</cpcPrimitiveCount>\n"
						 "</participant>\n",
						 data_ptr->id,
						 data_ptr->name,
						 data_ptr->desc,
						 esif_guid_print((esif_guid_t *)data_ptr->class_guid, guid_str),
						 data_ptr->version,
						 data_ptr->state,
						 esif_lp_state_str((enum esif_lp_state)data_ptr->state),
						 data_ptr->timer_period,
						 data_ptr->capability,
						 capability_str,
						 data_ptr->dsp_code,
						 data_ptr->dsp_ver_major,
						 data_ptr->dsp_ver_minor,
						 "Compact Primitive Catalog",
						 data_ptr->cpc_version,
						 data_ptr->cpc_signature,
						 sig_str,
						 data_ptr->cpc_size,
						 data_ptr->cpc_primitive_count);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}

static char *esif_shell_cmd_paths(EsifShellCmdPtr shell)
{
	char *output = shell->outbuf;
	char targetFilePath[MAX_PATH] = { 0 };
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DV, NULL, 0);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "Datavault path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DPTF, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "DPTF path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DLL, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Policy path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DPTF, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Combined.xsl path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_UI, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "UI path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DSP, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "DSP path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_LOG, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Log path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_CMD, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Command path:\n %s \n\n", targetFilePath);
	return output;
}

// Participant
static char *esif_shell_cmd_participant(EsifShellCmdPtr shell)
{
	int argc             = shell->argc;
	char **argv          = shell->argv;
	char *output         = shell->outbuf;
	UInt8 participant_id = 0;
	EsifUpPtr upPtr     = NULL;
	EsifUpDataPtr metaPtr = NULL;

	if (argc < 2) {
		return NULL;
	}

	participant_id = (UInt8)esif_atoi(argv[1]);

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participant_id);
	if (NULL == upPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Participant not available\n");
		goto exit;
	}

	metaPtr = EsifUp_GetMetadata(upPtr);
	if (NULL == metaPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Participant metadata not available\n");
		goto exit;
	}


	// Parse
	if (FORMAT_TEXT == g_format) {
		char guid_str[ESIF_GUID_PRINT_SIZE];
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"\n"
			"Instance:          %d\n"
			"Version:           %d\n"
			"Enumerator:        %d\n"
			"Name:              %s\n"
			"Desc:              %s\n"
			"Driver Name:       %s\n"
			"Device Name:       %s\n"
			"Device Path:       %s\n"
			"Class:             %s\n"
			"Ptype:             %s (%d)\n"
			"Flags:             0x%08x\n"
			"Status:            %s(%d)\n",
			participant_id,
			metaPtr->fVersion,
			metaPtr->fEnumerator,
			metaPtr->fName,
			metaPtr->fDesc,
			metaPtr->fDriverName,
			metaPtr->fDeviceName,
			metaPtr->fDevicePath,
			esif_guid_print((esif_guid_t *)metaPtr->fDriverType, guid_str),
			esif_domain_type_str(metaPtr->fAcpiType),
			metaPtr->fAcpiType,
			metaPtr->fFlags,
			esif_pm_participant_state_str((enum esif_pm_participant_state)1),
			0);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
			"\n"
			"ACPI Attributes\n"
			"--------------------------------------------------------------------\n"
			"Device:      %s\n"
			"Scope:       %s\n"
			"Unique ID:   %s\n"
			"Type:        0x%08x\n",
			metaPtr->fAcpiDevice,
			metaPtr->fAcpiScope,
			metaPtr->fAcpiUID,
			metaPtr->fAcpiType);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
			"\n"
			"PCI Attributes\n"
			"--------------------------------------------------------------------\n"
			"Vendor:      0x%08x\n"
			"Device:      0x%08x\n"
			"Bus:         0x%02x\n"
			"Bus Device:  0x%02x\n"
			"Function:    0x%02x\n"
			"Revision:    0x%02x\n"
			"Class:       0x%02x\n"
			"SubClass:    0x%02x\n"
			"ProgIF:      0x%02x\n\n",
			metaPtr->fPciVendor,
			metaPtr->fPciDevice,
			metaPtr->fPciBus,
			metaPtr->fPciBusDevice,
			metaPtr->fPciFunction,
			metaPtr->fPciRevision,
			metaPtr->fPciClass,
			metaPtr->fPciSubClass,
			metaPtr->fPciProgIf);
	} else {// FORMAT_XML
		char guid_str[ESIF_GUID_PRINT_SIZE];
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"<participant>\n"
			"  <id>%d</id>\n"
			"  <enum>%d</enum>\n"
			"  <name>%s</name>\n"
			"  <desc>%s</desc>\n"
			"  <class>%s</class>\n"
			"  <version>%d</version>\n"
			"  <status>%d</status>\n"
			"  <statusStr>%s</statusStr>\n"
			"  <acpiDevice>%s</acpiDevice>\n"
			"  <acpiScope>%s</acpiScope>\n"
			"  <acpiUID>%s</acpiUID>\n"
			"  <acpiType>0x%08x</acpiType>\n"
			"  <pciVendor>0x%08x</pciVendor>\n"
			"  <pciDevice>0x%08x</pciDevice>\n"
			"  <pciBus>0x%02x</pciBus>\n"
			"  <pciBusDevice>0x%02x</pciBusDevice>\n"
			"  <pciFunction>0x%02x</pciFunction>\n"
			"  <pciRevision>0x%02x</pciRevision>\n"
			"  <pciClass>0x%02x</pciClass>\n"
			"  <pciSubClass>0x%02x</pciSubClass>\n"
			"  <pciProgIF>0x%02x</pciProgIF>\n"
			"</participant>\n",
			participant_id,
			metaPtr->fEnumerator,
			metaPtr->fName,
			metaPtr->fDesc,
			esif_guid_print((esif_guid_t *)metaPtr->fDriverType, guid_str),
			metaPtr->fVersion,
			1,
			esif_pm_participant_state_str((enum esif_pm_participant_state)1),
			metaPtr->fAcpiDevice,
			metaPtr->fAcpiScope,
			metaPtr->fAcpiUID,
			metaPtr->fAcpiType,
			metaPtr->fPciVendor,
			metaPtr->fPciDevice,
			metaPtr->fPciBus,
			metaPtr->fPciBusDevice,
			metaPtr->fPciFunction,
			metaPtr->fPciRevision,
			metaPtr->fPciClass,
			metaPtr->fPciSubClass,
			metaPtr->fPciProgIf);
	}
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return output;
}


// RemARK
static char *esif_shell_cmd_rem(EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	// Do Nothing
	return NULL;
}


// Repeat
/*static*/ char *esif_shell_cmd_repeat(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	if (argc < 2) {
		return NULL;
	}
	// repeat <count>
	else if (argc == 2) {
		g_repeat = esif_atoi(argv[1]);
		g_repeat = esif_ccb_min(esif_ccb_max(g_repeat, 0), MAX_REPEAT);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "repeat next command count=%d times\n", g_repeat);
	}
	// repeat <count> command args ...
	else {
		size_t buflen = 0;
		int opt = 1;
		int count = 0;
		char *cmd = NULL;
		int repeat = esif_atoi(argv[opt]);
		repeat = esif_ccb_min(esif_ccb_max(repeat, 0), MAX_REPEAT);

		for (opt = 2; opt < argc; opt++) {
			buflen += esif_ccb_strlen(argv[opt], OUT_BUF_LEN) + 3;
		}
		cmd = esif_ccb_malloc(buflen);
		if (cmd == NULL)
			return NULL;

		for (count = 0; count < repeat; count++) {
			esif_ccb_memset(cmd, 0, buflen);
			for (opt = 2; opt < argc; opt++) {
				esif_ccb_sprintf_concat(buflen, cmd, "\"%s\"", argv[opt]);
				if (opt + 1 < argc)
					esif_ccb_strcat(cmd, " ", buflen);
			}
			parse_cmd(cmd, ESIF_FALSE);
		}
		esif_ccb_free(cmd);
	}
	return output;
}


// Repeat Delay
/*static*/ char *esif_shell_cmd_repeatdelay(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_repeat_delay = esif_atoi(argv[1]);
	g_repeat_delay = esif_ccb_min(esif_ccb_max(g_repeat_delay, 0), MAX_REPEAT_DELAY);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "repeat delay = %d msec between each repeated command\n", g_repeat_delay);
	return output;
}


// Set Buffer
static char *esif_shell_cmd_setb(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_binary_buf_size = esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "binary_buf_size=%d\n", g_binary_buf_size);
	return output;
}


// Set Error level
static char *esif_shell_cmd_seterrorlevel(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_errorlevel = esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "seterrorlevel = %d\n", g_errorlevel);
	return output;
}


// Set Primitive
static char *esif_shell_cmd_setp(EsifShellCmdPtr shell)
{
	int argc            = shell->argc;
	char **argv         = shell->argv;
	char *output        = shell->outbuf;
	enum esif_rc rc     = ESIF_OK;
	EsifString desc     = ESIF_NOT_AVAILABLE;
	u32 id = 0;
	u16 qualifier       = EVENT_MGR_DOMAIN_D0;
	char *qualifier_str = NULL;
	u8 instance         = 0;
	u8 *data_ptr        = NULL;
	char *suffix        = "";
	enum esif_data_type type = ESIF_DATA_VOID;
	int opt = 1;
	u32 data_len = 0;
	u32 buf_len = 0;
	struct esif_data request;
	struct esif_data response = {ESIF_DATA_VOID, NULL, 0};
	u32 add_data;
	EsifDataPtr dataValue = NULL;
	
	if (argc < 5) {
		return NULL;
	}

	// Deduce suffix and type options from command
	if (esif_ccb_stricmp(argv[0], "setp") == 0) {
		suffix = "";
		type   = ESIF_DATA_AUTO;
	} else if (esif_ccb_stricmp(argv[0], "setp_t") == 0) {
		suffix = &argv[0][4];
		type   = ESIF_DATA_TEMPERATURE;
	} else if (esif_ccb_stricmp(argv[0], "setp_pw") == 0) {
		suffix = &argv[0][4];
		type   = ESIF_DATA_POWER;
	}

	// Primitive ID
	id = esif_atoi(argv[opt++]);

	// Qualifier ID
	qualifier_str = argv[opt++];
	qualifier     = domain_str_to_short(qualifier_str);

	// Instance ID
	instance = (u8)esif_atoi(argv[opt++]);
	dataValue = EsifData_Create();
	if (dataValue == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	rc = EsifData_FromString(dataValue, argv[opt++], ESIF_DATA_AUTO);
	if (rc != ESIF_OK || dataValue->buf_ptr == NULL) {
		goto exit;
	}
	data_len = dataValue->data_len;
	buf_len = data_len + sizeof(add_data);
	data_ptr = (u8 *) esif_ccb_malloc(buf_len);
	if (NULL == data_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ccb_malloc failed for %u bytes\n",
			ESIF_FUNC, data_len);
		goto exit;
	}
	esif_ccb_memcpy((data_ptr), dataValue->buf_ptr, dataValue->data_len);

	// Optional, Additional Argument, Integer Only For Now
	if (opt < argc) {
		add_data = esif_atoi(argv[opt++]);
		// Command, req_data = {data, add_data}
		esif_ccb_memcpy((data_ptr + dataValue->data_len), &add_data, sizeof(add_data));
		
	} 
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s ", esif_primitive_str((enum esif_primitive_type)id));

	request.type     = type;
	request.buf_len  = data_len;
	request.buf_ptr  = (void *)data_ptr;
	request.data_len = data_len;

	// Verify this is a SET
	if (EsifPrimitiveVerifyOpcode(
			(u8)g_dst,
			id,
			qualifier_str,
			instance,
			ESIF_PRIMITIVE_OP_SET) == ESIF_FALSE) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Primitive (%d) not a SET: %s\n", id, esif_primitive_str(id));
		rc = ESIF_E_INVALID_REQUEST_TYPE;
		goto exit;
	}

	rc = EsifExecutePrimitive(
			(u8)g_dst,
			id,
			qualifier_str,
			instance,
			&request,
			&response);


	if (ESIF_OK != rc) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "setp%s(%03u.%s.%03d) error code = %s(%d)\n",
						suffix, id, qualifier_str, instance,
						esif_rc_str(rc), rc);

		g_errorlevel = 6;
		goto exit;
	}

	switch (request.type) {
	case ESIF_DATA_UINT32:
		desc = "";
		break;

	case ESIF_DATA_TEMPERATURE:
		desc = "Degrees C";
		break;

	case ESIF_DATA_POWER:
		desc = "MilliWatts";
		break;

	case ESIF_DATA_STRING:
		desc = "ASCII";
		break;

	case ESIF_DATA_BINARY:
		desc = "BINARY";
		break;

	case ESIF_DATA_TABLE:
		desc = "TABLE";
		break;

	case ESIF_DATA_PERCENT:
		desc = "Centi-Percent";
		break;

	case ESIF_DATA_FREQUENCY:
		desc = "Frequency";
		break;

	case ESIF_DATA_TIME:
		desc = "Time (ms)";
		break;

	default:
		desc = "";
		break;
	}

	switch (request.type) {
	case ESIF_DATA_STRING:
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s setp%s(%03u,%s,%03d) value = %s %s\n", esif_primitive_str(
			(enum esif_primitive_type)id), suffix, id, qualifier_str, instance, (char *) dataValue->buf_ptr, desc);
		break;
	default:
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s setp%s(%03u,%s,%03d) value = 0x%08x %u %s\n", esif_primitive_str(
			(enum esif_primitive_type)id), suffix, id, qualifier_str, instance, *(UInt32 *) dataValue->buf_ptr, *(UInt32 *) dataValue->buf_ptr, desc);
		break;
	}

exit:

	if (response.buf_ptr != data_ptr) {
		esif_ccb_free(response.buf_ptr);
	}
	esif_ccb_free(data_ptr);
	EsifData_Destroy(dataValue);
	return output;
}

#define TRACELEVEL_ALL (-1)

// Convert string to trace level
int cmd_trace_parse_level(char *str)
{
	static struct {int level; char *name;} trace_levels[] = {
		{ESIF_TRACELEVEL_FATAL, "FATAL"},
		{ESIF_TRACELEVEL_ERROR, "ERROR"},
		{ESIF_TRACELEVEL_WARN,  "WARN"},
		{ESIF_TRACELEVEL_WARN,  "WARNING"},
		{ESIF_TRACELEVEL_INFO,  "INFO"},
		{ESIF_TRACELEVEL_DEBUG, "DEBUG"},
		{TRACELEVEL_ALL,		"ALL"},
		{0, NULL}
	};
	int result = ESIF_TRACELEVEL_DEFAULT;
	
	if (isdigit(*str)) {
		result = esif_atoi(str);
		result = esif_ccb_min(result, g_traceLevel_max);
	}
	else {
		int j;
		for (j = 0; trace_levels[j].name != NULL; j++) {
			if (esif_ccb_stricmp(trace_levels[j].name, str) == 0) {
				result = trace_levels[j].level;
				break;
			}
		}
	}
	return result;
}

// Convert string(s) to trace module mask
esif_tracemask_t cmd_trace_parse_mask(int argc, char **argv)
{
	esif_tracemask_t bitmask=0;
	int j=0;

	// Convert each argument to route and add it to the bitmask
	for (j=0; j < argc; j++) {
		enum esif_tracemodule module = (enum esif_tracemodule)0;
		esif_tracemask_t thismask=0;
		int invert = 0;
		char *name = argv[j];

		// ~MODULE
		if (*name == '~') {
			name++;
			invert = 1;
		}

		// ALL = All modules
		if (esif_ccb_stricmp(name, "ALL") == 0) {
			thismask = ESIF_TRACEMASK_ALL;
		}
		else {
			module = EsifTraceModule_FromString(name);
			if (module == (enum esif_tracemodule)0 && esif_ccb_stricmp(argv[j], "DEFAULT") != 0) {
				continue;
			}
			thismask = ((esif_tracemask_t)1 << (int)(module));
		}

		if (invert) {
			bitmask = ~thismask;
		}
		else {
			bitmask |= thismask;
		}
	}

	// If no bitmask specified, assume it is a  hex bitmask
	if (bitmask == 0) {
		esif_ccb_sscanf(argv[0], esif_tracemask_fmtx, &bitmask);
	}


	//EsifTraceModule_FromString("x");
	//esif_ccb_sscanf(argv[0], esif_tracemask_fmtx, &bitmask);
	return bitmask;
}

// Convert string(s) to trace route mask
esif_traceroute_t cmd_trace_parse_route(int argc, char **argv)
{
	static struct { esif_traceroute_t bit; char *name; } trace_routes[] = {
		{ ESIF_TRACEROUTE_CONSOLE,	"CON" },
		{ ESIF_TRACEROUTE_CONSOLE,	"CONSOLE" },
		{ ESIF_TRACEROUTE_EVENTLOG,	"EVT" },
		{ ESIF_TRACEROUTE_EVENTLOG,	"EVENTLOG" },
		{ ESIF_TRACEROUTE_DEBUGGER,	"DBG" },
		{ ESIF_TRACEROUTE_DEBUGGER,	"DEBUGGER" },
		{ ESIF_TRACEROUTE_LOGFILE,	"LOG" },
		{ ESIF_TRACEROUTE_CONSOLE|ESIF_TRACEROUTE_EVENTLOG|ESIF_TRACEROUTE_DEBUGGER|ESIF_TRACEROUTE_LOGFILE, "ALL" },
		{ 0, "NONE" },
		{ 0, NULL}
	};
	int j=0;
	int k=0;
	esif_traceroute_t bitmask=0;

	// Convert each argument to route and add it to the bitmask
	for (j=0; j < argc; j++) {
		for (k=0; trace_routes[k].name != NULL; k++) {
			if (esif_ccb_stricmp(trace_routes[k].name, argv[j]) == 0) {
				bitmask |= trace_routes[k].bit;
			}
			else if (argv[j][0] == '~' && esif_ccb_stricmp(trace_routes[k].name, &argv[j][1]) == 0) {
				bitmask = ~(trace_routes[k].bit);
			}
		}
	}

	// If no bitmask specified, assume it is a  hex bitmask
	if (bitmask == 0) {
		unsigned int tmpmask=0;
		esif_ccb_sscanf(argv[0], "%x", &tmpmask);
		bitmask = (esif_traceroute_t)tmpmask;
	}
	return bitmask;
}

// Trace
char *esif_shell_cmd_trace(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	// trace <level>
	// trace level <level>
	if ((argc == 2 && isdigit(argv[1][0])) || (argc > 2 && esif_ccb_stricmp(argv[1], "level")==0)) {
		UInt32 verbosity = 0;
		EsifData verbosity_data = {ESIF_DATA_UINT32,
								   &verbosity,
								   sizeof(verbosity),
								   sizeof(verbosity)};
		int level = (u8)cmd_trace_parse_level((argc == 2 ? argv[1] : argv[2]));

		// Set new trace level and notify DPTF
		g_traceLevel = esif_ccb_min(g_traceLevel_max, level);
		verbosity = (UInt32)g_traceLevel;
		EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_LOG_VERBOSITY_CHANGED, &verbosity_data);
		CMD_OUT("\ntrace level %d\n", g_traceLevel);
	}
	// trace module <level> <bitmask>
	// trace module <level> [<module> ...]
	else if (argc > 3 && (esif_ccb_stricmp(argv[1], "module")==0 || esif_ccb_stricmp(argv[1], "mask")==0)) {
		int level = cmd_trace_parse_level(argv[2]);
		esif_tracemask_t bitmask = cmd_trace_parse_mask(argc-3, &argv[3]);

		if (level == TRACELEVEL_ALL) {
			int j;
			for (j=0; j <= g_traceLevel_max; j++)
				g_traceinfo[j].modules = bitmask;
		}
		else
			g_traceinfo[level].modules = bitmask;
	}
	// trace route <level> <bitmask>
	// trace route <level> [<route> ...]
	else if (argc > 3 && esif_ccb_stricmp(argv[1], "route")==0) {
		int level = cmd_trace_parse_level(argv[2]);
		esif_traceroute_t bitmask = cmd_trace_parse_route(argc-3, &argv[3]);

		if (level == TRACELEVEL_ALL) {
			int j;
			for (j=0; j <= g_traceLevel_max; j++)
				g_traceinfo[j].routes = bitmask;
		}
		else
			g_traceinfo[level].routes = bitmask;
	}
	// trace test [level]
	else if (argc > 1 && esif_ccb_stricmp(argv[1], "test")==0) {
		int level=-1;
		if (argc > 2)
			level = cmd_trace_parse_level(argv[2]);
		if (argc==2 || level==ESIF_TRACELEVEL_ERROR)	ESIF_TRACE_ERROR("ERROR Message(1)");
		if (argc==2 || level==ESIF_TRACELEVEL_WARN)		ESIF_TRACE_WARN("WARN Message(2)");
		if (argc==2 || level==ESIF_TRACELEVEL_INFO)		ESIF_TRACE_INFO("INFO Message(3)");
		if (argc==2 || level==ESIF_TRACELEVEL_DEBUG)	ESIF_TRACE_DEBUG("DEBUG Message(4)");
	}
	// trace log [ [open] <file> [append] | [close] | [write] <msg> ]
	else if (argc > 2 && esif_ccb_stricmp(argv[1], "log")==0) {
		char *file = argv[2];

		// trace log close
		if (esif_ccb_stricmp(file, "close")==0) {
			shell->argc=3;
			shell->argv[0]="log";
			shell->argv[1]="close";
			shell->argv[2]="trace";
			return esif_shell_cmd_log(shell);
		}
		// trace log write <msg>
		if (argc > 3 && esif_ccb_stricmp(file, "write")==0) {
			char *msg = argv[3];
			size_t msglen = esif_ccb_strlen(msg, OUT_BUF_LEN) + MAX_CTIME_LEN;
			char *timestamp_msg = (char *)esif_ccb_malloc(msglen);
			time_t now=0;
			char timestamp[MAX_CTIME_LEN]={0};

			time(&now);
			esif_ccb_ctime(timestamp, sizeof(timestamp), &now);
			timestamp[20] = 0; // truncate year
			esif_ccb_sprintf(msglen, timestamp_msg, "%s%s", timestamp+4, msg);

			shell->argc=4;
			shell->argv[0]="log";
			shell->argv[1]="write";
			shell->argv[2]="trace";
			shell->argv[3]= timestamp_msg;
			output = esif_shell_cmd_log(shell);
			esif_ccb_free(timestamp_msg);
			return output;
		}
		// trace log open <file> [append]
		else if (argc > 3 && esif_ccb_stricmp(file, "open")==0) {
			file = argv[3];
		}
		shell->argc=4;
		shell->argv[0]="log";
		shell->argv[1]="open";
		shell->argv[2]="trace";
		shell->argv[3]= file;
		if (argc > 4) {
			shell->argv[4] = argv[4];
			shell->argc++;
		}
		return esif_shell_cmd_log(shell);
	}
	// trace nolog
	else if (argc > 1 && esif_ccb_stricmp(argv[1], "nolog")==0) {
		char *newargv[MAX_ARGV]={"log", "close", "trace", argv[3]};
		shell->argc = 4;
		shell->argv = newargv;
		output = esif_shell_cmd_log(shell);
		shell->argc = argc;
		shell->argv = argv;
		return output;
	}
#ifdef ESIF_ATTR_MEMTRACE
	// trace leak
	else if (argc > 1 && esif_ccb_stricmp(argv[1], "leak")==0) {
		// create intentional memory leak for debugging
		u32 *memory_leak = (u32*)esif_ccb_malloc(sizeof(*memory_leak));
		UNREFERENCED_PARAMETER(memory_leak);
	}
#endif

	//////////////////

	// trace
	if (argc > 0) {
		int j;
		CMD_OUT("\nLevel\tLabel\tModules    Routes Targets\n");
		CMD_OUT("-----\t-----\t---------- ------ --------------------------------------------\n");
		for (j=0; j <= g_traceLevel_max; j++) {
			int k;
			CMD_OUT("%s %d\t%s\t0x%08X 0x%02X   ", (g_traceLevel==j ? "*" : " "), g_traceinfo[j].level,
					g_traceinfo[j].label, g_traceinfo[j].modules, g_traceinfo[j].routes);
			if (g_traceinfo[j].routes) {
				if (g_traceinfo[j].routes & ESIF_TRACEROUTE_CONSOLE) {
					CMD_OUT("CON ");
				}
				if (g_traceinfo[j].routes & ESIF_TRACEROUTE_EVENTLOG) {
					CMD_OUT("EVT ");
				}
				if (g_traceinfo[j].routes & ESIF_TRACEROUTE_DEBUGGER) {
					CMD_OUT("DBG ");
				}
				if (g_traceinfo[j].routes & ESIF_TRACEROUTE_LOGFILE) {
					CMD_OUT("LOG%s ", (EsifLogFile_IsOpen(ESIF_LOG_TRACE) ? "" : "(?)"));
				}
			}
			CMD_OUT("<< ");
			if (g_traceinfo[j].modules == ESIF_TRACEMASK_ALL)
				CMD_OUT("*");
			else {
				for (k=0; k < ESIF_TRACEMASK_MAX; k++) {
					if (g_traceinfo[j].modules & ((esif_tracemask_t)1 << k)) {
						const char *modstr = EsifTraceModule_ToString((enum esif_tracemodule)k);
						if (modstr != NULL)
							CMD_OUT("%s ", modstr);
					}
				}
			}
			CMD_OUT("\n");
		}
		CMD_OUT("\n");
	}
	return output;
}


static char *esif_shell_cmd_set_osc(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	struct esif_data_complex_osc osc = {0};
	struct esif_data_complex_osc *ret_osc;
	const u32 data_len = sizeof(struct esif_data_complex_osc);
	u8 *data_ptr = NULL;
	u32 id;
	struct esif_data request = {ESIF_DATA_VOID};
	struct esif_data response = {ESIF_DATA_VOID};
	char guid_buf[ESIF_GUID_PRINT_SIZE];

	// Use Active For NOW
	esif_guid_t policy_guids[] = {
		{0xd6, 0x41, 0xa4, 0x42, 0x6a, 0xae, 0x2b, 0x46, 0xa8, 0x4b, 0x4a, 0x8c, 0xe7, 0x90, 0x27, 0xd3},
		{0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef}
	};

	if (argc < 3) {
		return NULL;
	}

	// guid_index
	id = esif_atoi(argv[1]);

	// Capabiity
	osc.capabilities = esif_atoi(argv[2]);

	if (id > 1) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: please select guid or fail guid ID\n",
						 ESIF_FUNC);
		goto exit;
	}

	data_ptr = (u8 *)esif_ccb_malloc(data_len);
	if (NULL == data_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ccb_malloc failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	request.type      = ESIF_DATA_STRUCTURE;
	request.buf_len   = data_len;
	request.buf_ptr   = (u8 *)data_ptr;
	request.data_len  = data_len;

	response.type     = ESIF_DATA_STRUCTURE;
	response.buf_len  = data_len;
	response.buf_ptr  = (u8 *)data_ptr;
	response.data_len = 0;

	// Command
	esif_ccb_memcpy(&osc.guid, &policy_guids[id], ESIF_GUID_LEN);
	osc.revision = 1;
	osc.count    = 2;
	esif_ccb_memcpy(data_ptr, &osc, data_len);


	/* Display Command Data */
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: guid: %s revision %d argcount = %d capabilities %08x\n",
					 ESIF_FUNC, esif_guid_print(&osc.guid, guid_buf),
					 osc.revision, osc.count, osc.capabilities);

	rc = EsifExecutePrimitive(
			(u8)g_dst,
			SET_OPERATING_SYSTEM_CAPABILITIES,
			"D0",
			255,
			&request,
			&response);

	// Get status in esif_data_complex_osc
	ret_osc = (struct esif_data_complex_osc *)(response.buf_ptr);

	if (ESIF_OK != rc) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s: set_osc error code = %s(%d), status code = %d\n",
						 ESIF_FUNC,
						 esif_rc_str(rc), rc,
						 ret_osc->status);
		g_errorlevel = 6;
		goto exit;
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s: set_osc returned status %u\n",
					 ESIF_FUNC, ret_osc->status);

exit:

	if (response.buf_ptr != data_ptr) {
		esif_ccb_free(response.buf_ptr);
	}
	esif_ccb_free(data_ptr);
	return output;
}


static char *esif_shell_cmd_set_scp(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;

	struct esif_data_complex_scp scp = {0};
	const u32 data_len = sizeof(scp);
	u8 *data_ptr = NULL;
	struct esif_data request;
	struct esif_data response = {ESIF_DATA_VOID, NULL, 0};

	if (argc < 4) {
		return NULL;
	}

	// Cooling Mode
	scp.cooling_mode = esif_atoi(argv[1]);

	// Power Limit
	scp.power_limit = esif_atoi(argv[2]);

	// Acoustic Limit
	scp.acoustic_limit = esif_atoi(argv[3]);

	data_ptr = (u8 *)esif_ccb_malloc(data_len);
	if (NULL == data_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "esif_ipc_alloc_command failed for %u bytes\n",
						 data_len);
		goto exit;
	}

	request.type     = ESIF_DATA_STRUCTURE;
	request.buf_ptr  = data_ptr;
	request.buf_len  = data_len;
	request.data_len = data_len;

	// Command
	esif_ccb_memcpy(data_ptr, &scp, data_len);

	rc = EsifExecutePrimitive(
			(u8)g_dst,
			SET_COOLING_POLICY,
			"D0",
			255,
			&request,
			&response);

	if (ESIF_OK != rc) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "set_scp error code = %s(%d)\n",
						 esif_rc_str(rc), rc);
		g_errorlevel = 6;
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "set_scp cooling mode %u, power limit %u, acoustic limit %u\n",
					 scp.cooling_mode, scp.power_limit, scp.acoustic_limit);
exit:

	esif_ccb_free(response.buf_ptr);
	esif_ccb_free(data_ptr);
	return output;
}

static char *esif_shell_cmd_set_dscp(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;

	struct esif_data_complex_dscp dscp = {0};
	const u32 data_len = sizeof(dscp);
	u8 *data_ptr = NULL;
	struct esif_data request;
	struct esif_data response = {ESIF_DATA_VOID, NULL, 0};

	if (argc < 7) {
		return NULL;
	}

	// Version
	dscp.version = esif_atoi(argv[1]);

	// Cooling Mode
	dscp.cooling_mode = esif_atoi(argv[2]);

	// Power Limit
	dscp.power_limit = esif_atoi(argv[3]);

	// Acoustic Limit
	dscp.acoustic_limit = esif_atoi(argv[4]);

	// Workload hint
	dscp.workload_hint = esif_atoi(argv[5]);

	// Device state hint
	dscp.device_state_hint = esif_atoi(argv[6]);

	data_ptr = (u8 *)esif_ccb_malloc(data_len);
	if (NULL == data_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "esif_ipc_alloc_command failed for %u bytes\n",
						 data_len);
		goto exit;
	}

	request.type     = ESIF_DATA_STRUCTURE;
	request.buf_ptr  = data_ptr;
	request.buf_len  = data_len;
	request.data_len = data_len;

	// Command
	esif_ccb_memcpy(data_ptr, &dscp, data_len);

	rc = EsifExecutePrimitive(
			(u8)g_dst,
			SET_DPTF_COOLING_POLICY,
			"D0",
			255,
			&request,
			&response);

	if (ESIF_OK != rc) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "set_dscp error code = %s(%d)\n",
						 esif_rc_str(rc), rc);
		g_errorlevel = 6;
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "set_dscp version %u, cooling mode %u, power limit %u, acoustic limit %u, workload hint %u device state hint %u\n",
					 dscp.version, dscp.cooling_mode, dscp.power_limit, dscp.acoustic_limit, dscp.workload_hint, dscp.device_state_hint);

exit:
	esif_ccb_free(response.buf_ptr);
	esif_ccb_free(data_ptr);
	return output;
}


// Test
static char *esif_shell_cmd_test(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_command_get_participants *data_ptr = NULL;
	int start_id = 0;
	int stop_id  = 0;
	int i;
	char *buf    = 0;

	if (argc < 2) {
		return NULL;
	}
	buf = argv[1];

	if (!strncmp(buf, "all", 3)) {
		start_id = 0;
	} else {
		start_id = esif_atoi(buf);
		stop_id  = start_id + 1;
	}

	{
		const u32 data_len = sizeof(struct esif_command_get_participants);
		struct esif_ipc_command *command_ptr = NULL;

		struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
		if (NULL == ipc_ptr || NULL == command_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
							 ESIF_FUNC, data_len);
			goto exit;
		}

		command_ptr->type = ESIF_COMMAND_TYPE_GET_PARTICIPANTS;
		command_ptr->req_data_type   = ESIF_DATA_VOID;
		command_ptr->req_data_offset = 0;
		command_ptr->req_data_len    = 0;
		command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
		command_ptr->rsp_data_offset = 0;
		command_ptr->rsp_data_len    = data_len;

		ipc_execute(ipc_ptr);

		if (ESIF_OK != ipc_ptr->return_code) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
							 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
			goto exit;
		}

		if (ESIF_OK != command_ptr->return_code) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
							 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
			goto exit;
		}

		// Our Data - List Of Participants
		data_ptr = (struct esif_command_get_participants *)(command_ptr + 1);

		if (start_id != stop_id - 1) {
			stop_id = data_ptr->count;
		}

		g_errorlevel = 0;
		// Loop thorugh from start to finish participants


		for (i = start_id; i < stop_id; i++) {
				#define COMMAND_LEN (32 + 1)
			char command[COMMAND_LEN];
			if (data_ptr->participant_info[i].state == ESIF_PM_PARTICIPANT_STATE_CREATED) {
				esif_ccb_sprintf(COMMAND_LEN, command, "dst %d", i);
				parse_cmd(command, ESIF_FALSE);

				esif_ccb_sprintf(COMMAND_LEN, command, "loadtst %s.tst", data_ptr->participant_info[i].dsp_code);
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Run Test: %s DUT %d\n", command, g_dst);
				parse_cmd(command, ESIF_FALSE);

				if (g_soe && g_errorlevel != 0) {
					break;
				}
			}
		}
exit:
		if (NULL != ipc_ptr) {
			esif_ipc_free(ipc_ptr);
		}
	}
	return output;
}


// Timestamp Mode
static char *esif_shell_cmd_timestamp(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *ts     = 0;

	if (argc < 2) {
		return NULL;
	}
	ts = argv[1];

	if (!strcmp(ts, "on")) {
		g_timestamp = 1;
	} else {
		g_timestamp = 0;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "timestamp=%d\n", g_timestamp);
	return output;
}


///////////////////////////////////////////////////////////////////////////////
// COMMANDS
///////////////////////////////////////////////////////////////////////////////

// Debug show
static char *esif_shell_cmd_debugshow(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_command_get_debug_module_level *data_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_debug_module_level);
	struct esif_ipc_command *command_ptr = NULL;
	u32 i = 0;
	char *state = NULL;
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_DEBUG_MODULE_LEVEL;
	command_ptr->req_data_type   = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 0;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// our data
	data_ptr = (struct esif_command_get_debug_module_level *)(command_ptr + 1);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "TraceLevel = %d\nModules = 0x%08X\n", data_ptr->tracelevel, data_ptr->modules);

	// Once for each BIT
	for (i = 0; i < 32; i++) {
		u32 bit = 1;
		bit = bit << i;
		if (data_ptr->modules & 0x1) {
			state = (char *)"ENABLED";
		} else {
			state = (char *)"DISABLED";
		}

		data_ptr->modules = data_ptr->modules >> 1;
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Module: %3s(%02u) Bit: 0x%08X State: %8s Level: 0x%08X\n",
						 esif_debug_mod_str((enum esif_debug_mod)i), i, bit, state, data_ptr->levels[i]);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// About
static char *esif_shell_cmd_about(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (g_format == FORMAT_TEXT) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\n"
						 "ESIF - Eco-System Independent Framework\n"
						 "Copyright (c) 2013-2016 Intel Corporation All Rights Reserved\n"
						 "\n"
						 "esif_uf - ESIF Upper Framework (UF) R3\n"
						 "Version:  %s\n"
						 "OS: %s\n"
						 "\n"
						 "ESIF Framework Modules:\n"
						 "ID Module      Description               Version \n"
						 "-- ----------- ------------------------- ----------\n"
						 "1  esif_shell  ESIF Command Interface    %s\n"
						 "2  etf         ESIF Test Framework       %s\n"
						 "\n"
						 "esif_lf - ESIF Lower Framework (LF) R0\n"
						 "ESIF Kernel Driver Information:\n"
						 "Version: %s\n"
						 "\n",
						 ESIF_UF_VERSION,
						 g_os,
						 g_esif_shell_version,
						 g_esif_etf_version,
						 g_esif_kernel_version

						 );
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<about>\n"
						 "  <ufVersion>%s</ufVersion>\n"
						 "  <lfVersion>%s</lfVersion>\n"
						 "  <osType>%s</osType>\n"
						 "  <shellVersion>%s</shellVersion>\n"
						 "  <etfVersion>%s</etfVersion>\n"
						 "</about>\n",
						 ESIF_UF_VERSION,
						 g_esif_kernel_version,
						 g_os,
						 g_esif_shell_version,
						 g_esif_etf_version);
	}

	return output;
}


// Exit
static char *esif_shell_cmd_exit(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	// Ignore commands from REST API
	if (g_isRest) {
		return NULL;
	}

	g_disconnectClient = ESIF_TRUE;
#ifndef ESIF_ATTR_OS_WINDOWS
	g_quit = ESIF_TRUE;
#endif

	esif_ccb_sprintf(OUT_BUF_LEN, output, "exit\n");
	return output;
}


static char *esif_shell_cmd_tableobject(EsifShellCmdPtr shell)
{
	int opt = 1;
	int targetParticipantId = 0;
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *action = NULL;
	char *targetTable = NULL;
	char *targetDomain = NULL;
	char *targetData = NULL;
	char *dataSource = NULL;
	char *dataMember = NULL;
	TableObject tableObject = {0};
	EsifDataPtr namespacePtr = NULL;
	EsifDataPtr pathPtr = NULL;
	EsifDataPtr responsePtr = NULL;
	eEsifError rc = ESIF_OK;
	enum tableMode mode = GET;

	/* 
	minimum 5 parameters; 
	format: tableobject <action> <tablename> <participant> <domain> [data]
	if the table expects a revision, the input string should be
	<revision number>:<data>, with the revision number occupying a
	char length of REVISION_INDICATOR_LENGTH 
	*/

	if (argc < 5) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Too few parameters \n");
		goto exit;
	}
	
	action = argv[opt++];
	targetTable = argv[opt++];
	targetParticipantId = esif_atoi(argv[opt++]);
	targetDomain = argv[opt++];

	/* determine if there is an alternative datasource other than the primitive*/
	if ((esif_ccb_stricmp(action, "get") == 0 && argc > 5) || (esif_ccb_stricmp(action, "set") == 0 && argc > 6)) {
		dataSource = argv[opt++];
		dataMember = argv[opt++];
	}

	/* determine whether we are intending to write data */
	if (esif_ccb_stricmp(action, "set") == 0) {
		if (argc < opt + 1) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Too few parameters \n");
			goto exit;
		}
		targetData = argv[opt++];
		mode = SET;
	}
	
	TableObject_Construct(&tableObject, targetTable, targetDomain, dataSource, dataMember, targetData, targetParticipantId, mode);
	
	rc = TableObject_LoadAttributes(&tableObject); /* properties such as table type (binary/virtual/datavault) */
	rc = TableObject_LoadData(&tableObject); /* determines the version, and in the case of GET will apply binary data */
	rc = TableObject_LoadSchema(&tableObject); /* get fields for table (dependant on version) */

	if (rc != ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Unable to load schema \n");
		goto exit;
	}
	
	// tableobject get <tablename> <participant> <domain>
	if (esif_ccb_stricmp(action, "get") == 0) {
		rc = TableObject_LoadXML(&tableObject);
		if(ESIF_OK != rc) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Primitive failed.\n");
			goto exit;
		}
		if (tableObject.dataXML != NULL) {
			esif_ccb_strcpy(output, tableObject.dataXML, esif_ccb_strlen(tableObject.dataXML, OUT_BUF_LEN));
		}
	}
	// tableobject set <tablename> <participant> <domain> "comma,delimited,columns!bang,delimited,rows"
	else if (esif_ccb_stricmp(action, "set") == 0) {
		rc = TableObject_Save(&tableObject);
		
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Unable to save table \n");
			goto exit;
		}	
	}
	// tableobject delete <tablename> <participant> <domain>
	else if (esif_ccb_stricmp(action, "delete") == 0  && argc >= opt) {
		if (argc < opt) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Too few parameters \n");
			goto exit;
		}
		
		rc = TableObject_Delete(&tableObject);
		
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Unable to delete table \n");
			goto exit;
		}	
	}

exit:
	TableObject_Destroy(&tableObject);
	EsifData_Destroy(namespacePtr);
	EsifData_Destroy(pathPtr);
	EsifData_Destroy(responsePtr);
	return output;
}


// Get Buffer Size
static char *esif_shell_cmd_getb(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "binary_buf_size=%d\n", g_binary_buf_size);
	return output;
}


// Get Error Level
static char *esif_shell_cmd_geterrorlevel(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	if (g_errorlevel <= 0) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "geterrorlevel = %s(%d)\n", esif_rc_str((enum esif_rc)-(g_errorlevel)), g_errorlevel);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "geterrorlevel = %s(%d)\n", EsifTestErrorStr((eEsifTestErrorType)g_errorlevel), g_errorlevel);
	}
	return output;
}


// Command Info
static char *esif_shell_cmd_info(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	esif_ccb_sprintf(OUT_BUF_LEN, output, "ESIF_UF GMIN Build Version = %s\n", ESIF_UF_VERSION);
#else
	const u32 data_len = sizeof(struct esif_command_get_kernel_info);
	struct esif_ipc_command *command_ptr = NULL;
	struct esif_command_get_kernel_info *data_ptr = NULL;
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_KERNEL_INFO;
	command_ptr->req_data_type   = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 0;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	if (ipc_execute(ipc_ptr) < 0) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc execute error\n", ESIF_FUNC);
		goto exit;
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// Our Data
	data_ptr = (struct esif_command_get_kernel_info *)(command_ptr + 1);

	if (FORMAT_TEXT == g_format) {
		// ** Do not change this message format unless you also update extract_kernel_version() **
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Kernel Version = %s\n", data_ptr->ver_str);
	} 
	else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<info>\n"
						 "  <kernelVersion>%s</kernelVersion>\n"
						 "</info>\n", data_ptr->ver_str);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
#endif
	return output;
}


// Backwards compatibility for calling from ipc_connect()
char *esif_cmd_info(char *output)
{
	char *argv[] = {"info", 0};
	EsifShellCmd shell;
	shell.argc   = sizeof(argv) / sizeof(char *);
	shell.argv   = argv;
	shell.outbuf = output;
	return esif_shell_cmd_info(&shell);
}


// Participants
static char *esif_shell_cmd_participantsk(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_command_get_participants *data_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_participants);
	struct esif_ipc_command *command_ptr = NULL;
	u32 i     = 0;
	u32 count = 0;
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_PARTICIPANTS;
	command_ptr->req_data_type   = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 0;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// out data
	data_ptr = (struct esif_command_get_participants *)(command_ptr + 1);
	count    = data_ptr->count;

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nKERNEL PARTICIPANTS:\n\n"
						 "ID   Name     Description                     Enum Ver Status       Active DSP     \n"
						 "---- -------- ------------------------------- ---- --- ------------ -----------------\n");
	} else {// XML
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<participants>\n");
	}


	for (i = 0; i < count; i++) {
		char *enumStr  = (char *)"NA";
		char *state_str = esif_lp_state_str((enum esif_pm_participant_state)data_ptr->participant_info[i].state);

		switch (data_ptr->participant_info[i].bus_enum) {
		case 0:
			enumStr = (char *)"ACPI";
			break;

		case 1:
			enumStr = (char *)"PCI";
			break;

		case 2:
			enumStr = (char *)"PLAT";
			break;

		case 3:
			enumStr = (char *)"CNJR";
			break;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"  %-2d %-8s %-31s %4s  %d  %-12s",
				data_ptr->participant_info[i].id,
				data_ptr->participant_info[i].name,
				data_ptr->participant_info[i].desc,
				enumStr,
				data_ptr->participant_info[i].version,
				state_str);

			if (data_ptr->participant_info[i].state == ESIF_LP_STATE_REGISTERED) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					" %s(%d.%d)\n",
					data_ptr->participant_info[i].dsp_code,
					data_ptr->participant_info[i].dsp_ver_major,
					data_ptr->participant_info[i].dsp_ver_minor);
			} else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
			}
		} else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"<participant>\n"
				"  <id>%d</id>\n"
				"  <name>%s</name>\n"
				"  <desc>%s</desc>\n"
				"  <enum>%d</enum>\n"
				"  <enumStr>%s</enumStr>\n"
				"  <version>%d</version>\n"
				"  <state>%d</state>\n"
				"  <stateStr>%s</stateStr>\n"
				"  <dspCode>%s</dspCode>\n"
				"  <dspVerMajor>%d</dspVerMajor>\n"
				"  <dspVerMinor>%d</dspVerMinor>\n"
				"</participant>\n",
				data_ptr->participant_info[i].id,
				data_ptr->participant_info[i].name,
				data_ptr->participant_info[i].desc,
				data_ptr->participant_info[i].bus_enum,
				enumStr,
				data_ptr->participant_info[i].version,
				data_ptr->participant_info[i].state,
				state_str,
				data_ptr->participant_info[i].dsp_code,
				data_ptr->participant_info[i].dsp_ver_major,
				data_ptr->participant_info[i].dsp_ver_minor);
		}
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	} else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</participants>\n");
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}

// Participants
static char *esif_shell_cmd_participants(EsifShellCmdPtr shell)
{
	eEsifError iterRc = ESIF_OK;
	UfPmIterator upIter = {0};
	EsifUpPtr upPtr = NULL;
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	char *attribute = "";
	u8 domainIndex	= 0;
	EsifDspPtr dspPtr = NULL;
	UInt8 participantId = 0;
	UInt8 lpId = 0;
	EsifUpDataPtr metaPtr = NULL;
	EsifUpDomainPtr domainPtr = NULL;
	UpDomainIterator udIter = { 0 };
	char *shellbuf = NULL;
	size_t shellbuf_len = 0;

	// Qualifier
	if (argc > 1) {
		attribute = argv[1];
	}

	if (g_format == FORMAT_XML) {
		esif_shell_sprintf(&shellbuf_len, &shellbuf, "<result>\n");

		iterRc = EsifUpPm_InitIterator(&upIter);
		if (iterRc != ESIF_OK) {
			esif_shell_sprintf(&shellbuf_len, &shellbuf, "Error getting data\n");
			goto exit;
		}

		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		while (ESIF_OK == iterRc) {
			char desc[ESIF_DESC_LEN + 3] = "";	/* desc... */
			char *enumStr = "NA";
			char instanceStr[8 + 1]     = "NA";
			u8 domainCount   = 1;

			dspPtr = EsifUp_GetDsp(upPtr);
			metaPtr = EsifUp_GetMetadata(upPtr);

			if ((NULL == dspPtr) || (NULL == metaPtr)) {
				iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
				continue;
			}

			domainCount = (UInt8)dspPtr->get_domain_count(dspPtr);

			/* Truncate Description If Large */
			if (ESIF_SHELL_STRLEN(metaPtr->fDesc) > 12) {
				esif_ccb_strcpy(desc, metaPtr->fDesc, 9);
				esif_ccb_sprintf_concat(ESIF_DESC_LEN, desc, "...");
			} else {
				esif_ccb_strcpy(desc, metaPtr->fDesc, 12);
			}

			switch (metaPtr->fEnumerator) {
			case ESIF_PARTICIPANT_ENUM_ACPI:
				enumStr = (char *)"ACPI";
				break;

			case ESIF_PARTICIPANT_ENUM_PCI:
				enumStr = (char *)"PCI";
				break;

			case ESIF_PARTICIPANT_ENUM_PLAT:
				enumStr = (char *)"PLAT";
				break;

			case ESIF_PARTICIPANT_ENUM_CONJURE:
				enumStr = (char *)"CNJR";
				break;
			}

			participantId = EsifUp_GetInstance(upPtr);

			lpId = EsifUp_GetLpInstance(upPtr);
			if (lpId != ESIF_INSTANCE_INVALID) {
				esif_ccb_sprintf(8, instanceStr, "%-2d", lpId);
			}

			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
				"  <participant>\n"
				"    <UpId>%u</UpId>\n"
				"    <LpId>%s</LpId>\n"
				"    <name>%s</name>\n"
				"    <shortDesc>%s</shortDesc>\n"
				"    <desc>%s</desc>\n"
				"    <enum>%u</enum>\n"
				"    <enumStr>%s</enumStr>\n"
				"    <version>%u</version>\n"
				"    <activeDsp>%s</activeDsp>\n"
				"    <acpiHID>%s</acpiHID>\n"
				"    <acpiUID>%s</acpiUID>\n"
				"    <acpiScope>%s</acpiScope>\n"
				"    <acpiType>%u</acpiType>\n"
				"    <domainCount>%u</domainCount>\n",
				participantId,
				instanceStr,
				metaPtr->fName,
				metaPtr->fDesc,
				desc,
				metaPtr->fEnumerator,
				enumStr,
				metaPtr->fVersion,
				dspPtr->code_ptr,
				metaPtr->fAcpiDevice,
				metaPtr->fAcpiUID,
				metaPtr->fAcpiScope,
				metaPtr->fAcpiType,
				domainCount);

			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
				"    <domains>\n");
			

			iterRc = EsifUpDomain_InitIterator(&udIter, upPtr);
			if (ESIF_OK != iterRc)
				goto domain_exit;

			iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			while (ESIF_OK == iterRc) {
				char guid_buf[ESIF_GUID_PRINT_SIZE];

				if (NULL == domainPtr) {
					iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
					continue;
				}

				esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
					"      <domain>\n"
					"        <id>%d</id>\n"
					"        <version>1</version>\n"
					"        <name>%s</name>\n"
					"        <guid>%s</guid>\n"
					"        <type>%d</type>\n"
					"        <capability>0x%x</capability>\n"
					"      </domain>\n",
					domainIndex, domainPtr->domainName,
					esif_guid_print((esif_guid_t *) domainPtr->domainGuid, guid_buf),
					domainPtr->domainType,
					domainPtr->capability_for_domain.capability_flags);

				iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			}

domain_exit:
			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
				"    </domains>\n"
				"  </participant>\n");

			iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		}

		esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "</result>\n");
		goto exit;
	}

	/* Create appropriate header */
	if (!strcmp(attribute, "acpi")) {
		esif_shell_sprintf(&shellbuf_len, &shellbuf,
			"\n"
			"ALL PARTICIPANTS: ACPI INFORMATION\n"
			"\n"
			"UPID LPID Name Description  Enum Ver HID        SCOPE                          UID  Type\n"
			"---- ---- ---- ------------ ---- --- ---------- ------------------------------ ---- ----\n");
	} else {
		esif_shell_sprintf(&shellbuf_len, &shellbuf,
			"\n"
			"ALL PARTICIPANTS:\n"
			"\n"
			"UPID LPID Name Description  Enum Ver Active DSP DC\n"
			"---- ---- ---- ------------ ---- --- ---------- --\n");
	}

	iterRc = EsifUpPm_InitIterator(&upIter);
	if (iterRc != ESIF_OK) {
		esif_shell_sprintf(&shellbuf_len, &shellbuf, "Error getting participant information\n");
		goto exit;
	}

	iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	while (ESIF_OK == iterRc) {
		char desc[ESIF_DESC_LEN + 3] = "";	/* desc... */
		char *enumStr = "NA";
		char instanceStr[8 + 1]     = "NA";
		u8 domainCount   = 1;

		dspPtr = EsifUp_GetDsp(upPtr);
		metaPtr = EsifUp_GetMetadata(upPtr);

		if ((NULL == dspPtr) || (NULL == metaPtr)) {
			iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
			continue;
		}

		domainCount = (UInt8)dspPtr->get_domain_count(dspPtr);

		/* Truncate Description If Large */
		if (ESIF_SHELL_STRLEN(metaPtr->fDesc) > 12) {
			esif_ccb_strcpy(desc, metaPtr->fDesc, 9);
			esif_ccb_sprintf_concat(ESIF_DESC_LEN, desc, "...");
		} else {
			esif_ccb_strcpy(desc, metaPtr->fDesc, 12);
		}

		switch (metaPtr->fEnumerator) {
		case ESIF_PARTICIPANT_ENUM_ACPI:
			enumStr = (char *)"ACPI";
			break;

		case ESIF_PARTICIPANT_ENUM_PCI:
			enumStr = (char *)"PCI";
			break;

		case ESIF_PARTICIPANT_ENUM_PLAT:
			enumStr = (char *)"PLAT";
			break;

		case ESIF_PARTICIPANT_ENUM_CONJURE:
			enumStr = (char *)"CNJR";
			break;
		}

		participantId = EsifUp_GetInstance(upPtr);

		lpId = EsifUp_GetLpInstance(upPtr);
		if (lpId != ESIF_INSTANCE_INVALID) {
			esif_ccb_sprintf(8, instanceStr, "%2d", lpId);
		}

		esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "%4d %4s %-5s%-12s %-4s %3d ",
			participantId,
			instanceStr,
			metaPtr->fName,
			desc,
			enumStr,
			metaPtr->fVersion);

		if (!strcmp(attribute, "acpi")) {
			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "%-10s %-30s %-4s %4d\n",
				metaPtr->fAcpiDevice,
				metaPtr->fAcpiScope,
				metaPtr->fAcpiUID,
				metaPtr->fAcpiType);
		} else {
			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "%-10s %-2d\n",
				dspPtr->code_ptr,
				domainCount);
		}

		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
	esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "\n");

	if (iterRc != ESIF_E_ITERATION_DONE) {
		esif_shell_sprintf(&shellbuf_len, &shellbuf, "Error getting participant information\n");
		goto exit;
	}

exit:
	EsifUp_PutRef(upPtr);
	if (shellbuf != NULL) {
		output = shell->outbuf = esif_shell_resize(shellbuf_len);
		esif_ccb_strcpy(output, shellbuf, OUT_BUF_LEN);
		esif_ccb_free(shellbuf);
	}
	return output;
}


// Quit
static char *esif_shell_cmd_quit(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	// Ignore commands from REST API
	if (g_isRest) {
		return NULL;
	}

	g_disconnectClient = ESIF_TRUE;
#ifndef ESIF_ATTR_OS_WINDOWS
	g_quit = ESIF_TRUE;
#endif

	esif_ccb_sprintf(OUT_BUF_LEN, output, "quit\n");
	return output;
}


// Timer Start
static char *esif_shell_cmd_timerstart(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	esif_ccb_get_time(&g_timer);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "timerstart\n");
	return output;
}


// Timer Stop
static char *esif_shell_cmd_timerstop(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct timeval stop = {0};
	struct timeval result;
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	if (g_timer.tv_sec == 0 && g_timer.tv_usec == 0) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "timer not active\n");
		goto exit;
	}

	esif_ccb_get_time(&stop);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "timerstop\n");
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Start time: %06lu.%06lu\n", 
					 g_timer.tv_sec,
					 g_timer.tv_usec);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Stop time: %06lu.%06lu\n", 
					 stop.tv_sec,
					 stop.tv_usec);

	timeval_subtract(&result, &stop, &g_timer);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Time: %06lu.%06lu (%lu seconds + %lu msec + %lu usec)\n", 
					 result.tv_sec,
					 result.tv_usec,
					 result.tv_sec,
					 result.tv_usec / 1000,
					 result.tv_usec % 1000);

	// Reset For Next Time
	g_timer.tv_sec  = 0;
	g_timer.tv_usec = 0;
exit:
	return output;
}


// Switch System between DPTF/ESIF
void cmd_app_subsystem(enum app_subsystem subsystem)
{
	switch (subsystem) {
	case SUBSYSTEM_ESIF:
		esif_init();
		break;

	default:
		break;
	}
}


// Conjure Participant
static char *esif_shell_cmd_conjure(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *libName;
	EsifCnjPtr a_conjure_ptr = NULL;
	u8 i = 0;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}

	libName = argv[1];

	/* Check to see if the participant is already loaded only one instance per particpipant lib allowed */
	if (NULL != esif_uf_conjure_get_instance_from_name(libName)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Conjure Library %s already started.\n", libName);
		goto exit;
	}

	for (i = 0; i < ESIF_MAX_CONJURES; i++) {
		if (NULL == g_cnjMgr.fEnrtries[i].fLibNamePtr) {
			break;
		}
	}


	if (ESIF_MAX_CONJURES == i) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Maximum Conjures Reached %u.\n", i);
		goto exit;
	} else {
		g_cnjMgr.fEntryCount++;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Conjure Sandbox %u Selected.\n", i);
	}

	a_conjure_ptr = &g_cnjMgr.fEnrtries[i];
	a_conjure_ptr->fLibNamePtr = (esif_string)esif_ccb_strdup(libName);

	rc = EsifConjureStart(a_conjure_ptr);
	if (ESIF_OK != rc) {
		/* Cleanup */
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Start Conjure Library: %s [%s (%d)]\n", libName, esif_rc_str(rc), rc);
		esif_ccb_free(a_conjure_ptr->fLibNamePtr);
		memset(a_conjure_ptr, 0, sizeof(*a_conjure_ptr));
	} else {
		/* Proceed */
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Started Conjure Library: %s Instance %u Max %u Running %u\n\n",
						 libName, i, ESIF_MAX_CONJURES, g_cnjMgr.fEntryCount);
	}

exit:
	return output;
}


static char *esif_shell_cmd_unconjure(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *libName  = 0;
	EsifCnjPtr a_conjure_ptr = NULL;

	if (argc < 2) {
		return NULL;
	}

	/* Parse The Library Name */
	libName = argv[1];

	a_conjure_ptr = esif_uf_conjure_get_instance_from_name(libName);
	if (NULL == a_conjure_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find Conjure Library: %s.\n", libName);
		goto exit;
	}

	rc = EsifConjureStop(a_conjure_ptr);
	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Stop Conjure Library: %s.\n", libName);
	} else {
		/* Proceed */
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Stopped Conjure Library: %s\n\n", libName);
		g_cnjMgr.fEntryCount--;
	}

exit:

	return output;
}

static char *esif_shell_cmd_status(EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char **argv   = shell->argv;
	char *output  = shell->outbuf;
	eEsifError rc = ESIF_OK;
	eEsifError iterRc = ESIF_OK;
	char tableName[TABLE_OBJECT_MAX_NAME_LEN] = { 0 };
	UfPmIterator upIter = {0};
	EsifUpPtr upPtr = NULL;
	UInt8 participantId;

	if (argc >= 2) {
		esif_ccb_sprintf(TABLE_OBJECT_MAX_NAME_LEN, tableName, "%s", argv[1]);
	}
	else {
		esif_ccb_sprintf(TABLE_OBJECT_MAX_NAME_LEN, tableName, "%s", "status");
	}
	
// TODO JDH Sync UI To use one cannonical way this is a workaround for now.
#ifdef  ESIF_ATTR_WEBSOCKET
	esif_ccb_sprintf(OUT_BUF_LEN, output, "update:<status>\n");
#else
	esif_ccb_sprintf(OUT_BUF_LEN, output, "<status>\n");
#endif

	iterRc = EsifUpPm_InitIterator(&upIter);
	if (iterRc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</status>\n");
		goto exit;
	}

	iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	while (iterRc == ESIF_OK) {
		EsifUpDomainPtr domainPtr = NULL;
		UpDomainIterator udIter = { 0 };
		EsifDspPtr dspPtr = NULL;

		participantId = EsifUp_GetInstance(upPtr);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
			"<stat>\n"
			"  <id>%d</id>\n"
			"  <name>%s</name>\n",
			participantId,
			EsifUp_GetName(upPtr));

		dspPtr = EsifUp_GetDsp(upPtr);
		if (dspPtr != NULL) {
			iterRc = EsifUpDomain_InitIterator(&udIter, upPtr);
			if (ESIF_OK != iterRc)
				goto exit;

			iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			
			while (ESIF_OK == iterRc) {
				TableObject tableObject = { 0 };
				if (NULL == domainPtr) {
					iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
					continue;
				}

				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  <domain>\n    <name>%s</name>\n", domainPtr->domainName);

				TableObject_Construct(
					&tableObject,
					tableName, 
					domainPtr->domainStr, 
					NULL,
					NULL,
					NULL,
					participantId,
					GET);
				rc = TableObject_LoadSchema(&tableObject);

				if (rc == ESIF_OK) {
					rc = TableObject_LoadXML(&tableObject);
					if (rc == ESIF_OK) {
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output, tableObject.dataXML);
					}
				}	
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  </domain>\n");
				TableObject_Destroy(&tableObject);

				iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			}

			if (iterRc != ESIF_E_ITERATION_DONE) {
				EsifUp_PutRef(upPtr);
			}
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</stat>\n");

		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</status>\n");
exit:
	EsifUp_PutRef(upPtr);	
	return output;
}


static char *esif_shell_cmd_domains(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	UInt32 domainIndex = 0;
	int i = 0;
	EsifUpPtr upPtr = NULL;
	eEsifError iterRc = ESIF_OK;
	EsifUpDomainPtr domainPtr = NULL;
	UpDomainIterator udIter = { 0 };

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	upPtr = EsifUpPm_GetAvailableParticipantByInstance((UInt8)g_dst);
	if (NULL == upPtr) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Unable to get participant, current dst may be invalid\n");
		goto exit;
	}

	iterRc = EsifUpDomain_InitIterator(&udIter, upPtr);
	if (ESIF_OK != iterRc)
		goto exit;

	iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, 
		"\n"
		"DOMAINS:\n"
		"ID Name     Qual PRI Capability Device Type\n"
		"-- -------- ---- --- ---------- ---------------------------------\n");


	while (ESIF_OK == iterRc) {		
		if (NULL == domainPtr) {
			iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			continue;
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
			"%02d %-8s %-4s %d   0x%08x %s(%d)\n",
			domainIndex,
			domainPtr->domainName,
			domainPtr->domainStr,
			domainPtr->domainPriority,
			domainPtr->capability_for_domain.capability_flags,
			esif_domain_type_str((enum esif_domain_type)domainPtr->domainType),
			domainPtr->domainType);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		for (i = 0; i < 32; i++) {	/* TODO:  Limit to actual enum size */
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s(%d): %02X\n", 
							 esif_capability_type_str((enum esif_capability_type)i), i, domainPtr->capability_for_domain.capability_mask[i]);
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");

		iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
	}
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return output;
}


static char *esif_shell_cmd_pst(EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char **argv   = shell->argv;
	char *output  = shell->outbuf;
	eEsifError rc = ESIF_OK;
	UInt8 participant_id = 0;
	UInt32 temp   = 0;
	UInt8 i = 0;
	struct esif_data request  = {ESIF_DATA_VOID, NULL, 0, 0};
	struct esif_data response = {ESIF_DATA_TEMPERATURE, &temp, sizeof(temp), 0};
	char *qualifier_str = "D0";

	if (argc < 2) {
		return NULL;
	}

	participant_id = (UInt8)esif_atoi(argv[1]);

	// Qualifier
	if (argc > 2) {
		qualifier_str = argv[2];
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "<pst>\n"
					 "  <id>%d</id>\n", participant_id);

	for (i = 0; i < 10; i++) {
		temp = 255;
		rc   = EsifExecutePrimitive(participant_id, GET_TRIP_POINT_ACTIVE, qualifier_str, i, &request, &response);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "  <ac%d>%d</ac%d>\n", i, *(UInt32 *)response.buf_ptr, i);
	}

	temp = 255;
	rc   = EsifExecutePrimitive(participant_id, GET_TRIP_POINT_PASSIVE, qualifier_str, 255, &request, &response);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "  <psv>%d</psv>\n", *(UInt32 *)response.buf_ptr);

	temp = 255;
	rc   = EsifExecutePrimitive(participant_id, GET_TRIP_POINT_WARM, qualifier_str, 255, &request, &response);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "  <wrm>%d</wrm>\n", *(UInt32 *)response.buf_ptr);

	temp = 255;
	rc   = EsifExecutePrimitive(participant_id, GET_TRIP_POINT_CRITICAL, qualifier_str, 255, &request, &response);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "  <crt>%d</crt>\n", *(UInt32 *)response.buf_ptr);

	temp = 255;
	rc   = EsifExecutePrimitive(participant_id, GET_TRIP_POINT_HOT, qualifier_str, 255, &request, &response);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "  <hot>%d</hot>\n", *(UInt32 *)response.buf_ptr);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "</pst>\n");

	return output;
}


static char *esif_shell_cmd_event(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	eEsifEventType event_type;
	u8 participant_id   = 0;
	u16 domain_id       = EVENT_MGR_DOMAIN_NA;
	char *qualifier_str = "NA";

	if (argc < 2) {
		return NULL;
	}

	// Event Type
	event_type = (eEsifEventType)esif_atoi(argv[1]);

	// Optional Participant ID
	if (argc > 2) {
		participant_id = (u8)esif_atoi(argv[2]);
	}

	// Optional Domain ID
	if (argc > 3) {
		qualifier_str = argv[3];
		domain_id     = domain_str_to_short(qualifier_str);
	}
	EsifEventMgr_SignalEvent(participant_id, domain_id, event_type, NULL);

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nSEND EVENT %s(%d) PARTICIPANT %d DOMAIN %d\n", esif_event_type_str(event_type), event_type, participant_id, domain_id);

	return output;
}


// Kernel Participant Extensions: eventkpe
static char *esif_shell_cmd_eventkpe(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc_command *command_ptr = NULL;
	struct esif_command_send_kpe_event *req_data_ptr = NULL;
	u32 data_len = sizeof(*req_data_ptr);
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	enum esif_event_type event_type = 0;
	u32 instance = 0;
	u8 event_data_present = ESIF_FALSE;
	u32 event_data = 0;

	if ((NULL == ipc_ptr) || (NULL == command_ptr)) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"esif_ipc_alloc_command failed for %u bytes\n",
			data_len);
		goto exit;
	}

	if (argc < 3) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"Must specify the event type and KPE instance\n");
		goto exit;
	}

	req_data_ptr = (struct esif_command_send_kpe_event *)(command_ptr + 1);

	event_type = esif_atoi(argv[1]);
	instance = esif_atoi(argv[2]);

	if (argc > 3) {
		event_data_present = ESIF_TRUE;
		event_data = esif_atoi(argv[3]);
	}
	req_data_ptr->event_type = event_type;
	req_data_ptr->instance = instance;
	req_data_ptr->data_present = event_data_present;
	req_data_ptr->data = event_data;

	command_ptr->type = ESIF_COMMAND_TYPE_SEND_KPE_EVENT;
	command_ptr->req_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = data_len;
	command_ptr->rsp_data_type   = ESIF_DATA_VOID;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = 0;

	esif_ccb_sprintf(OUT_BUF_LEN, output,
		"Sending KPE event type %s(%d) to KPE %u ",
		esif_event_type_str(event_type),
		event_type,
		instance);

	if (event_data_present) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, ", data = %u: ", event_data);
	} else {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "(no data): ");	
	}

	ipc_execute(ipc_ptr);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "RC = %s(%d)\n",
		esif_rc_str(command_ptr->return_code),
		command_ptr->return_code);	

exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


static char *esif_shell_cmd_help(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	UInt32 capabilityid = 0;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	esif_ccb_sprintf(OUT_BUF_LEN, output, "ESIF CLI Copyright (c) 2013-2016 Intel Corporation All Rights Reserved\n");
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n"
										  "Key:  <>-Required parameters\n"
										  "      []-Optional parameters\n"
										  "       |-Choice of parameters\n"
										  "     ...-Repeated parameters\n"
										  "\n"
										  "GENERAL COMMANDS:\n"
										  "help                                     Displays this text\n"
										  "quit or exit                             Leave\n"
										  "format <xml|text>						Command Output Format (Default=text)\n"
										  "info                                     Get Kernel Version\n"
										  "about                                    List ESIF Information\n"
										  "rem                                      Comment/Remark - ignored\n"
										  "repeat <count>                           Repeat Next Command N Times\n"
										  "repeat_delay <delay>                     Repeat Delay In msec\n"
										  "echo [?] [parameter...]                  Echos Parameters - if ? is used, each\n"
										  "                                         parameter is on a separate line\n"
										  "memstats [reset]                         Show/Reset Memory Statistics\n"
										  "autoexec [command] [...]                 Execute Default Startup Script\n"
										  "\n"
										  "TEST SCRIPT COMMANDS:\n"
										  "load    <filename> [load parameters...]  Load and Execute Command File\n"
										  "loadtst <filename> [load parameters...]  Like 'load' but uses DSP DV for file\n"
										  "cat     <filename> [load parameters...]  Display Command File\n"
										  "proof   <filename> [load parameters...]  Prove Command File Replace Tokens\n"
										  "Load parameters replace tokens ($1...$9) in file. $dst$ replaced by file path\n"
										  "Use 'proof' to check parameter replacements\n"
										  "test <id | all>                          Test By ID or ALL Will Run All Tests\n"
										  "soe  <on|off>                            Stop On Error\n"
										  "seterrorlevel                            Set / Reset Error Level\n"
										  "geterrorlevel                            Get Current Error level\n"
										  "timerstart                               Start Interval Timer\n"
										  "timerstop                                Stop Interval Timer\n"
										  "sleep <msec>                             Sleep for the specified number of milliseconds\n"
										  "\n"
										  "UI COMMANDS:\n"
										  "ui getxslt   [appname]                   Return XSLT Formatting information\n"
										  "ui getgroups [appname]                   Get List Of Groups of Left Hand Tabs\n"
										  "ui getmodulesingroup <appname> <groupId> Get A List Of Modules For The Group\n"
										  "ui getmoduledata <appname> <groupId> <moduleId>\n"
										  "                                         Get Data For The App/Group/Module\n"
										  "\n"
										  "DSP COMMANDS:\n"
										  "dsps                                     List all loaded DSPs\n"
										  "dspquery [name [vendorid [deviceid [enum [ptype [hid [uid]]]]]]] Query for matching DSP\n"
										  "infocpc <filename> [pattern]             Get Dst CPC Information\n"
										  "infofpc <filename> [pattern]             Get Dst FPC Information\n"
										  "\n"
										  "PARTICIPANT COMMANDS:\n"
										  "participants                             List Active Participants\n"
										  "participantsk                            List Kernel Participants\n"
										  "participant  <id>                        Get Participant Information\n"
										  "participantk <id>                        Get Kernel Participant Information\n"
										  "dst  <id>                                Set Target Participant By ID\n"
										  "dstn <name>                              Set Target Participant By Name\n"
										  "domains                                  List Active Domains For Participant\n"
										  "\n"
										  "PRIMITIVE EXECUTION API:\n"
										  "getp <id> [qualifier] [instance] [test]  Execute Get Primitive With Automatic\n"
										  "                                         Return Type and Size\n"
										  "'test' options:\n"
										  "     -l <value>  This is the lower bounds of the testing range\n"
										  "     -u <value>  This is the upper bounds of the testing range\n"
										  "     -b <value>  This is used for primitives that will point to a binary \n"
										  "                 object for a return type. The value specified is the\n"
										  "                 expected maximum size of the binary.\n"
// " -f          This specifies that the test value should be\n"
// "             obtained from a file, rather than a DSP.\n"
// " -s <value>  This is used to seed primitives with a value. It can \n"
// "             also be used to write values to files (by using the -f\n"
// "             argument in conjunction with -s)\n"
// " -d <dir>    This is used to specifiy an export when using a binary.\n"
										  "getp_u32 <id> [qualifier] [instance] [test] Execute U32 Get Primitive\n"
										  "getp_t   <id> [qualifier] [instance] [test] Like getp But Converts Temperature\n"
										  "getp_pw  <id> [qualifier] [instance]        Like getp But Converts Power To mW\n"
										  "getp_s   <id> [qualifier] [instance]        Like getp But Return STRING\n"
										  "getp_b   <id> [qualifier] [instance]        Like getp But Return BINARY DATA\n"
										  "getp_bd  <id> [qualifier] [instance]        Like getp But Dumps Hex BINARY DATA\n"
										  "getp_bs  <id> [qualifier] [instance] [file] Like getp_b But Dumps To File\n"
										  "getp_bt  <id> [qualifier] [instance]        Like getp_bs But Dumps As Table\n"
										  "getf_b   <file>                             Like getp_b But Reads From File\n"
										  "getf_bd  <file>                             Like getp_bd But Reads From File\n"
										  "\n"
										  "setp    <id> <qualifier> <instance> <data>  Execute Set Primitive\n"
										  "setp_t  <id> <qualifier> <instance> <temp>  Execute Set as Temperature (C)\n"
										  "setp_pw <id> <qualifier> <instance> <power> Execute Set as Power (mW)\n"
										  "set_osc <id> <capablities>                  Execute ACPI _OSC Command For\n"
										  "                                            Listed GUIDs Below:\n"
										  "         Active Policy 0 = {0xd6,0x41,0xa4,0x42,0x6a,0xae,0x2b,0x46,\n"
										  "                            0xa8,0x4b,0x4a,0x8c,0xe7,0x90,0x27,0xd3}\n"
										  "         Fail Case     1 = {0xde,0xad,0xbe,0xef,0xde,0xad,0xbe,0xef,\n"
										  "                            0xde,0xad,0xbe,0xef,0xde,0xad,0xbe,0xef}\n"
										  "                                            Capabilities per APCI Spec\n"
										  "set_scp  <cooling> <power> <acoustic>       Execute ACPI _SCP Command\n"
										  "set_dscp <version> <cooling> <power> <acoustic> <workload> <devstate>\n"
										  "                                            Execute ACPI DSCP Command\n"
										  "setb <buffer_size>                          Set Binary Buffer Size\n"
										  "getb                                        Get Binary Buffer Size\n"
										  "idsp [add | delete <uuid>]                  Display, add or remove a UUID from\n"
										  "                                            the IDSP in override.dv\n"
										  "\n"
										  "CONFIG API:\n"
										  "config list                              List Open DataVaults\n"
										  "config open   [@datavault]               Open and Load DataVault\n"
										  "config close  [@datavault]               Close DataVault\n"
										  "config drop   [@datavault]               Drop Closed DataVault\n"
										  "config get    [@datavault] <key>         Get DataVault Key (or wildcard)\n"
										  "config set    [@datavault] <key> <value> [ESIF_DATA_TYPE] [option ...]\n"
										  "                                         Set DataVault Key/Value/Type\n"
										  "config keys   [@datavault] <key>         Enumerate matching Key(s) or wildcard\n"
										  "config delete [@datavault] <key>         Delete DataVault Key (or '*')\n"
										  "config exec   [@datavault] <key>         Execute Script in DataVault Key\n"
										  "config copy   [@datavault] <key> [...] [@targetdv] [option ...]\n"
										  "                                         Copy (and Replace) keys(s) to another DV\n"
										  "config merge  [@datavault] <key> [...] [@targetdv] [option ...]\n"
										  "                                         Merge (No Replace) keys(s) to another DV\n"
										  "config export [@datavault] <key> [...] [@targetdv] [bios|dv]\n"
										  "                                         Export DV keys to BIOS or DV File Format\n"
										  "\n"
										  "EVENT API:\n"
										  "event    <eventType> [participant] [domain]   Send User Mode Event\n" 
										  "eventkpe <eventType> <index> [u32 data]       Send Kernel Event to KPE\n"
										  "                                              index - Index of the KPE based on\n"
										  "                                              the order of driversk (0-based)\n"
										  "\n"
#ifndef ESIF_FEAT_OPT_ACTION_SYSFS
										  "IPC API:\n"
										  "ipcauto                                  Auto Connect/Retry\n"
										  "ipccon                                   Force IPC Connect\n"
										  "ipcdis                                   Force IPC Disconnection\n"
										  "\n"
#endif
										  "APPLICATION MANAGEMENT:\n"
										  "apps                                     List all ESIF hosted Applications\n"
										  "appstart   <application>                 Start an ESIF Application\n"
										  "appstop    <application>                 Stop an ESIF Application\n"
										  "appselect  <application>                 Select a running Application To Manage\n"
										  "appstatus  <application>                 App Status\n"
										  "appenable  <application>                 App Enable\n"
										  "appabout   <application>                 App About\n"
										  "\n"
										  "ACTION MANAGEMENT:\n"
										  "actions                                  List all DSP Actions\n"
										  "actionsk                                 Get Kernel DSP Action Information\n"
										  "actionsu                                 Get User-Mode DSP Action Information\n"
										  "actionstart <action>                     Start a Loadable DSP Action\n"
										  "actionstop  <action>                     Stop a Loadable DSP Action\n"
										  "driversk                                 List Kernel Participant Extensions\n"
										  "driverk <id>                             Get Kernel Participant Extension Info\n"
										  "upes                                     List User-mode Participant Extensions\n"
										  "\n"
										  "CONJURE MANAGEMENT:\n"
										  "conjures                                 List loaded ESIF Conjure Libraries\n"
										  "conjure   <library name>                 Load Upper Framework Conjure Library\n"
										  "unconjure <library name>                 Unload Upper Framework Conjure Library\n"
										  "\n"
										  "USER-MODE PARTICIPANT DATA LOGGING:\n"
										  "datalog                                    Returns status - Started or stopped\n"
										  "datalog schedule [delay [period [list]]]   Start a delayed logging thread (ms \n"
										  "                                           from now, time interval in ms, list \n"
										  "                                           of participant ids or names)\n"
										  "datalog start [period [list]]              Start logging (time interval in ms, \n"
										  "                                           list of participant ids or names)\n"
										  "datalog stop                               Stop data logging\n"
										  "participantlog "PARTICIPANTLOG_CMD_START_STR" [all |[PID DID capMask]...]\n"
										  "                                        Starts data logging for that particular\n"
										  "                                        participant ID,domain and capability ID\n"
										  "                                        combination\n"
										  "                                        all     - Starts data logging for all\n"
										  "                                                  the available participants\n"
										  "                                        PID     - Participant ID (e.g 1,2)|\n"
										  "                                                  Participant Name(e.g TCHG,\n"
										  "                                                  TCPU)\n"
										  "                                        DID     - Domain ID(e.g D0,D1)|all\n"
										  "                                                  (for all domains)\n"
										  "                                        capMask - capability Mask(e.g 0x2100)|\n"
										  "                                                  all(for all capabilities)\n"
										  "                                        If no arguments are specified for start\n"
										  "                                        command, by default logging will start\n"
										  "                                        for all the available participants\n"
										  "                                        Capability Mask Details:\n"
										  );
	for (capabilityid = 0; capabilityid < MAX_CAPABILITY_MASK; capabilityid++) {
		if (!esif_ccb_stricmp(esif_capability_type_str(capabilityid), ESIF_NOT_AVAILABLE)) {
			break;
		}
		else if (esif_ccb_strlen(esif_capability_type_str(capabilityid), MAX_PATH) > sizeof("ESIF_CAPABILITY_TYPE")) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
										  "                                        0x%08X -  %s\n",
										  (1 << capabilityid),
										  esif_capability_type_str(capabilityid) + sizeof("ESIF_CAPABILITY_TYPE")
										  );
		}
	}
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
										  "participantlog "PARTICIPANTLOG_CMD_SCHEDULE_STR" [delay] [all |[PID DID capMask]...]\n"
										  "                                        Start a delayed logging thread ms\n"
										  "                                        from now.\n"
										  "                                        delay - Time interval in ms.Defaults is\n"
										  "                                                5s (5000ms)\n"
										  "                                        Other arguments list descriptions are\n"
										  "                                        same as the start command.\n"
										  "participantlog "PARTICIPANTLOG_CMD_INTERVAL_STR" time            Sets the polling interval for logging\n"
										  "                                        to time specified in ms.\n"
										  "                                        Default is 2s(2000ms)\n"
										  "participantlog "PARTICIPANTLOG_CMD_ROUTE_STR" [target ...] [filename]\n"
										  "                                        Logs the participant data log to the\n"
										  "                                        specified target.The target can be any\n"
										  "                                        of the following\n"
										  "                                        (CONSOLE,EVENTVIEWER,DEBUGGER,FILE,ALL)\n"
										  "                                        Default target is FILE\n"
										  "                                        If all is specified as target, then\n"
										  "                                        log target is set for all the available\n"
										  "                                        targets\n"
										  "                                        Filename is optional and will be\n"
										  "                                        considered only if file is specified\n"
										  "                                        as target and filename should be the\n"
										  "                                        following argument immediately after\n"
										  "                                        it.If no filename is specified, By\n"
										  "                                        default a new file will be created\n"
										  "                                        based on time stamp\n"
										  "                                        e.g,\n"
										  "                                        participant_log_2015-11-24-142412.csv\n"
										  "                                        If no arguments are specified for route\n"
										  "                                        command, by default route will be set\n"
										  "                                        for file and new file will be created\n"
										  "participantlog "PARTICIPANTLOG_CMD_STOP_STR"                     Stops participant data logging if\n"
										  "                                        started already\n"
										  "\n"										  
										  "USER-MODE TRACE LOGGING:\n"
										  "trace                             Show User Mode Trace Settings\n"
										  "trace level  [level]              View or Set Global Trace Level\n"
										  "trace module <level> <module ...> Set Trace Module Bitmask (ALL, NONE, Name)\n"
										  "trace route  <level> <route ...>  Set Trace Routing Bitmask (CON, EVT, DBG, LOG)\n"
										  "trace log open <filename>         Open Trace Log File\n"
										  "trace log close                   Close Trace Log File\n"
										  "timestamp <on|off>                Show Execution Timestamps in IPC Trace Data\n"
										  "\n"
										  "log                               Display Open Log Files\n"
										  "log list                          Display Open Log Files\n"
										  "log <filename>                    Log To File If Debug On It Will Tee Output\n"
										  "log open  [type] <filename>       Open Log File (types: shell(dflt) trace ui)\n"
										  "log write [type] <\"Log Msg\">      Write 'Log Msg' to Log File\n"
										  "log close [type]                  Close Log File\n"
										  "log scan  [pattern]               Display Log Files Match Pattern (dflt=*.log)\n"
										  "nolog                             Stop Logging To File\n"
										  "\n"
										  "KERNEL-MODE TRACE LOGGING:\n"
										  "debugshow                      Show Debug Status\n"
										  "debugset <modules>             Set Kernel Modules To Debug\n"
										  "debuglvl <module> <cat_mask>   Set Kernel Debug Category Mask For Module\n"
										  "debuglvl <tracelevel>          Set Kernel Trace Level (0..4 Default=ERROR(0))\n"
										  "Where <modules> is a hex bit-mask of modules (e.g. 0xfc)\n"
										  "      <module> is a module number\n"
										  "      <cat_mask> is a hex bit-mask of the module debugging categories to enable\n"
										  "\n\n"
										  );
	return output;
}


static char *esif_shell_cmd_ui(EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char **argv   = shell->argv;
	char *output  = shell->outbuf;
	char *subcmd  = NULL;
	u8 gid   = 0;
	u8 mid   = 0;
	u32 gmid = 0;
	u32 appStatusIn = 0;
	char *appname_str = "DPTF";
	EsifData data_status;
	eEsifError rc = ESIF_OK;
	eAppStatusCommand command = (eAppStatusCommand)0;
	EsifAppPtr a_app_ptr = NULL;

	if (argc < 2)
		return NULL;
	subcmd = argv[1];

	// ui getxslt <appname>
	if (esif_ccb_stricmp(subcmd, "getxslt")==0) {
		command = eAppStatusCommandGetXSLT;
		if (argc >= 3)
			appname_str = argv[2];
	}
	// ui getgroups <appname>
	else if (esif_ccb_stricmp(subcmd, "getgroups")==0) {
		command = eAppStatusCommandGetGroups;
		if (argc >= 3)
			appname_str = argv[2];
	}
	// ui getmodulesingroup <appname> <gid>
	else if (esif_ccb_stricmp(subcmd, "getmodulesingroup")==0 && argc >= 4) {
		command = eAppStatusCommandGetModulesInGroup;

		appname_str = argv[2];
		gid  = (u8)esif_atoi(argv[3]);
		appStatusIn = gid;
	}
	// ui getmoduledata <appname> <gid> <mid>
	else if (esif_ccb_stricmp(subcmd, "getmoduledata")==0 && argc >= 5) {
		command = eAppStatusCommandGetModuleData;

		appname_str = argv[2];
		gid  = (u8)esif_atoi(argv[3]);
		mid  = (u8)esif_atoi(argv[4]);

		gmid = gid;
		gmid = gmid << 16;
		gmid = gmid | mid;
		appStatusIn = gmid;
	}
	// unknown subcmd
	else {
		return NULL;
	}

	data_status.type     = ESIF_DATA_STRING;
	data_status.buf_ptr  = output;
	data_status.buf_len  = OUT_BUF_LEN;
	data_status.data_len = 0;

	a_app_ptr = g_appMgr.GetAppFromName(&g_appMgr, appname_str);
	if (NULL == a_app_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find %s App\n", appname_str);
		goto exit;
	}

	rc = a_app_ptr->fInterface.fAppGetStatusFuncPtr(a_app_ptr->fHandle, command, appStatusIn, &data_status);
	if (ESIF_E_NEED_LARGER_BUFFER == rc) {
		output = shell->outbuf = esif_shell_resize(data_status.data_len);
		data_status.buf_ptr = output;
		data_status.buf_len = data_status.data_len;
		rc = a_app_ptr->fInterface.fAppGetStatusFuncPtr(a_app_ptr->fHandle, command, appStatusIn, &data_status);
	}
	if (ESIF_OK != rc) {
		goto exit;
	}
exit:
	return output;
}

static char *esif_shell_cmd_web(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	// Ignore commands from REST API
	if (g_isRest) {
		return NULL;
	}

	// web [status]
	if (argc < 2 || esif_ccb_stricmp(argv[1], "status")==0) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "web server %s\n", (EsifWebIsStarted() ? "started" : "stopped"));
	}
	// web start [restricted] [ip.addr] [port]
	else if (esif_ccb_stricmp(argv[1], "start")==0) {
		if (!EsifWebIsStarted()) {
			int arg=2;
			char *ipaddr = NULL;
			u32 port = 0;
			Bool restricted = ESIF_FALSE;
			int tries = 0;

			if (argc > arg && esif_ccb_stricmp(argv[arg], "restricted") == 0) {
				restricted = ESIF_TRUE;
				arg++;
			}
			if (argc > arg && (isalpha(argv[arg][0]) || strchr(argv[arg], '.') != NULL)) {
				ipaddr = argv[arg++];
			}
			if (argc > arg && isdigit(argv[arg][0])) {
				port = esif_atoi(argv[arg++]);
			}
			EsifWebSetIpaddrPort(ipaddr, port, restricted);
			EsifWebStart();
			
			// thread synchronization delay for output
			for (tries = 0; (tries < 10 && !EsifWebIsStarted()); tries++) {
				esif_ccb_sleep_msec(10);
			}

			if (!EsifWebIsStarted())
				CMD_OUT("web server start failed\n");
			else {
				CMD_OUT("web server started%s\n", (restricted ? " (restricted mode)" : ""));
			}
		}
		else {
			CMD_OUT("web server already started\n");
		}
	}
	// web stop
	else if (esif_ccb_stricmp(argv[1], "stop")==0) {
		if (EsifWebIsStarted()) {
			EsifWebStop();
		}
		else {
			CMD_OUT("web server not started\n");
		}
	}
	return output;
}


static char *esif_shell_cmd_config(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *subcmd = NULL;
	char *namesp = g_DataVaultDefault;
	int  opt = 1;
	eEsifError   rc = ESIF_OK;
	EsifDataPtr  data_nspace= NULL;
	EsifDataPtr  data_key	= NULL;
	EsifDataPtr  data_value = NULL;
	EsifDataPtr  data_targetdv = NULL;
	EsifDataType data_type = ESIF_DATA_AUTO;

	if (argc > 1) {
		subcmd = argv[opt++];
	}

	// Get Optional DataVault parameter
	// config <subcmd> [@datavault] ...
	if (argc > 2 && argv[2][0]=='@') {
		namesp = &argv[opt++][1];
	}
	
	// config [list]
	if (argc < 2 || esif_ccb_stricmp(subcmd, "list")==0) {
		int j=0;
		CMD_OUT("  Name             Keys\n  ---------------- -----\n");
		for (j = 0; j < ESIF_MAX_NAME_SPACES; j++) {
			if (g_DataBankMgr->elements[j].name[0]) {
				esif_string isdefault = (esif_ccb_stricmp(g_DataBankMgr->elements[j].name, g_DataVaultDefault)==0 ? "*" : "");
				CMD_OUT("%-2s%-16s %d\n", isdefault, g_DataBankMgr->elements[j].name, DataCache_GetCount (g_DataBankMgr->elements[j].cache));
			}
		}
	}
	// config get [@datavault] <keyspec> [type]
	else if (argc > opt && esif_ccb_stricmp(subcmd, "get")==0) {
		char *keyspec = argv[opt++];
		char *type = (argc > opt ? argv[opt++] : NULL);

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
		data_value  = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		if (data_nspace==NULL || data_key==NULL || data_value==NULL) 
			goto exit;

		// Enumerate matching keys if keyspec contains "*" or "?" [unless STRING output type specified]
		if (esif_ccb_strpbrk(keyspec, "*?") != NULL && esif_ccb_stricmp((type ? type : ""), "STRING") != 0) {
			EsifConfigFindContext context = NULL;
			int item = 0;

			if ((rc = EsifConfigFindFirst(data_nspace, data_key, NULL, &context)) == ESIF_OK) {
				do {
					esif_flags_t flags = 0;
					if ((rc = EsifConfigGetItem(data_nspace, data_key, data_value, &flags)) == ESIF_OK) {
						char *valuestr = EsifData_ToString(data_value);
						item++;
						CMD_DEBUG("\n"
							"item    = %d\n"
							"type    = %s\n"
							"length  = %d\n"
							"key     = \"%s\"\n"
							"value   = %s%s%s\n",
							item,
							esif_data_type_str(data_value->type),
							data_value->data_len,
							(char*)data_key->buf_ptr,
							(data_value->type == ESIF_DATA_STRING ? "\"" : ""), (valuestr ? valuestr : ""), (data_value->type == ESIF_DATA_STRING ? "\"" : "")
							);

#ifdef ESIF_ATTR_DEBUG
						CMD_DEBUG(
							"flags   = 0x%08X\n"
							"buf_ptr = %p %s\n"
							"buf_len = %d\n"
							"data_len= %d\n",
							flags,
							data_value->buf_ptr,
							((flags & ESIF_SERVICE_CONFIG_FILELINK) ? (esif_string)data_value->buf_ptr : ""),
							data_value->buf_len,
							data_value->data_len
							);
#endif
						esif_ccb_free(valuestr);
					}
					EsifData_Set(data_key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
					EsifData_Set(data_value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
				} while ((rc = EsifConfigFindNext(data_nspace, data_key, NULL, &context)) == ESIF_OK);

				EsifConfigFindClose(&context);
				if (rc == ESIF_E_ITERATION_DONE)
					rc = ESIF_OK;
			}
		}
		// Otherwise get the key/value pair
		else {
			if ((rc = EsifConfigGet(data_nspace, data_key, data_value)) == ESIF_OK) {
				// Convert ESIF Data Type to viewable String
				char *keyvalue = EsifData_ToString(data_value);
				if (keyvalue) {
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", keyvalue);
					esif_ccb_free(keyvalue);
				}
			}
		}

		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config set [@datavault] <key> <value> [type] [option ...]
	else if (argc > opt+1 && esif_ccb_stricmp(subcmd, "set")==0) {
		char *keyname = argv[opt++];
		char *keyvalue= argv[opt++];
		esif_flags_t options = ESIF_SERVICE_CONFIG_PERSIST; // Implied unless NOPERSIST

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyname, 0, ESIFAUTOLEN);
		data_value  = EsifData_Create();
		if (data_nspace==NULL || data_key==NULL || data_value==NULL) 
			goto exit;

		// [type]
		if (argc > opt) {
			EsifDataType newtype = esif_data_type_str2enum(argv[opt]);
			if (newtype != ESIF_DATA_VOID || esif_ccb_stricmp(argv[opt], "VOID") == 0) {
				opt++;
				data_type = newtype;
			}
		}

		// [option ...]
		while (argc > opt) {
			options = EsifConfigFlags_Set(options, argv[opt++]);
		}

		// If this is a link, use the supplied string, otherwise decode the value
		if (options & ESIF_SERVICE_CONFIG_FILELINK) {
			EsifData_Set(data_value, data_type, esif_ccb_strdup(keyvalue), ESIFAUTOLEN, ESIFAUTOLEN);
			if (NULL == data_value->buf_ptr) {
				rc = ESIF_E_NO_MEMORY;
			}
		}
		else {
			char *sep = " && ";
			char *replaced = NULL;

			// Replace " && " with newlines unless type is specified
			if ((data_type == ESIF_DATA_AUTO) && (esif_ccb_strstr(keyvalue, sep) != NULL)) {
				if ((replaced = esif_str_replace(keyvalue, sep, "\n")) != NULL) {
					keyvalue = replaced;
				}
			}

			// Convert Input String to ESIF Data Type
			rc = EsifData_FromString(data_value, keyvalue, data_type);
			esif_ccb_free(replaced);
		}

		// Set Configuration Value
		if (rc == ESIF_OK) {
			rc = EsifConfigSet(data_nspace, data_key, options, data_value);
		}

		// Results
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	// config keys [@datavault] <keyspec>
	else if (argc > opt && esif_ccb_stricmp(subcmd, "keys") == 0) {
		char *keyspec = argv[opt++];
		EsifConfigFindContext context = NULL;

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
		if (data_nspace == NULL || data_key == NULL)
			goto exit;

		// Enumerate Matching Keys 
		if ((rc = EsifConfigFindFirst(data_nspace, data_key, NULL, &context)) == ESIF_OK) {
			do {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s\n", (esif_string)data_key->buf_ptr);
				EsifData_Set(data_key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
			} while ((rc = EsifConfigFindNext(data_nspace, data_key, NULL, &context)) == ESIF_OK);

			EsifConfigFindClose(&context);
			if (rc == ESIF_E_ITERATION_DONE)
				rc = ESIF_OK;
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config copy  [@datavault] <keyspec> [...] @targetdv [option ...]
	// config merge [@datavault] <keyspec> [...] @targetdv [option ...]
	else if (argc > opt + 1 && (esif_ccb_stricmp(subcmd, "copy") == 0) || (esif_ccb_stricmp(subcmd, "merge") == 0)){
		char *keyspecs = esif_ccb_strdup(argv[opt++]);
		char *targetdv = NULL;
		esif_flags_t options = ESIF_SERVICE_CONFIG_PERSIST;
		Bool replaceKeys = (esif_ccb_stricmp(subcmd, "copy") == 0);
		UInt32 keycount = 0;

		// <keyspec> [...]
		while ((keyspecs != NULL) && (argc > opt) && (*argv[opt] != '@')) {
			size_t keylen = esif_ccb_strlen(keyspecs, OUT_BUF_LEN) + esif_ccb_strlen(argv[opt], OUT_BUF_LEN) + 2;
			char *old_keyspecs = keyspecs;
			keyspecs = (char *)esif_ccb_realloc(keyspecs, keylen);
			if (keyspecs == NULL) {
				esif_ccb_free(old_keyspecs);
				break;
			}
			esif_ccb_sprintf_concat(keylen, keyspecs, "\t%s", argv[opt++]);
		}

		if (keyspecs == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else if ((argc > opt) && (*argv[opt] == '@')) {
			targetdv = &argv[opt++][1];
		}

		if (namesp && keyspecs && targetdv) {
			data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
			data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyspecs, 0, ESIFAUTOLEN);
			data_targetdv = EsifData_CreateAs(ESIF_DATA_STRING, targetdv, 0, ESIFAUTOLEN);
		}
		if (data_nspace == NULL || data_key == NULL || data_targetdv == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}

		// [option ...]
		while (rc == ESIF_OK && argc > opt) {
			options = EsifConfigFlags_Set(options, argv[opt++]);
		}

		// Copy or Merge Data
		if (rc == ESIF_OK) {
			rc = EsifConfigCopy(data_nspace, data_targetdv, data_key, options, replaceKeys, &keycount);
		}

		// Results
		if (rc == ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%d Keys %s from %s.dv to %s.dv\n", keycount, (replaceKeys ? "Copied" : "Merged"), namesp, targetdv);
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
		esif_ccb_free(keyspecs);
	}
	// config delete [@datavault] <keyspec>
	else if (argc > opt && esif_ccb_stricmp(subcmd, "delete")==0) {
		char *keyspec = argv[opt++];
		esif_flags_t options = ESIF_SERVICE_CONFIG_DELETE;
		DataVaultPtr DB = DataBank_GetNameSpace(g_DataBankMgr, namesp);
		UInt32 keycount = 0;

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
		data_value  = EsifData_Create();

		// Delete Configuration Value or all values if key = "*"
		if (data_nspace != NULL && data_key != NULL && data_value != NULL) {
			if (DB != NULL) {
				keycount = DataCache_GetCount(DB->cache);
			}
			rc = EsifConfigSet(data_nspace, data_key, options, data_value);
		}

		// Results
		if (rc == ESIF_OK) {
			if (DB != NULL) {
				keycount = keycount - DataCache_GetCount(DB->cache);
			}
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%d Keys Deleted from %s.dv\n", keycount, namesp);
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config exec [@datavault] <key>
	else if (argc > opt && esif_ccb_stricmp(subcmd, "exec")==0) {
		char *keyname = argv[opt++];

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyname, 0, ESIFAUTOLEN);
		data_value  = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

		if (data_nspace != NULL && data_key != NULL && data_value != NULL && (esif_ccb_strpbrk(keyname, "*?") == 0) &&
			(rc = EsifConfigGet(data_nspace, data_key, data_value)) == ESIF_OK && data_value->type == ESIF_DATA_STRING) {
			parse_cmd((char *)data_value->buf_ptr, ESIF_FALSE);
			output = NULL;
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}

	// config export [@datavault] <keyspec> [...] [@targetdv] [bios|dv|both]
	else if (argc > opt && esif_ccb_stricmp(subcmd, "export") == 0) {
		char *outformat = "bios";
		char *keyspecs = esif_ccb_strdup(argv[opt++]);
		char *targetdv = "BiosDataVault";
		char *targetext = ".ASL";
		char dv_file[MAX_PATH] = { 0 };
		char export_file[MAX_PATH] = { 0 };
		FILE *fpin = NULL;
		FILE *fpout = NULL;
		eEsifError rc = ESIF_OK;
		UInt32 keycount = 0;

		// Step 1: Export <keyspec> [...] to temporary targetdv.dv (Default = "BiosDataVault.dv")
		while ((keyspecs != NULL) && (argc > opt) && (*argv[opt] != '@')) {
			size_t keylen = esif_ccb_strlen(keyspecs, OUT_BUF_LEN) + esif_ccb_strlen(argv[opt], OUT_BUF_LEN) + 2;
			char *new_keyspecs = (char *)esif_ccb_realloc(keyspecs, keylen);
			if (new_keyspecs == NULL) {
				esif_ccb_free(keyspecs);
				keyspecs = NULL;
				break;
			}
			keyspecs = new_keyspecs;
			esif_ccb_sprintf_concat(keylen, keyspecs, "\t%s", argv[opt++]);
		}
		if (keyspecs == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else if ((argc > opt) && (*argv[opt] == '@')) {
			targetdv = &argv[opt++][1];
			if (argc > opt) {
				outformat = argv[opt++];
			}
		}

		// Create Parameters
		if (namesp && keyspecs && targetdv) {
			data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
			data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyspecs, 0, ESIFAUTOLEN);
			data_targetdv = EsifData_CreateAs(ESIF_DATA_STRING, targetdv, 0, ESIFAUTOLEN);
		}
		if (data_nspace == NULL || data_key == NULL || data_targetdv == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}

		// Copy Data to targetdv.dv and Close targetdv.dv, unless targetdv.dv already exists
		if (rc == ESIF_OK) {
			DataVaultPtr DB = DataBank_GetNameSpace(g_DataBankMgr, targetdv);
			if (DB != NULL) {
				rc = ESIF_E_IO_OPEN_FAILED;
			}
			else {
				rc = EsifConfigCopy(data_nspace, data_targetdv, data_key, ESIF_SERVICE_CONFIG_PERSIST, ESIF_TRUE, &keycount);
				if ((DB = DataBank_GetNameSpace(g_DataBankMgr, targetdv)) != NULL) {
					if (esif_ccb_stricmp(outformat, "bios") == 0) {
						DataBank_CloseNameSpace(g_DataBankMgr, targetdv);
					}
				}
			}
		}
		esif_ccb_free(keyspecs);

		// Step 2: Convert targetdv.dv to targetdv.asl and delete targetdv.dv (default="BiosDataVault.ASL")
		if (rc == ESIF_OK && keycount > 0 && esif_ccb_stricmp(outformat, "dv") != 0) {
			char dvfilename[MAX_PATH] = { 0 };
			esif_ccb_strcpy(dvfilename, targetdv, sizeof(dvfilename));
			esif_ccb_strlwr(dvfilename, sizeof(dvfilename)); // .dv files are always lowercase
			esif_build_path(dv_file, sizeof(dv_file), ESIF_PATHTYPE_DV, dvfilename, ESIFDV_FILEEXT);
			esif_build_path(export_file, sizeof(export_file), ESIF_PATHTYPE_DV, targetdv, targetext);

			if (((fpin = esif_ccb_fopen(dv_file, "rb", NULL)) != NULL) && ((fpout = esif_ccb_fopen(export_file, "wb", NULL)) != NULL)) {
				u8 buffer[1024] = { 0 };
				size_t bytes = 0;
				size_t total_bytes = 0;
				while ((bytes = esif_ccb_fread(buffer, sizeof(buffer), sizeof(char), sizeof(buffer), fpin)) > 0) {
					size_t j;
					for (j = 0; j < bytes; j++) {
						fprintf(fpout, "%s%s0x%02X",
							(total_bytes > 0 ? "," : ""),
							(total_bytes > 0 && total_bytes % 16 == 0 ? "\r\n" : ""),
							(unsigned int)buffer[j]);
						total_bytes++;
					}
				}
			}
			else {
				rc = ESIF_E_IO_ERROR;
			}
			if (fpin) {
				esif_ccb_fclose(fpin);
			}
			if (fpout) {
				esif_ccb_fclose(fpout);
			}
			
			// Delete temporary targetdv.dv if BIOS output format [default]
			if (esif_ccb_stricmp(outformat, "bios") == 0) {
				esif_ccb_unlink(dv_file);
			}
		}

		if (rc == ESIF_OK) {
			if (esif_ccb_stricmp(outformat, "dv") == 0) {
				esif_ccb_sprintf(sizeof(export_file), export_file, "@%s", targetdv);
			}
			if (keycount == 0) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "No Keys Found to Export\n");
			}
			else {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%d keys exported to %s\n", keycount, export_file);
			}
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config scan [pattern]
	else if (esif_ccb_stricmp(subcmd, "scan")==0) {
		esif_ccb_file_enum_t find_handle = INVALID_HANDLE_VALUE;
		struct esif_ccb_file ffd = {0};
		char file_path[MAX_PATH] = {0};
		char *log_pattern = "*.dv";

		if (argc > 2) {
			log_pattern = argv[2];
		}
		esif_build_path(file_path, sizeof(file_path), ESIF_PATHTYPE_DV, NULL, NULL);

		if ((find_handle = esif_ccb_file_enum_first(file_path, log_pattern, &ffd)) != INVALID_HANDLE_VALUE) {
			do {
				if (esif_ccb_strcmp(ffd.filename, ".")==0 || esif_ccb_strcmp(ffd.filename, "..")==0 || esif_ccb_strchr(ffd.filename, '.')==NULL) {
					continue; // Ignore . and .. and files w/o extensions
				}
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s\n", ffd.filename);
			} while (esif_ccb_file_enum_next(find_handle, log_pattern, &ffd));
			esif_ccb_file_enum_close(find_handle);
		}
	}
	// config open @datavault
	else if (esif_ccb_stricmp(subcmd, "open")==0) {
		if (DataBank_GetNameSpace(g_DataBankMgr, namesp) != NULL) {
			rc = ESIF_E_READONLY;
		}
		else {
			DataVaultPtr DB = DataBank_OpenNameSpace(g_DataBankMgr, namesp);
			if (DB == NULL) {
				rc = ESIF_E_NOT_FOUND;
			}
			else {
				char datavault[MAX_PATH]={0};
				esif_build_path(datavault, sizeof(datavault), ESIF_PATHTYPE_DV, DB->name, ESIFDV_FILEEXT);
				IOStream_SetFile(DB->stream, datavault, "rb");
				if ((rc = DataVault_ReadVault(DB)) != ESIF_OK) {
					DataBank_CloseNameSpace(g_DataBankMgr, namesp);
				}
			}
		}

		// Results
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config close @datavault
	else if (esif_ccb_stricmp(subcmd, "close")==0) {
		DataVaultPtr DB = DataBank_GetNameSpace(g_DataBankMgr, namesp);
		if (DB == NULL) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else if (DB->flags & ESIF_SERVICE_CONFIG_STATIC) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else {
			DataBank_CloseNameSpace(g_DataBankMgr, namesp);
		}

		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config drop @datavault
	else if (esif_ccb_stricmp(subcmd, "drop")==0) {
		if (DataBank_GetNameSpace(g_DataBankMgr, namesp) != NULL) {
			rc = ESIF_E_NOT_SUPPORTED;
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
		else {
			int ignore = 0;
			char datavault[MAX_PATH]={0};
			esif_build_path(datavault, MAX_PATH, ESIF_PATHTYPE_DV, namesp, ESIFDV_FILEEXT);
			ignore = esif_ccb_unlink(datavault);
		}
	}
	// config select @datavault
	else if (esif_ccb_stricmp(subcmd, "select")==0) {
		if (DataBank_GetNameSpace(g_DataBankMgr, namesp) == NULL) {
			rc = ESIF_E_NOT_FOUND;
		}
		else {
			esif_ccb_strcpy(g_DataVaultDefault, namesp, sizeof(g_DataVaultDefault));
		}

		// Results
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}

exit:
	// Cleanup
	EsifData_Destroy(data_nspace);
	EsifData_Destroy(data_key);
	EsifData_Destroy(data_value);
	EsifData_Destroy(data_targetdv);
	return output;
}


// Kernel Participant Extensions: driversk, driverk
static char *esif_shell_cmd_driversk(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc_command *command_ptr = NULL;
	u32 data_len = sizeof(struct esif_command_get_drivers);
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	struct esif_command_get_drivers *data_ptr = NULL;
	struct esif_driver_info *drv_ptr = NULL;
	u32 count = 0;
	u32 i = 0;
	u8 get_driver = ESIF_FALSE;
	u32 driver_id = 0;
	char guid_str[ESIF_GUID_PRINT_SIZE] = { 0 };

	if (argc > 1) {
		get_driver = ESIF_TRUE;
		driver_id = esif_atoi(argv[1]);
	}

	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"esif_ipc_alloc_command failed for %u bytes\n",
			data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_DRIVERS;
	command_ptr->req_data_type   = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 0;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	if (get_driver) {
		command_ptr->req_data_type = ESIF_DATA_UINT32;
		command_ptr->req_data_len = (u32) sizeof(u32);
		*(u32 *)(command_ptr + 1) = driver_id;
	}

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"IPC error code = %s(%d)\n",
			esif_rc_str(ipc_ptr->return_code),
			ipc_ptr->return_code);
		goto exit;
	}

	/*
	 * If the buffer is too small, reallocate based on the number of drivers that are present and retry
	 */
	if (ESIF_E_NEED_LARGER_BUFFER == command_ptr->return_code) {
		data_ptr = (struct esif_command_get_drivers *)(command_ptr + 1);
		count = data_ptr->available_count;
		if (0 == count) {
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"Invalid driver count (0) returned for ESIF_E_NEED_LARGER_BUFFER\n");
			goto exit;	
		}
		data_len = sizeof(struct esif_command_get_drivers) + ((count - 1) * sizeof(*data_ptr->driver_info));

		esif_ipc_free(ipc_ptr);

		ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

		if (NULL == ipc_ptr || NULL == command_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN,
				output,
				"esif_ipc_alloc_command failed for %u bytes\n",
				data_len);
			goto exit;
		}

		command_ptr->type = ESIF_COMMAND_TYPE_GET_DRIVERS;
		command_ptr->req_data_type   = ESIF_DATA_VOID;
		command_ptr->req_data_offset = 0;
		command_ptr->req_data_len    = 0;
		command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
		command_ptr->rsp_data_offset = 0;
		command_ptr->rsp_data_len    = data_len;

		if (get_driver) {
			command_ptr->req_data_type = ESIF_DATA_UINT32;
			command_ptr->req_data_len = sizeof(u32);
			*(u32 *)(command_ptr + 1) = driver_id;
		}

		ipc_execute(ipc_ptr);
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"ipc error code = %s(%d)\n",
			esif_rc_str(ipc_ptr->return_code),
			ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"Command error code = %s(%d)\n",
			 esif_rc_str(command_ptr->return_code),
			 command_ptr->return_code);
		goto exit;
	}

	// out data
	data_ptr = (struct esif_command_get_drivers *)(command_ptr + 1);
	count    = data_ptr->returned_count;

	// driverk <id>
	if (get_driver) {
		if (count < 1) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "driver not available\n");
		}
		else if (FORMAT_TEXT == g_format) {
			drv_ptr = &data_ptr->driver_info[0];
			esif_ccb_sprintf(
				OUT_BUF_LEN,
				output,
				"\n"
				"Instance:    %d\n"
				"Action Type: %s (%d)\n"
				"Version:     %d\n"
				"GUID:        %s\n"
				"Flags:       0x%08X\n"
				"Name:        %s\n"
				"Description: %s\n"
				"Driver Name: %s\n"
				"Device Name: %s\n"
				"\n"
				,
				driver_id,
				esif_action_type_str(drv_ptr->action_type), drv_ptr->action_type,
				(u32)drv_ptr->version,
				esif_guid_print(&drv_ptr->class_guid, guid_str),
				drv_ptr->flags,
				drv_ptr->name,
				drv_ptr->desc,
				drv_ptr->driver_name,
				drv_ptr->device_name
				);
		}
		else {
			drv_ptr = &data_ptr->driver_info[0];
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"<driver>\n"
				"  <instance>%d</instance>"
				"  <action_type>%d</action_type>\n"
				"  <action_type_str>%s</action_type_str>\n"
				"  <version>%d</version>\n"
				"  <guid>%s</guid>\n"
				"  <flags>0x%08X</flags>\n"
				"  <name>%s</name>\n"
				"  <desc>%s</desc>\n"
				"  <driver_name>%s</driver_name>\n"
				"  <device_name>%s</device_name>\n"
				"</driver>\n"
				,
				driver_id,
				drv_ptr->action_type,
				esif_action_type_str(drv_ptr->action_type),
				(u32)drv_ptr->version,
				esif_guid_print(&drv_ptr->class_guid, guid_str),
				drv_ptr->flags,
				drv_ptr->name,
				drv_ptr->desc,
				drv_ptr->driver_name,
				drv_ptr->device_name
				);
		}
	}
	// driversk
	else {
		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(
				OUT_BUF_LEN,
				output,
				"\nKERNEL PARTICIPANT EXTENSION (KPE) DRIVERS:\n\n"
				"ActionType Name                 Description          Device           Ver  Flags\n"
				"---------- -------------------- -------------------- ---------------- ---- ----------\n");
		}
		else {// XML
			esif_ccb_sprintf(OUT_BUF_LEN, output, "<driversk>\n");
		}

		for (i = 0; i < count; i++) {
			drv_ptr = &data_ptr->driver_info[i];

			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN,
					output,
					"%-10.10s %-20s %-20s %-16s %-4d 0x%08X\n",
					ltrim(esif_action_type_str(drv_ptr->action_type), PREFIX_ACTION_TYPE),
					drv_ptr->name,
					drv_ptr->desc,
					drv_ptr->device_name,
					drv_ptr->version,
					drv_ptr->flags
					);
			}
			else {// FORMAT_XML
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					"<driver>\n"
					"  <action_type>%d</action_type>\n"
					"  <action_type_str>%s</action_type_str>\n"
					"  <name>%s</name>\n"
					"  <desc>%s</desc>\n"
					"  <device_name>%s</device_name>\n"
					"  <version>%d</version>\n"
					"  <flags>0x%08X</flags>\n"
					"  <guid>%s</guid>\n"
					"</driver>\n",
					drv_ptr->action_type,
					esif_action_type_str(drv_ptr->action_type),
					drv_ptr->name,
					drv_ptr->desc,
					drv_ptr->device_name,
					drv_ptr->version,
					drv_ptr->flags,
					esif_guid_print(&drv_ptr->class_guid, guid_str)
					);
			}
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		}
		else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</driversk>\n");
		}
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}

static char *esif_shell_cmd_shell(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	// shell
	if (argc < 2) {
		CMD_OUT("shell %s\n", (g_shell_enabled ? "enabled" : "disabled"));
	}
	// shell enable
	else if (esif_ccb_stricmp(argv[1], "enable")==0) {
		g_shell_enabled = 1;
		CMD_OUT("shell enabled\n");
	}
	// shell disable
	else if (esif_ccb_stricmp(argv[1], "disable")==0) {
		g_shell_enabled = 0;
		CMD_OUT("shell disabled\n");
	}
	return output;
}

static char *esif_shell_cmd_sleep(EsifShellCmdPtr shell)
{
	   int argc     = shell->argc;
	   char **argv  = shell->argv;
	   char *output = shell->outbuf;

	   // sleep <msec>
	   if (argc > 1) {
			  UInt32 msec = esif_atoi(argv[1]);
			  esif_ccb_sleep_msec(msec);
	   }
	   return output;
}



// Run External Shell Command
static void esif_shell_exec_cmdshell(char *line)
{
	esif_ccb_system(line);
}


// Helper for esif_shell_cmd_idsp
typedef struct IdspEntry_s {
	union esif_data_variant variant;
	char uuid[ESIF_GUID_LEN];
} IdspEntry, *IdspEntryPtr;


// Helper for esif_shell_cmd_idsp
static char *concat_uuid_list(
	char *outputPtr,
	IdspEntryPtr entryPtr,
	int numEntries
	)
{
	char uuidStr[ESIF_GUID_PRINT_SIZE] = {0};
	int i = 0;

	ESIF_ASSERT(outputPtr != NULL);
	ESIF_ASSERT(entryPtr != NULL);

	for (i = 0; i < numEntries; i++) {
		esif_guid_print((esif_guid_t *)(entryPtr->uuid), uuidStr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, outputPtr, "%s\n", uuidStr);

		entryPtr++;
	}

	return outputPtr;
}


// Helper for esif_shell_cmd_idsp
static IdspEntryPtr find_uuid_in_idsp_entries(
	char *uuidPtr,
	IdspEntryPtr entryPtr,
	int numEntries
	)
{
	int i = 0;

	ESIF_ASSERT(uuidPtr != NULL);
	ESIF_ASSERT(entryPtr != NULL);

	for (i = 0; i < numEntries; i++) {
		if (memcmp(entryPtr->uuid, uuidPtr, ESIF_GUID_LEN) == 0) {
			goto exit;
		}
		entryPtr++;
	}
	entryPtr = NULL;
exit:
	return entryPtr;
}


static char *esif_shell_cmd_idsp(EsifShellCmdPtr shell)
{
	eEsifError rc = ESIF_OK;
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *subcmd = NULL;
	int  opt = 1;
	char* uuidStr = NULL;
	char uuidBuffer[ESIF_GUID_LEN] = {0};
	UInt16 guidShorts[32] = {0};
	char notAllowed = '\0';
	int uuidFieldsRead = 0;
	EsifDataPtr reqPtr = NULL;
	EsifDataPtr rspPtr = NULL;
	int numEntries = 0;
	IdspEntryPtr matchingEntryPtr = NULL;
	IdspEntryPtr endPtr = NULL;
	IdspEntryPtr curEntryPtr = NULL;
	IdspEntryPtr nextEntryPtr = NULL;
	IdspEntryPtr newIdspPtr = NULL;
	int newIdspSize = 0;
	
	// Get the current IDSP
	rspPtr = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	if (NULL == rspPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to allocate memory\n");
		goto exit;
	}
	rc = EsifExecutePrimitive(
		ESIF_INSTANCE_LF,
		GET_SUPPORTED_POLICIES,
		"D0",
		255,
		NULL,
		rspPtr);
	if (rc != ESIF_OK) {
		rspPtr->data_len = 0; /* Defensive, should already be 0 */
	}

	numEntries = (int)(rspPtr->data_len / sizeof(*curEntryPtr));

	// Parse command arguments

	// idsp
	if (1 == argc) {
		if (numEntries != 0) {
			concat_uuid_list(output, (IdspEntryPtr)rspPtr->buf_ptr, numEntries);
		} else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to retrieve or no IDSP present\n");
		}
		goto exit;
	}

	// idsp subcmd (Missing UUID)
	if (2 == argc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: UUID must be specified\n");
		goto exit;
	}

	// idsp subcmd UUID
	subcmd = argv[opt++];
	uuidStr = argv[opt++];

	// Validate the UUID is correctly formatted
	uuidFieldsRead = esif_ccb_sscanf(uuidStr, "%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx-%1hx%1hx%1hx%1hx-%1hx%1hx%1hx%1hx-%1hx%1hx%1hx%1hx-%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%c", 
		&guidShorts[0], &guidShorts[1], &guidShorts[2], &guidShorts[3],
		&guidShorts[4], &guidShorts[5], &guidShorts[6], &guidShorts[7],
		&guidShorts[8], &guidShorts[9], &guidShorts[10], &guidShorts[11],
		&guidShorts[12], &guidShorts[13], &guidShorts[14], &guidShorts[15],
		&guidShorts[16], &guidShorts[17], &guidShorts[18], &guidShorts[19],
		&guidShorts[20], &guidShorts[21], &guidShorts[22], &guidShorts[23],
		&guidShorts[24], &guidShorts[25], &guidShorts[26], &guidShorts[27],
		&guidShorts[28], &guidShorts[29], &guidShorts[30], &guidShorts[31],
		SCANFBUF(&notAllowed, 1));

	if (uuidFieldsRead != (ESIF_GUID_LEN * 2)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: UUID must be in the format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\n");
		goto exit;		
	}

	uuidFieldsRead = esif_ccb_sscanf(uuidStr, "%2hx%2hx%2hx%2hx-%2hx%2hx-%2hx%2hx-%2hx%2hx-%2hx%2hx%2hx%2hx%2hx%2hx", 
		&guidShorts[0], &guidShorts[1], &guidShorts[2], &guidShorts[3],
		&guidShorts[4], &guidShorts[5], &guidShorts[6], &guidShorts[7],
		&guidShorts[8], &guidShorts[9], &guidShorts[10], &guidShorts[11],
		&guidShorts[12], &guidShorts[13], &guidShorts[14], &guidShorts[15]);
	esif_copy_shorts_to_bytes((UInt8 *)uuidBuffer, guidShorts, ESIF_GUID_LEN);

	matchingEntryPtr = find_uuid_in_idsp_entries(uuidBuffer, (IdspEntryPtr)rspPtr->buf_ptr, numEntries);

	// idsp add UUID
	if (esif_ccb_stricmp(subcmd, "add") == 0) {
		if (matchingEntryPtr != NULL) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Specified UUID already present\n");
			goto exit;
		}

		newIdspSize = rspPtr->data_len + sizeof(*newIdspPtr);
		newIdspPtr = (IdspEntryPtr)esif_ccb_malloc(newIdspSize);
		if (newIdspPtr == NULL) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to write new IDSP\n");
			goto exit;
		}

		// Copy the current UUIDs to the new IDSP buffer
		esif_ccb_memcpy((char *)newIdspPtr, rspPtr->buf_ptr, rspPtr->data_len);

		// Set up the new entry at the end of the new IDSP buffer
		curEntryPtr = newIdspPtr + numEntries;
		curEntryPtr->variant.type = ESIF_DATA_BINARY;
		curEntryPtr->variant.string.length = ESIF_GUID_LEN;
		curEntryPtr->variant.string.reserved = 0;
		esif_ccb_memcpy(curEntryPtr->uuid, uuidBuffer, ESIF_GUID_LEN);

		// Write the new IDSP to the datavault
		reqPtr = EsifData_CreateAs(ESIF_DATA_BINARY, newIdspPtr, newIdspSize, newIdspSize);
		rc = EsifExecutePrimitive(
			ESIF_INSTANCE_LF,
			SET_SUPPORTED_POLICIES,
			"D0",
			255,
			reqPtr,
			NULL);
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to write new IDSP\n");
			goto exit;
		}
	}

	// idsp delete UUID
	else if (esif_ccb_stricmp(subcmd, "delete") == 0) {
		if (NULL == matchingEntryPtr) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Specified UUID not found\n");
			goto exit;
		}

		// Move UUIDs up to cover the UUID being deleted
		endPtr = ((IdspEntryPtr)rspPtr->buf_ptr) + numEntries;
		curEntryPtr = matchingEntryPtr;
		nextEntryPtr = curEntryPtr + 1;
		while (nextEntryPtr < endPtr) {
			esif_ccb_memcpy(curEntryPtr->uuid, nextEntryPtr->uuid, sizeof(curEntryPtr->uuid));
			curEntryPtr++;
			nextEntryPtr++;
		}

		// Write the new IDSP to the datavault
		reqPtr = EsifData_CreateAs(ESIF_DATA_BINARY, rspPtr->buf_ptr, 0, rspPtr->data_len - sizeof(*curEntryPtr));
		rc = EsifExecutePrimitive(
			ESIF_INSTANCE_LF,
			SET_SUPPORTED_POLICIES,
			"D0",
			255,
			reqPtr,
			NULL);
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to write new IDSP\n");
			goto exit;
		}
	}

	// Unknown subcommand
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unknown option specified\n");
		goto exit;
	}
exit:
	EsifData_Destroy(reqPtr);
	EsifData_Destroy(rspPtr);
	return output;
}







///////////////////////////////////////////////////////////////////////////////
// DEBUG
///////////////////////////////////////////////////////////////////////////////

static void dump_table_hdr(struct esif_table_hdr *tab)
{
	CMD_DEBUG("Binary Dump For Table Header\n\n");
	CMD_DEBUG("binary_data_object[%u] = {\n", (int)sizeof(*tab));

	CMD_DEBUG("    table header = {\n");
	CMD_DEBUG("       revision = %d,\n", tab->revision);
	CMD_DEBUG("       num of rows = %d,\n", tab->rows);
	CMD_DEBUG("       num of cols = %d,\n", tab->cols);
	CMD_DEBUG("}\n");
}


// Binary Data Variant Object
static void dump_binary_object(
	u8 *byte_ptr,
	u32 byte_len
	)
{
	union esif_data_variant *obj = (union esif_data_variant *)byte_ptr;
	int remain_bytes = byte_len, counter = 1024;
	char *str;

	CMD_DEBUG("Binary Dump Up To %d Objects:\n\n", counter);
	CMD_DEBUG("binary_data_object[%u] = {\n", byte_len);

	while (remain_bytes > 0 && counter) {
		remain_bytes -= sizeof(*obj);
		counter--;
		switch (obj->type) {
		case ESIF_DATA_UINT8:
		case ESIF_DATA_UINT16:
		case ESIF_DATA_UINT32:
		case ESIF_DATA_UINT64:
		case ESIF_DATA_INT8:
		case ESIF_DATA_INT16:
		case ESIF_DATA_INT32:
		case ESIF_DATA_INT64:
		case ESIF_DATA_TEMPERATURE:
		case ESIF_DATA_FREQUENCY:
		case ESIF_DATA_PERCENT:
		{
			CMD_DEBUG("    integer = { type = %s(%d) value = %lld (0x%llx) }\n",
					  esif_data_type_str(obj->integer.type),
					  obj->integer.type,
					  obj->integer.value,
					  obj->integer.value);
			obj = (union esif_data_variant *)((u8 *)obj + sizeof(*obj));
			break;
		}

		case ESIF_DATA_STRING:
		{
			str = (char *)((u8 *)obj + sizeof(*obj));

			CMD_DEBUG("    string = {\n");
			CMD_DEBUG("       type = %s(%d),\n", esif_data_type_str(obj->string.type), obj->string.type);
			CMD_DEBUG("       length = %d,\n", obj->string.length);
			CMD_DEBUG("       value = \"%s\"\n", str);
			CMD_DEBUG("    }\n");
			remain_bytes -= obj->string.length;
			obj = (union esif_data_variant *)((u8 *)obj + (sizeof(*obj) + obj->string.length));
			break;
		}

		case ESIF_DATA_UNICODE:
		{
			u32 i = 0;
			str = (char *)((u8 *)obj + sizeof(*obj));
			CMD_DEBUG("    unicode = {\n");
			CMD_DEBUG("       type = %s(%d),\n", esif_data_type_str(obj->string.type), obj->string.type);
			CMD_DEBUG("       length = %d,\n", obj->string.length);
			CMD_DEBUG("       value = ");

			for (i = 0; i < obj->string.length; i++) {
				CMD_DEBUG("%02X ", (u8)str[i]);
			}

			CMD_DEBUG("\n    }\n");
			remain_bytes -= obj->string.length;
			obj = (union esif_data_variant *)((u8 *)obj + (sizeof(*obj) + obj->string.length));
			break;
		}

		case ESIF_DATA_BINARY:
		{
			u32 i = 0;
			str = (char *)((u8 *)obj + sizeof(*obj));
			CMD_DEBUG("    binary = {\n");
			CMD_DEBUG("       type = %s(%d),\n", esif_data_type_str(obj->string.type), obj->string.type);
			CMD_DEBUG("       length = %d,\n", obj->string.length);
			CMD_DEBUG("       value = ");

			for (i = 0; i < obj->string.length; i++) {
				CMD_DEBUG("%02X ", (u8)str[i]);
			}

			CMD_DEBUG("\n    }\n");
			remain_bytes -= obj->string.length;
			obj = (union esif_data_variant *)((u8 *)obj + (sizeof(*obj) + obj->string.length));
			break;
		}

		/* No more binary data, so stop here! */
		default:
			counter = 0;
		}
	}
	CMD_DEBUG("}\n");
}


// Binary Data
static void dump_binary_data(
	u8 *byte_ptr,
	u32 byte_len
	)
{
	u32 i = 0;
	CMD_DEBUG("Buffer Dump Up To 256 Bytes Maximum:\n");
	for (i = 0; ((i < byte_len) && (i < 256)); i++) {
		if (0 == i) {
			CMD_DEBUG("\n%04x: ", i);
		}

		if (i > 0 && 0 == (i % 8)) {
			CMD_DEBUG("\n%04x: ", i);
		}
		CMD_DEBUG("%02x", *((u8 *)byte_ptr + i));

		if ('\r' == *((u8 *)byte_ptr + i)) {
			CMD_DEBUG("(CR) ");
		} else if ('\n' == *((u8 *)byte_ptr + i)) {
			CMD_DEBUG("(LF) ");
		} else {
			CMD_DEBUG("(%2c) ", *((u8 *)byte_ptr + i));
		}
	}
	CMD_DEBUG("\n\n");
}


// Execute Shell Command
EsifString esif_shell_exec_command(
	char *line,
	size_t buf_len,
	UInt8 isRest
	)
{
	char *in_str  = line;
	char *out_str = NULL;
	EsifAppPtr a_app_ptr = g_appMgr.fSelectedAppPtr;
	enum output_format last_format = g_format;
	char *cmd_separator = NULL;
	size_t cmdlen=0;

	if (esif_ccb_strlen(line, buf_len) >= (buf_len - 1)) {
		return NULL;
	}

	esif_uf_shell_lock();

	// Create output buffer if necessary
	if ((g_outbuf == NULL) && ((g_outbuf = esif_ccb_malloc(OUT_BUF_LEN)) == NULL)) {
		goto exit;
	}

	// REST API always uses XML results
	g_isRest = isRest;
	if (ESIF_TRUE == isRest) {
		last_format = g_format;
		g_format = FORMAT_XML;
	}

	cmdlen = esif_ccb_strlen(in_str, buf_len);
	do {
		// Skip first iteration
		if (cmd_separator != NULL) {
			esif_ccb_strcpy(line, cmd_separator, cmdlen);
		}

		// Allow multiple commands separated by newlines
		if (((cmd_separator = strstr(line, "\n")) != NULL) || 
			((cmd_separator = strstr(line, " && ")) != NULL)) {
			char sep = *cmd_separator;
			*cmd_separator = 0;
			cmd_separator += (sep=='\n' ? 1 : 4);
		}

		g_errorlevel = 0;
		if (ESIF_TRUE == isRest) {
			g_format = FORMAT_XML;
		}

		// Run External Command Shell. Disabled in UMDF, shell, & Daemon mode and REST API
		if (g_cmdshell_enabled && !g_isRest && line[0]=='!') {
			esif_shell_exec_cmdshell(line + 1);
			out_str = NULL;
			goto display_output;
		}

		line = esif_ccb_strtok(line, ESIF_SHELL_STRTOK_SEP, &g_line_context);
		if (line == NULL || *line == '#') {
			out_str = NULL;
			goto display_output;
		}
		g_outbuf[0] = 0;

		//
		// Global Commands Always Available. May There Be Few
		//
		if (esif_ccb_stricmp(line, "appselect") == 0) {
			esif_shell_exec_dispatch(line, g_outbuf);
			out_str = g_outbuf;
			goto display_output;
		}

		if (NULL == a_app_ptr) {
			out_str = esif_shell_exec_dispatch(line, g_outbuf);
		} else {
			struct esif_data request;
			struct esif_data response;
			enum esif_rc rc = ESIF_OK;

			request.type      = ESIF_DATA_STRING;
			request.buf_ptr   = line;
			request.buf_len   = (u32)ESIF_SHELL_STRLEN(line);
			request.data_len  = (u32)ESIF_SHELL_STRLEN(line);

			response.type     = ESIF_DATA_STRING;
			response.buf_ptr  = g_outbuf;
			response.buf_len  = OUT_BUF_LEN;
			response.data_len = 0;

			rc = a_app_ptr->fInterface.fAppCommandFuncPtr(a_app_ptr->fHandle, &request, &response, g_line_context);

			if (ESIF_OK == rc) {
				out_str = g_outbuf;
			} else {
				out_str = NULL;
			}
		}

display_output:
		if (ESIF_FALSE == isRest) {
			if (NULL == out_str) {
				CMD_OUT("%s", "");
			}
			if ((NULL != out_str) && (out_str[0] != '\0')) {
				CMD_OUT("%s", out_str);
			}
		}
	} while (line != NULL && cmd_separator != NULL);

	esif_ccb_memset(in_str, 0, cmdlen+1);
	if (ESIF_TRUE == isRest) {
		g_format = last_format;
	}
	g_isRest = 0;

exit:
	esif_uf_shell_unlock();
	return out_str;
}


// Parse Command By Subsystem
EsifString parse_cmd(
	char *line,
	UInt8 isRest
	)
{
	return esif_shell_exec_command(line, MAX_LINE, isRest);
}


//////////////////////////////////////////////////////////////////////////////
// Command Interpreter

// Shell Wrapper
typedef void (*VoidFunc)();
typedef char *(*ArgvFunc)(EsifShellCmd *);

typedef enum FuncType_t {
	fnArgv,		// Use Command Line argc/argv Parser
} FuncType;
typedef struct EsifShellMap_t {
	char *cmd;
	FuncType type;
	VoidFunc func;
} EsifShellMap;

#ifndef ESIF_FEAT_OPT_ACTION_SYSFS

// Shell Command Wrapper
char *esif_shell_cmd_ipc_autoconnect(EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	ipc_autoconnect(30); // 30 second timeout
	return NULL;
}


// Shell Command Wrapper
char *esif_shell_cmd_ipc_connect(EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	ipc_connect();
	return NULL;
}

// Shell Command Wrapper
char *esif_shell_cmd_ipc_disconnect(EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	ipc_disconnect();
	return NULL;
}

#endif

// Shell Command Mapping. Keep this array sorted alphabetically to facilitate Binary Searches
static EsifShellMap ShellCommands[] = {
	{"about",                fnArgv, (VoidFunc)esif_shell_cmd_about               },
	{"actions",              fnArgv, (VoidFunc)esif_shell_cmd_actions             },
	{"actionsk",             fnArgv, (VoidFunc)esif_shell_cmd_actionsk            },
	{"actionstart",          fnArgv, (VoidFunc)esif_shell_cmd_actionstart         },
	{"actionstop",           fnArgv, (VoidFunc)esif_shell_cmd_actionstop          },
	{"actionsu",             fnArgv, (VoidFunc)esif_shell_cmd_actionsu            },
	{"apps",                 fnArgv, (VoidFunc)esif_shell_cmd_apps                },
	{"appselect",            fnArgv, (VoidFunc)esif_shell_cmd_appselect           },// Global Command
	{"appstart",             fnArgv, (VoidFunc)esif_shell_cmd_appstart            },
	{"appstop",              fnArgv, (VoidFunc)esif_shell_cmd_appstop             },
	{"autoexec",             fnArgv, (VoidFunc)esif_shell_cmd_autoexec            },
	{"cat",                  fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"cattst",               fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"config",               fnArgv, (VoidFunc)esif_shell_cmd_config              },
	{"conjure",              fnArgv, (VoidFunc)esif_shell_cmd_conjure             },
	{"conjures",             fnArgv, (VoidFunc)esif_shell_cmd_conjures            },
	{"datalog",			     fnArgv, (VoidFunc)EsifShellCmdDataLog                },
	{"debuglvl",             fnArgv, (VoidFunc)esif_shell_cmd_debuglvl            },
	{"debugset",             fnArgv, (VoidFunc)esif_shell_cmd_debugset            },
	{"debugshow",            fnArgv, (VoidFunc)esif_shell_cmd_debugshow           },
	{"domains",              fnArgv, (VoidFunc)esif_shell_cmd_domains             },
	{"driverk",              fnArgv, (VoidFunc)esif_shell_cmd_driversk            },
	{"driversk",             fnArgv, (VoidFunc)esif_shell_cmd_driversk            },
	{"dspquery",			 fnArgv, (VoidFunc)esif_shell_cmd_dspquery			  },
	{"dsps",                 fnArgv, (VoidFunc)esif_shell_cmd_dsps                },
	{"dst",                  fnArgv, (VoidFunc)esif_shell_cmd_dst                 },
	{"dstn",                 fnArgv, (VoidFunc)esif_shell_cmd_dstn                },
	{"echo",                 fnArgv, (VoidFunc)esif_shell_cmd_echo                },
	{"event",                fnArgv, (VoidFunc)esif_shell_cmd_event               },
	{"eventkpe",             fnArgv, (VoidFunc)esif_shell_cmd_eventkpe            },
	{"exit",                 fnArgv, (VoidFunc)esif_shell_cmd_exit                },
	{"format",               fnArgv, (VoidFunc)esif_shell_cmd_format              },
	{"getb",                 fnArgv, (VoidFunc)esif_shell_cmd_getb                },
	{"geterrorlevel",        fnArgv, (VoidFunc)esif_shell_cmd_geterrorlevel       },
	{"getf_b",               fnArgv, (VoidFunc)esif_shell_cmd_getf                },
	{"getf_bd",              fnArgv, (VoidFunc)esif_shell_cmd_getf                },
	{"getp",                 fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst)"
	{"getp_b",               fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as binary"
	{"getp_bd",              fnArgv, (VoidFunc)esif_shell_cmd_getp                },
	{"getp_bs",              fnArgv, (VoidFunc)esif_shell_cmd_getp                },
	{"getp_bt",              fnArgv, (VoidFunc)esif_shell_cmd_getp                },
	{"getp_pw",              fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as power"
	{"getp_s",               fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as string"
	{"getp_t",               fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as temperature"
	{"getp_u32",             fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as uint32"
	{"help",                 fnArgv, (VoidFunc)esif_shell_cmd_help                },
	{"idsp",                 fnArgv, (VoidFunc)esif_shell_cmd_idsp                },
	{"info",                 fnArgv, (VoidFunc)esif_shell_cmd_info                },
	{"infocpc",              fnArgv, (VoidFunc)esif_shell_cmd_infocpc             },
	{"infofpc",              fnArgv, (VoidFunc)esif_shell_cmd_infofpc             },
#ifndef ESIF_FEAT_OPT_ACTION_SYSFS
	{"ipcauto",              fnArgv, (VoidFunc)esif_shell_cmd_ipc_autoconnect     },
	{"ipccon",               fnArgv, (VoidFunc)esif_shell_cmd_ipc_connect         },
	{"ipcdis",               fnArgv, (VoidFunc)esif_shell_cmd_ipc_disconnect      },
#endif
	{"load",                 fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"loadtst",              fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"log",                  fnArgv, (VoidFunc)esif_shell_cmd_log                 },
	{"memstats",             fnArgv, (VoidFunc)esif_shell_cmd_memstats            },
	{"nolog",                fnArgv, (VoidFunc)esif_shell_cmd_nolog               },
	{"part",                 fnArgv, (VoidFunc)esif_shell_cmd_participant         },
	{"participant",          fnArgv, (VoidFunc)esif_shell_cmd_participant         },	
	{"participantk",         fnArgv, (VoidFunc)esif_shell_cmd_participantk        },
	{"participantlog",       fnArgv, (VoidFunc)EsifShellCmd_ParticipantLog        },
	{"participants",         fnArgv, (VoidFunc)esif_shell_cmd_participants        },
	{"participantsk",        fnArgv, (VoidFunc)esif_shell_cmd_participantsk       },
	{"partk",                fnArgv, (VoidFunc)esif_shell_cmd_participantk        },
	{"parts",                fnArgv, (VoidFunc)esif_shell_cmd_participants        },
	{"partsk",               fnArgv, (VoidFunc)esif_shell_cmd_participantsk       },
	{"paths",				 fnArgv, (VoidFunc) esif_shell_cmd_paths },
	{"proof",                fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"prooftst",             fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"pst",                  fnArgv, (VoidFunc)esif_shell_cmd_pst                 },
	{"quit",                 fnArgv, (VoidFunc)esif_shell_cmd_quit                },
	{"rem",                  fnArgv, (VoidFunc)esif_shell_cmd_rem                 },
	{"repeat",               fnArgv, (VoidFunc)esif_shell_cmd_repeat              },
	{"repeat_delay",         fnArgv, (VoidFunc)esif_shell_cmd_repeatdelay         },
	{"set_dscp",             fnArgv, (VoidFunc)esif_shell_cmd_set_dscp            },
	{"set_osc",              fnArgv, (VoidFunc)esif_shell_cmd_set_osc             },
	{"set_scp",              fnArgv, (VoidFunc)esif_shell_cmd_set_scp             },
	{"setb",                 fnArgv, (VoidFunc)esif_shell_cmd_setb                },
	{"seterrorlevel",        fnArgv, (VoidFunc)esif_shell_cmd_seterrorlevel       },
	{"setp",                 fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"setp_pw",              fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"setp_t",               fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"shell",                fnArgv, (VoidFunc)esif_shell_cmd_shell               },
	{"sleep",                fnArgv, (VoidFunc)esif_shell_cmd_sleep               },
	{"soe",                  fnArgv, (VoidFunc)esif_shell_cmd_soe                 },
	{"status",               fnArgv, (VoidFunc)esif_shell_cmd_status              },
	{"tableobject",          fnArgv, (VoidFunc)esif_shell_cmd_tableobject         },
	{"test",                 fnArgv, (VoidFunc)esif_shell_cmd_test                },	
	{"thermalapi",           fnArgv, (VoidFunc)EsifShellCmdThermalApi             },
	{"timerstart",           fnArgv, (VoidFunc)esif_shell_cmd_timerstart          },
	{"timerstop",            fnArgv, (VoidFunc)esif_shell_cmd_timerstop           },
	{"timestamp",            fnArgv, (VoidFunc)esif_shell_cmd_timestamp           },
	{"trace",                fnArgv, (VoidFunc)esif_shell_cmd_trace               },
	{"ufpoll",				 fnArgv, (VoidFunc)esif_shell_cmd_ufpoll			  },
	{"ui",                   fnArgv, (VoidFunc)esif_shell_cmd_ui                  },// formerly ui_getxslt, ui_getgroups, ui_getmodulesingroup, ui_getmoduledata
	{"unconjure",            fnArgv, (VoidFunc)esif_shell_cmd_unconjure           },
	{"upes",                 fnArgv, (VoidFunc)esif_shell_cmd_upes                },
	{"web",                  fnArgv, (VoidFunc)esif_shell_cmd_web                 },
};

// ESIF Command Dispatcher
char *esif_shell_exec_dispatch(
	esif_string line,
	esif_string output
	)
{
	static int items = sizeof(ShellCommands) / sizeof(EsifShellMap);
	int start   = 0, end = items - 1, node = items / 2;
	char *cmd   = 0;
	char *rcStr = output;
	EsifShellCmd shell = {0};

	// Do a Binary Search
	cmd = line;
	while (start <= end) {
		int comp = esif_ccb_stricmp(cmd, ShellCommands[node].cmd);
		if (comp == 0) {
			int argc = 0;
			char *argv[MAX_ARGV] = {0};
			switch (ShellCommands[node].type) {

			// Command Line argc/argv Parser Support only
			case fnArgv:
				do {
					if (*cmd == ';') {	// break on comment
						break;
					}
					argv[argc++] = cmd;
				} while (argc < MAX_ARGV && (cmd = esif_ccb_strtok(NULL, ESIF_SHELL_STRTOK_SEP, &g_line_context)) != NULL);

				shell.argc   = argc;
				shell.argv   = argv;
				shell.outbuf = output;
				rcStr = (*(ArgvFunc)(ShellCommands[node].func))(&shell);
				break;

			default:
				break;
			}
			return rcStr;
		} else if (comp > 0) {
			start = node + 1;
		} else {
			end = node - 1;
		}
		node = (end - start) / 2 + start;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "ERROR: Unrecognized ESIF Command\n");
	return output;
}

// Execute Shell Command, with repeats if necessary
enum esif_rc esif_shell_execute(char *command)
{
	enum esif_rc esifStatus = ESIF_OK;
	char *pTemp = NULL;
	char *cmd2 = NULL;
	int count = 0;

	ESIF_ASSERT(command != NULL);
	
	if (esif_ccb_strlen(command, MAX_LINE) >= (MAX_LINE - 1)) {
		return ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}

	pTemp = command;
	while (*pTemp != '\0') {
		if (*pTemp == '\r' || *pTemp == '\n' || *pTemp == '#') {
			*pTemp = '\0';
			break;
		}
		pTemp++;
	}

	if ((1 == g_repeat) || esif_ccb_strnicmp(command, "repeat", 6) == 0) {
		parse_cmd(command, ESIF_FALSE);
	}
	else {
		for (count = 0; count < g_repeat; count++) {
			esif_ccb_free(cmd2);
			cmd2 = esif_ccb_strdup(command);
			if (cmd2 != NULL) {
				parse_cmd(cmd2, ESIF_FALSE); /* parse destroys command line */
				if (g_soe && g_errorlevel != 0) {
					esifStatus = g_errorlevel;
					break;
				}

				if (g_repeat_delay && (count + 1 < g_repeat)) {
					esif_ccb_sleep_msec(g_repeat_delay);
				}
			}
		}
		g_repeat = 1;
	}

	esif_ccb_free(cmd2);
	return esifStatus;
}