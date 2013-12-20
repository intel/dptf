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
#include "esif_uf_version.h"

const char *g_esif_shell_version = ESIF_UF_VERSION;

extern char *g_esif_etf_version;
extern char *g_esif_erapi_version;

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

#include "esif_uf_primitive.h"
#include "esif_uf_rest.h"

#include "esif_command.h"	/* Commands */
#include "esif_ipc.h"		/* IPC Abstraction */
#include "esif_primitive.h"	/* Primitive */
#include "esif_cpc.h"		/* Compact Primitive Catalog */

// friend classes
#define _EQLCMD_CLASS
#define _ESIFDATALIST_CLASS

#include "esif_lib_databank.h"
#include "esif_lib_eqlparser.h"
#include "esif_lib_eqlcmd.h"

#include "esif_dsp.h"
#include "esif_uf_fpc.h"
#include "esif_uf_ccb_system.h"
#include "esif_uf_app.h"
extern struct esif_uf_dm g_dm;

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// Shell strings limited to 64k
#define ESIF_SHELL_STRLEN(s)        esif_ccb_strlen(s, OUT_BUF_LEN)

// Alias Dispatcher
char*esif_shell_exec_dispatch (esif_string line, esif_string output);

#define FILE_NAME_STR_LEN 128
#define FILE_READ         "rb"
#define FILE_WRITE        "w"
#define PATH_STR_LEN      128
#define CPC_SIGNATURE     "@CPC"
#define REG_EXPR_STR_LEN  256

// Limits
#define MAX_CPC_SIZE        0x7fffffff
#define MAX_PRIMITIVES      0x7fffffff
#define MAX_ALGORITHMS      0x7fffffff
#define MAX_DOMAINS         0x7fffffff
#define MAX_EVENTS          0x7fffffff

/* Friends */
extern EsifActMgr g_actMgr;
extern EsifAppMgr g_appMgr;
extern EsifCnjMgr g_cnjMgr;
extern EsifUppMgr g_uppMgr;

// Work around strtok_s make work like strtok for now
char g_saved_command[32 + 1];
u32 g_uf_xform = 0;				// TODO: Remove me once it's no longer needed

int g_dst   = 0;
int g_trace = 0;

// StopWatch
struct timeval g_timer = {0};

enum output_format g_format = FORMAT_TEXT;

//
// NOT Declared In Header Only This Module Should Use These
//
extern FILE *g_debuglog;		// Lof File
extern int g_autocpc;			// Automatically Assign CPC?
int g_binary_buf_size = 4096;	// Buffer Size
extern int g_errorlevel;	// Exit Errorlevel
extern int g_ipcmode;		// IPC Mode IOCTL, READ, LAL
extern int g_quit;			// Quit Application?
int g_repeat = 1;		// Repeat N Times
int g_repeat_delay = 0;		// Repeat Delay In msec
extern int g_timestamp;		// Timestamp on / off?
char g_os[64];

int g_soe = 1;

struct EsifShellCmd_s {
	int   argc;
	char  * *argv;
	char  *outbuf;
};

typedef struct EsifShellCmd_s EsifShellCmd, *EsifShellCmdPtr;

// Output Buffer Keep Off Stack
char g_out_buf[(64 * 1024)];

// Use our own strtok() function, which understands "quoted strings with spaces"
#define ESIF_SHELL_STRTOK_SEP   " \t\r\n"
static char *g_line_context;
#undef  esif_ccb_strtok
#define esif_ccb_strtok(str, sep, ctxt) esif_shell_strtok(str, sep, ctxt)
static char*esif_shell_strtok (
	char *str,
	char *seps,
	char * *context
	)
{
	char *result = 0;
	if (!str) {
		str = *context;
	}
	if (str) {
		while (*str && strchr(seps, *str))
			str++;

		if (*str) {
			// Allow "Quoted Strings" with embedded spaces or other separators
			if (*str == '\"') {
				seps = "\"";
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
unsigned int esif_atoi (const esif_string str)
{
	unsigned int val = 0;
	if (NULL == str) {
		return 0;
	}

	if (!strncmp(str, "0x", 2)) {
		esif_ccb_sscanf(str + 2, "%x", &val, sizeof(val));
	} else {
		esif_ccb_sscanf(str, "%d", &val, sizeof(val));
	}

	return val;
}


UInt64 esif_atoi64 (const esif_string str)
{
	UInt64 val = 0;
	if (NULL == str) {
		return 0;
	}

	if (!strncmp(str, "0x", 2)) {
		esif_ccb_sscanf(str + 2, "%llx", &val, sizeof(val));
	} else {
		esif_ccb_sscanf(str, "%lld", &val, sizeof(val));
	}

	return val;
}


static char*esif_primitive_domain_str (
	u16 domain,
	char *str,
	u8 str_len
	)
{
	u8 *ptr = (u8*)&domain;
	esif_ccb_sprintf(str_len, str, "%c%c", *ptr, *(ptr + 1));
	return str;
}


/* Need Header */
char*esif_str_replace (
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
		with = (char*)"";
	}
	len_with = ESIF_SHELL_STRLEN(with);

	/* Count the number of replacement instances */
	for (count = 0; NULL != (tmp = strstr(ins, rep)); ++count)
		ins = tmp + len_rep;

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

	tmp     = result = (char*)esif_ccb_malloc(tmp_len);

	if (NULL == result) {
		return NULL;
	}

	while (count--) {
		ins = strstr(orig, rep);/* Find the replacement in origin */

		len_front = ins - orig;	/* Calculate bytes before replacement */
		esif_ccb_strcpy(tmp, orig, tmp_len);/* Copy entire string */
		tmp += len_front;					/* Move To Repalement Loc */

		/* Copy the replacement @ the replace locaiton */
		esif_ccb_strcpy(tmp, with, (tmp_len - len_front));
		tmp  += len_with;

		orig += len_front + len_rep;// move to next "end of rep"
	}

	/*
	** Append what we have left of orig to the end.
	*/
	esif_ccb_strcpy(tmp, orig, ESIF_SHELL_STRLEN(orig) + 1);
	return result;
}


///////////////////////////////////////////////////////////////////////////////
// INITIALIZE
///////////////////////////////////////////////////////////////////////////////

// Init
void esif_init ()
{
	char command[64];

	EsifString os_arch = NULL;
	EsifString build   = NULL;
	static u8 first    = ESIF_TRUE;

#ifdef ESIF_ATTR_64BIT
	os_arch = (EsifString)"x64";
#else
	os_arch = (EsifString)"x86";
#endif

#ifdef ESIF_ATTR_DEBUG
	build = (EsifString)"Debug";
#else
	build = (EsifString)"Release";
#endif

	esif_ccb_sprintf(sizeof(g_os), g_os, "%s %s %s", ESIF_ATTR_OS, os_arch, build);

	//
	// The endline character must be part of a single string or the prompt is displayed after
	// the endline is sent if it is by itself.  This messes up the display of the banner.
	//
	CMD_OUT("\n\nEEEEEEEEEE   SSSSSSSSSS   IIIIIIIII   FFFFFFFFFF\n"
			"EEE          SSS             III      FFF\n"
			"EEE          SSS             III      FFF\n"
			"EEEEEEEEEE   SSSSSSSSSS      III      FFFFFFFFFF\n"
			"EEE                 SSS      III      FFF\n"
			"EEE                 SSS      III      FFF     OS:      %s\n"
			"EEEEEEEEEE   SSSSSSSSSS   IIIIIIIII   FFF     Version: %s\n\n",
			g_os, g_esif_shell_version);

	if (ESIF_TRUE == first) {
		esif_ccb_sprintf(64, command, "load start");
		parse_cmd(command, ESIF_FALSE);

#ifdef ESIF_ATTR_OS_WINDOWS_UMDF
		EsifRestStart();

		/* Start DPTF */
		esif_ccb_sprintf(64, command, "appstart dptf");
		parse_cmd(command, ESIF_FALSE);

#endif
	}
	first--;
}


// Exit
void esif_uf_subsystem_exit ()
{
}


///////////////////////////////////////////////////////////////////////////////
// PARSED COMMANDS
///////////////////////////////////////////////////////////////////////////////

static char*esif_shell_cmd_dspqacpi (EsifShellCmdPtr shell)
{
	int argc          = shell->argc;
	char * *argv      = shell->argv;
	char *output      = shell->outbuf;
	char *acpi_device = "*";
	char *acpi_uid    = "*";
	char *acpi_type   = "*";
	char *acpi_scope  = "*";

	struct esif_uf_dm_query_acpi qry = {0};
	esif_string dsp_selected = NULL;

	if (argc > 1) {
		acpi_device = argv[1];
	}

	if (argc > 2) {
		acpi_type = argv[2];
	}

	if (argc > 3) {
		acpi_uid = argv[3];
	}

	if (argc > 4) {
		acpi_scope = argv[4];
	}

	qry.acpi_device = acpi_device;
	qry.acpi_type   = acpi_type;
	qry.acpi_uid    = acpi_uid;
	qry.acpi_scope  = acpi_scope;

	dsp_selected    = esif_uf_dm_query(ESIF_UF_DM_QUERY_TYPE_ACPI, &qry);
	if (NULL == dsp_selected) {
		dsp_selected = "NONE";
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nQuery WHERE Clause:\n\n"
					 "acpi_device: %s\n"
					 "acpi_type:   %s\n"
					 "acpi_uid:    %s\n"
					 "acpi_scope:  %s\n"
					 "Result:\n\n"
					 "Your DSP: %s\n\n",
					 acpi_device,
					 acpi_type,
					 acpi_uid,
					 acpi_scope,
					 dsp_selected);

	return output;
}


static char*esif_shell_cmd_dsps (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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

			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s  <dsp>\n", output);
			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%s"
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
							 output,
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

			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s  </dsp>\n", output);
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s</result>\n", output);

		return output;
	}


	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nLoaded Device Support Packages (DSP):\n"
					 "Count:  %u\n\n",
					 g_dm.dme_count);

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "%sACPI Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: = (HID(8) & TYPE(4) & UID(2) & SCOPE(1))\n"
					 "Minterms: 4\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION HID      TYPE UID SCOPE\n"
					 "-- ------------ ------- -------- ---- --- ------------------------------\n", output);

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr || *dsp_ptr->bus_enum != 0) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%02u %-12s %-7s %-8s %-4s %-3s %-30s\n", output,
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->acpi_device,
						 dsp_ptr->acpi_type,
						 "",
						 dsp_ptr->acpi_scope);
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "%s"
					 "\nPCI Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: (VENDOR(128)& DEVICE(64)& BS(32)& DV(16)& FN(8)& RV(4)& SC(2)& PI(1))\n"
					 "Minterms: 8\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION VENDOR DEVICE BS DV FN RV SC PI\n"
					 "-- ------------ ------- ------ ------ -- -- -- -- -- --\n", output);

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr || *dsp_ptr->bus_enum != 1) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%02u %-12s %-7s %-6s %-6s %2s %2s %2s %2s %2s %2s\n", output,
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

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "%s"
					 "\nConjure Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: (GUID(1))\n"
					 "Minterms: 1\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION GUID\n"
					 "-- ------------ ------- ------------------------------------\n", output);

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr	/* || *dsp_ptr->bus_enum != 3 */) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%02u %-12s %-7s %-36s\n", output,
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->type);
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "%s"
					 "\nPlatform Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: (GUID(1))\n"
					 "Minterms: 1\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION GUID\n"
					 "-- ------------ ------- ------------------------------------\n", output);

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr || *dsp_ptr->bus_enum != 2) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%02u %-12s %-7s %-36s\n", output,
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->type);
	}


	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
	return output;
}


static char*esif_shell_cmd_conjures (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	u8 i = 0;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nLoaded Conjures:\n\n"
					 "ID Name         Description                         Type   Version     \n"
					 "-- ------------ ----------------------------------- ------ ------------\n"

					 );
	/* Enuerate Applications */
	for (i = 0; i < ESIF_MAX_CONJURES; i++) {
		EsifCnjPtr a_conjure_ptr    = &g_cnjMgr.fEnrtries[i];
		char desc[ESIF_DESC_LEN]    = "TBD";
		char version[ESIF_DESC_LEN] = "TBD";

		ESIF_DATA(data_desc, ESIF_DATA_STRING, desc, ESIF_DESC_LEN);
		ESIF_DATA(data_version, ESIF_DATA_STRING, version, ESIF_DESC_LEN);

		if (NULL == a_conjure_ptr || NULL == a_conjure_ptr->fLibNamePtr) {
			continue;
		}
		a_conjure_ptr->fInterface.fConjureGetDescriptionFuncPtr(&data_desc);
		a_conjure_ptr->fInterface.fConjureGetVersionFuncPtr(&data_version);

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%02u %-12s %-35s %-6s %-12s\n", output, i,
						 a_conjure_ptr->fLibNamePtr,
						 (esif_string)desc,
						 "plugin",
						 (esif_string)version);
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
	return output;
}


static char*esif_shell_cmd_actions (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	struct esif_link_list_node *curr_ptr = g_actMgr.fActTypes->head_ptr;
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (g_format == FORMAT_XML) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<result>\n");

		while (curr_ptr) {
			EsifActTypePtr cur_actiontype_ptr = (EsifActTypePtr)curr_ptr->data_ptr;
			if (cur_actiontype_ptr != NULL) {
				char *loc_ptr  = " K ";
				char *type_ptr = "plugin";

				if (ESIF_FALSE == cur_actiontype_ptr->fIsKernel) {
					loc_ptr = " U ";
				}

				if (ESIF_FALSE == cur_actiontype_ptr->fIsPlugin) {
					type_ptr = "static";
				}

				esif_ccb_sprintf(OUT_BUF_LEN, output,
								 "%s<action>\n"
								 "    <id>%u</id>\n"
								 "    <name>%s</name>\n"
								 "    <desc>%s</desc>\n"
								 "    <location>%s</location>\n"
								 "    <type>%s</type>\n"
								 "    <version>%s</version>\n"
								 "    <os>%s</os>\n"
								 "  </action>\n",
								 output,
								 cur_actiontype_ptr->fType,
								 cur_actiontype_ptr->fName,
								 cur_actiontype_ptr->fDesc,
								 loc_ptr,
								 type_ptr,
								 cur_actiontype_ptr->fVersion,
								 cur_actiontype_ptr->fOsType);
			}
			curr_ptr = curr_ptr->next_ptr;
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s</result>\n", output);
		return output;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nAvailable Actions:\n\n"
					 "ID Name         Description                         Loc Type   Version      OS Support  \n"
					 "-- ------------ ----------------------------------- --- ------ ------------ ------------\n"
					 );

	while (curr_ptr) {
		EsifActTypePtr cur_actiontype_ptr = (EsifActTypePtr)curr_ptr->data_ptr;
		if (cur_actiontype_ptr != NULL) {
			char *loc_ptr  = " K ";
			char *type_ptr = "plugin";

			if (ESIF_FALSE == cur_actiontype_ptr->fIsKernel) {
				loc_ptr = " U ";
			}

			if (ESIF_FALSE == cur_actiontype_ptr->fIsPlugin) {
				type_ptr = "static";
			}

			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%02u %-12s %-35s %-3s %-6s %-12s %s \n", output,
							 cur_actiontype_ptr->fType,
							 cur_actiontype_ptr->fName,
							 cur_actiontype_ptr->fDesc,
							 loc_ptr,
							 type_ptr,
							 cur_actiontype_ptr->fVersion,
							 cur_actiontype_ptr->fOsType);
		}
		curr_ptr = curr_ptr->next_ptr;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
	return output;
}


static char*esif_shell_cmd_apps (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	u8 i = 0;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nRUNNING APPLICATIONS:\n\n"
					 "ID Name         Description                         Type   Version      Events    \n"
					 "-- ------------ ----------------------------------- ------ ------------ ----------\n");
	/* Enuerate Applications */
	for (i = 0; i < ESIF_MAX_APPS; i++) {
		u8 j = 0;
		EsifAppPtr a_app_ptr = &g_appMgr.fEntries[i];
		char desc[ESIF_DESC_LEN];
		char version[ESIF_DESC_LEN];

		ESIF_DATA(data_desc, ESIF_DATA_STRING, desc, ESIF_DESC_LEN);
		ESIF_DATA(data_version, ESIF_DATA_STRING, version, ESIF_DESC_LEN);

		if (NULL == a_app_ptr || NULL == a_app_ptr->fLibNamePtr) {
			continue;
		}
		a_app_ptr->fInterface.fAppGetDescriptionFuncPtr(&data_desc);
		a_app_ptr->fInterface.fAppGetVersionFuncPtr(&data_version);

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%02u %-12s %-35s %-6s %-12s 0x%016llx\n", output, i,
						 a_app_ptr->fLibNamePtr,
						 (esif_string)data_desc.buf_ptr,
						 "plugin",
						 (esif_string)data_version.buf_ptr,
						 a_app_ptr->fRegisteredEvents);

		/* Now temproary dump particiapnt events here */
		for (j = 0; j < MAX_PARTICIPANT_ENTRY; j++) {
			u8 k = 0;
			if (a_app_ptr->fParticipantData[j].fAppParticipantHandle == NULL) {
				continue;
			}
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s = 0x%016llx Participant Registered Events\n", output,
							 a_app_ptr->fParticipantData[j].fUpPtr->fMetadata.fName,
							 a_app_ptr->fParticipantData[j].fRegisteredEvents);

			for (k = 0; k < MAX_DOMAIN_ENTRY; k++) {
				if (a_app_ptr->fParticipantData[j].fDomainData[k].fAppDomainHandle == NULL) {
					continue;
				}
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s  D%d = 0x%016llx Domain Registered Events\n", output,
								 k,
								 a_app_ptr->fParticipantData[j].fDomainData[k].fRegisteredEvents);
			}
		}
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%02u %s %s    0xffffffffffffffff\n",
					 output, g_appMgr.fEntryCount,
					 "esif         ESIF Shell                          static",
					 g_esif_shell_version);

	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s", output,
					 "\nHint To manage / connnect use appselect name e.g. appselect esif\n\n");
	return output;
}


static char*esif_shell_cmd_actionstart (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *lib_name  = 0;
	EsifActPtr a_action_ptr = NULL;
	u8 i = 0;

	if (argc < 2) {
		return NULL;
	}

	/* Parse The Library Nme */
	lib_name = argv[1];

	/* Check to see if the action is already running only one instance per action allowed */
	if (NULL != g_actMgr.GetActFromName(&g_actMgr, lib_name)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Action %s Already Started.\n", lib_name);
		goto exit;
	}

	for (i = 0; i < ESIF_MAX_ACTIONS; i++)
		if (NULL == g_actMgr.fEnrtries[i].fLibNamePtr) {
			break;
		}


	if (ESIF_MAX_ACTIONS == i) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Maximum Actions Reached %u.\n", i);
		goto exit;
	} else {
		g_actMgr.fEntryCount++;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Action Sandbox %u Selected.\n", i);
	}


	a_action_ptr = &g_actMgr.fEnrtries[i];
	a_action_ptr->fLibNamePtr = (esif_string)esif_ccb_strdup(lib_name);

	rc = EsifActStart(a_action_ptr);
	if (ESIF_OK != rc) {
		/* Cleanup */
		esif_ccb_free(a_action_ptr->fLibNamePtr);
		memset(a_action_ptr, 0, sizeof(*a_action_ptr));
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Start Action: %s.\n", lib_name);
	} else {
		/* Proceed */
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%sStarted Action: %s Instance %u Max %u Running %u\n\n",
						 output, lib_name, i, ESIF_MAX_ACTIONS, g_actMgr.fEntryCount);
	}

exit:
	return output;
}


static char*esif_shell_cmd_appstart (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *lib_name  = 0;
	EsifAppPtr a_app_ptr = NULL;
	u8 i = 0;

	/* Parse The Library Nme */
	if (argc < 2) {
		return NULL;
	}
	lib_name = argv[1];

	/* Check to see if the app is already running only one instance per app allowed */
	if (NULL != g_appMgr.GetAppFromName(&g_appMgr, lib_name)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Application %s Already Started.\n", lib_name);
		goto exit;
	}

	for (i = 0; i < ESIF_MAX_APPS; i++)
		if (NULL == g_appMgr.fEntries[i].fLibNamePtr) {
			break;
		}


	if (ESIF_MAX_APPS == i) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Maximum Applicaitons Reached %u.\n", i);
		goto exit;
	} else {
		g_appMgr.fEntryCount++;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Application Sandbox %u Selected.\n", i);
	}

	a_app_ptr = &g_appMgr.fEntries[i];
	a_app_ptr->fLibNamePtr = (esif_string)esif_ccb_strdup(lib_name);
	rc = EsifAppStart(a_app_ptr);

	if (ESIF_OK != rc) {
		/* Cleanup */
		esif_ccb_free(a_app_ptr->fLibNamePtr);
		memset(a_app_ptr, 0, sizeof(*a_app_ptr));
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Start App: %s Reason %s(%d).\n", lib_name, esif_rc_str(rc), rc);
	} else {
		/* Proceed */
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%sStarted App: %s Instance %u Max %u Running %u\n\n",
						 output, lib_name, i, ESIF_MAX_APPS, g_appMgr.fEntryCount);
	}

exit:
	return output;
}


static char*esif_shell_cmd_actionstop (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *lib_name  = 0;
	EsifActPtr a_action_ptr = NULL;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}
	lib_name     = argv[1];

	a_action_ptr = g_actMgr.GetActFromName(&g_actMgr, lib_name);
	if (NULL == a_action_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find Action: %s.\n", lib_name);
		goto exit;
	}

	// rc = EsifActionStop(a_action_ptr);
	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Stop Action: %s.\n", lib_name);
	} else {
		/* Proceed */
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%sStopped Action: %s\n\n",
						 output, lib_name);
		esif_ccb_free(a_action_ptr->fLibNamePtr);
		a_action_ptr->fLibNamePtr = NULL;
		g_actMgr.fEntryCount--;
	}

exit:

	return output;
}


static char*esif_shell_cmd_appstop (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *lib_name  = 0;
	EsifAppPtr a_app_ptr = NULL;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}
	lib_name  = argv[1];

	a_app_ptr = g_appMgr.GetAppFromName(&g_appMgr, lib_name);
	if (NULL == a_app_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find App: %s.\n", lib_name);
		goto exit;
	}

	rc = EsifAppStop(a_app_ptr);
	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Stop App: %s.\n", lib_name);
	} else {
		/* Proceed */
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%sStopped App: %s\n\n",
						 output, lib_name);
		esif_ccb_free(a_app_ptr->fLibNamePtr);
		a_app_ptr->fLibNamePtr = NULL;
		g_appMgr.fEntryCount--;
	}
exit:
	return output;
}


static char*esif_shell_cmd_appselect (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *lib_name  = 0;

	EsifAppPtr a_app_ptr = NULL;
	struct esif_data data_banner;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}
	lib_name = argv[1];

	/* Works But Ugly */
	if (!strcmp(lib_name, "esif")) {
		g_appMgr.fSelectedAppPtr = NULL;
		esif_init();
		goto exit;
	}

	a_app_ptr = g_appMgr.GetAppFromName(&g_appMgr, lib_name);
	if (NULL == a_app_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find App: %s.\n", lib_name);
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


// Auto CPC
static char*esif_shell_cmd_autocpc (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	char *auto_cpc;

	if (argc < 2) {
		return NULL;
	}
	auto_cpc = argv[1];

	if (!strcmp(auto_cpc, "on")) {
		g_autocpc = 1;
	} else {
		g_autocpc = 0;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "autocpc=%d\n", g_autocpc);
	return output;
}


// Stop On Error
static char*esif_shell_cmd_soe (EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char * *argv  = shell->argv;
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
static char*esif_shell_cmd_debuglvl (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc *ipc_ptr = NULL;
	struct esif_command_set_debug_module_level data;
	const u32 data_len = sizeof(struct esif_command_set_debug_module_level);
	struct esif_ipc_command *command_ptr = NULL;

	if (argc < 3) {
		return NULL;
	}

	data.module = esif_atoi(argv[1]);
	data.level  = esif_atoi(argv[2]);

	ipc_ptr     = esif_ipc_alloc_command(&command_ptr, data_len);
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

	// Command
	esif_ccb_memcpy((command_ptr + 1), &data, data_len);
	ipc_execute(ipc_ptr);

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

	esif_ccb_sprintf(OUT_BUF_LEN, output, "module = %d, level = 0x%08X\n", data.module, data.level);
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// Debug Set
static char*esif_shell_cmd_debugset (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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


// TODO Use g_uf_xform to switch who (LF or UF) should run the xform. There is no need
// to do so, as soon as the xform fully runs on UF (Kernel xform will retire)
static char*esif_shell_cmd_varset (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	u32 val;
	char *valstr;

	// Format: varset var_name = val, Example: varset i = 100
	if (argc < 2) {
		return NULL;
	}

	// Get the name in string, but we don't use it yet!
	if ((valstr = strchr(argv[argc - 1], '=')) != 0) {
		*valstr++ = 0;
	} else {
		valstr = argv[argc - 1];
	}

	// Get the val
	val = esif_atoi(valstr);
	if (val) {
		g_uf_xform = 1;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "g_uf_xform = 1 (User-Mode ESIF Upper Framework Temp/Power Xform)\n");
	} else {
		g_uf_xform = 0;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "g_uf_xform = 0 (Kernel-Mode ESIF Lower Framework Temp/Power Xform)\n");
	}
	return output;
}


// Trace
char*esif_shell_cmd_trace (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;

	if (argc > 1) {
		g_traceLevel = (u8)esif_atoi(argv[1]);
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "Trace Level = %d\n", g_traceLevel);

	ESIF_TRACE_VERBOSE_EX("VERBOSE_EX Message\n");
	ESIF_TRACE_VERBOSE("VERBOSE Message\n");
	ESIF_TRACE_DEBUG("DEBUG Message\n");
	ESIF_TRACE_INFO("INFO Message\n");
	ESIF_TRACE_DYN(ESIF_TRACEMODULE, ESIF_TRACELEVEL_NOTICE, "NOTICE Message\n");
	ESIF_TRACE_WARN("WARN Message\n");
	ESIF_TRACE_ERROR("ERROR Message\n");
	ESIF_TRACE_DYN(ESIF_TRACEMODULE, ESIF_TRACELEVEL_CRITICAL, "CRITICAL Message\n");
	ESIF_TRACE_DYN(ESIF_TRACEMODULE, ESIF_TRACELEVEL_ALERT, "ALERT Message\n");
	ESIF_TRACE_DYN(ESIF_TRACEMODULE, ESIF_TRACELEVEL_FATAL, "FATAL Message\n");
	return output;
}


// Select Destination By ID
char*esif_shell_cmd_dst (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_dst = (u8)esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "destination participant = %d selected\n", g_dst);
	return output;
}


// Select Destination By Name
char*esif_shell_cmd_dstn (EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char * *argv   = shell->argv;
	char *output   = shell->outbuf;
	char *name_str = 0;
	EsifUpPtr up_ptr = NULL;

	if (argc < 2) {
		return NULL;
	}
	name_str = argv[1];

	up_ptr = EsifUpManagerGetAvailableParticipantByName(name_str);

	if(up_ptr != NULL) {
		g_dst = up_ptr->fInstance;

		esif_ccb_sprintf(OUT_BUF_LEN, output, "destination %s participant = %d selected\n", name_str, g_dst);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "destination %s participant = %d not found\n", name_str, g_dst);
	}
	return output;
}


// Echo
static char*esif_shell_cmd_echo (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	int j;
	for (j = 1; j < argc; j++)
		esif_ccb_sprintf(OUT_BUF_LEN - ESIF_SHELL_STRLEN(output), output + ESIF_SHELL_STRLEN(output), "%s%s", argv[j], (j + 1 < argc ? " " : "\n"));

	return output;
}


// Format
static char*esif_shell_cmd_format (EsifShellCmdPtr shell)
{
	int argc         = shell->argc;
	char * *argv     = shell->argv;
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


static void dump_binary_object (u8 *byte_ptr, u32 byte_len);
static void dump_table_hdr (struct esif_table_hdr *tab);
static void dump_binary_data (u8 *byte_ptr, u32 byte_len);

// Get From File
static char*esif_shell_cmd_getf (EsifShellCmdPtr shell)
{
	int argc         = shell->argc;
	char * *argv     = shell->argv;
	char *output     = shell->outbuf;
	char *filename   = 0;
	char full_path[FILE_NAME_STR_LEN];
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
	esif_ccb_sprintf(FILE_NAME_STR_LEN, full_path, "%s", esif_build_path(full_path, FILE_NAME_STR_LEN, ESIF_DIR_BIN, filename));

	esif_ccb_fopen(&fp_ptr, full_path, (char*)FILE_READ);
	if (NULL != fp_ptr) {
		struct stat file_stat = {0};
		esif_ccb_stat(full_path, &file_stat);
		file_size = file_stat.st_size;
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: file not found (%s)\n", ESIF_FUNC, full_path);
		goto exit;
	}

	// allocate space
	buf_ptr = (u8*)esif_ccb_malloc(file_size);
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
static char*esif_shell_cmd_getp (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;

	int optargc     = 0;	// Simulates Command Line Argument Count
	char *optargv[32];	// Simulates Command Line Arguments
	int opt     = 1;

	u32 id      = 0;
	char *qualifier_str = "D0";
	u8 instance = 255;
	char full_path[PATH_STR_LEN];
	char desc[32];
	u8 *data_ptr  = NULL;
	u16 qualifier = 0;

	struct esif_data request = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data response;

	char *suffix = 0;
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
	qualifier = convert_string_to_short(qualifier_str);

	// Instance ID
	if (opt < argc) {
		instance = (u8)esif_atoi(argv[opt++]);
	}

	// Optional, Additional Argument, Integer Only For Now
	if (opt < argc) {
		add_data        = esif_atoi(argv[opt++]);

		request.type    = ESIF_DATA_UINT32;
		request.buf_len = sizeof(add_data);
		request.buf_ptr = (void*)&add_data;
	}

	// For File Dump
	if (2 == dump) {
		char *filename = 0;
		if (argc <= opt) {
			return NULL;
		}
		filename = argv[opt++];
		esif_ccb_sprintf(PATH_STR_LEN, full_path, "%s.bin", esif_build_path(full_path, PATH_STR_LEN, ESIF_DIR_BIN, filename));
	}

	// Create a pseudo argc/argv to pass to Test functions
	esif_ccb_memcpy(&optargv[0], &argv[opt], sizeof(char*) * (argc - opt));
	optargc += argc - opt;

	// Setup Response
	response.type = type;
	if (ESIF_DATA_ALLOCATE != buf_size) {
		data_ptr = (u8*)esif_ccb_malloc(buf_size);
		if (NULL == data_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ccb_malloc failed for %u bytes\n",
							 ESIF_FUNC, buf_size);
			goto exit;
		}
		response.buf_ptr  = (void*)data_ptr;
		response.buf_len  = buf_size;
		response.data_len = 0;
	} else {
		response.buf_len  = ESIF_DATA_ALLOCATE;
		response.buf_ptr  = NULL;
		response.data_len = 0;
	}

	rc = EsifExecutePrimitive(
			(u8)g_dst,
			id,
			qualifier_str,
			instance,
			&request,
			&response);

	data_ptr = (u8*)response.buf_ptr;

	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s getp%s(%03u.%s.%03d)",
					 esif_primitive_str((enum esif_primitive_type)id), suffix, id, qualifier_str, instance);

	if (ESIF_E_NEED_LARGER_BUFFER == rc) {
		//
		// PRIMITIVE Error Code NEED Larger Buffer
		//
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s error code = %s(%d) HAVE %u bytes NEED ATLEAST %u bytes\n",
						 output,
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
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s info code = %s(%d)\n",
						 output, esif_rc_str(rc), rc);

		goto exit;
	} else if (ESIF_OK != rc) {
		//
		// PRIMITIVE Error Code
		//
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s: error code = %s(%d)\n",
						 output, ESIF_FUNC, esif_rc_str(rc), rc);
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
		esif_ccb_strcpy(&desc[0], "", 32);
		break;

	case ESIF_DATA_TEMPERATURE:
		esif_ccb_strcpy(&desc[0], "Degrees C", 32);
		break;

	case ESIF_DATA_POWER:
		esif_ccb_strcpy(&desc[0], "MilliWatts", 32);
		break;

	case ESIF_DATA_STRING:
		esif_ccb_strcpy(&desc[0], "ASCII", 32);
		break;

	case ESIF_DATA_BINARY:
		esif_ccb_strcpy(&desc[0], "BINARY", 32);
		break;

	case ESIF_DATA_TABLE:
		esif_ccb_strcpy(&desc[0], "TABLE", 32);
		break;

	case ESIF_DATA_PERCENT:
		esif_ccb_strcpy(&desc[0], "Percent", 32);
		break;

	default:
		esif_ccb_strcpy(&desc[0], "", 32);
		break;
	}

	if (ESIF_DATA_UINT32 == type ||
		ESIF_DATA_TEMPERATURE == type ||
		ESIF_DATA_POWER == type) {
		// Our Data
		u32 val = *(u32*)(response.buf_ptr);
		if (optargc > 1) {
			// Have Testable Data
			eEsifTestErrorType rc = EsifTestPrimitive(id, qualifier, instance, val, optargc, optargv);

			if (ESIF_TEST_OK == rc) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s value = %u(0x%08x): %s\n",
								 output, val, val, ESIF_TEST_PASSED);
			} else {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s value = %u(0x%08x): %s %s(%d)\n",
								 output, val, val, ESIF_TEST_FAILED, EsifTestErrorStr(rc), rc);
				g_errorlevel = rc;
			}
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
		} else {
			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s value = 0x%08x %u %s\n",
								 output, val, val, desc);
			} else {
				esif_ccb_sprintf(OUT_BUF_LEN, output,
								 "%s<result>\n"
								 "    <value>%u</value>\n"
								 "    <valueDesc>%s</valueDesc>\n",
								 output, val, desc);
			}
		}
	} else if (ESIF_DATA_STRING == type) {
		char *str_ptr = (char*)(response.buf_ptr);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s string(%u of %u) = %s\n",
						 output,
						 response.data_len,
						 response.buf_len,
						 str_ptr);
	} else {// Binary Dump
		u8 *byte_ptr = (u8*)(response.buf_ptr);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s Binary Data(%u of %u):",
						 output,
						 response.data_len,
						 response.buf_len);

		// Have Testable Data
		if (optargc > 1) {
			eEsifTestErrorType rc = EsifTestPrimitiveBinary(id, qualifier, instance, byte_ptr,
															(u16)response.data_len, optargc, optargv);

			if (ESIF_TEST_OK == rc) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s %s\n",
								 output, ESIF_TEST_PASSED);
			} else {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s %s %s(%d)\n",
								 output, ESIF_TEST_FAILED, EsifTestErrorStr(rc), rc);
				g_errorlevel = rc;
			}
		} else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s %s\n",
							 output, ESIF_TEST_SKIPPED);
		}

		if (0 == dump) {
			if (ESIF_DATA_TABLE == type) {
				dump_table_hdr((struct esif_table_hdr*)byte_ptr);
			} else {
				dump_binary_object(byte_ptr, response.data_len);
			}
		} else if (1 == dump) {
			dump_binary_data(byte_ptr, response.data_len);
		} else if (2 == dump) {
			FILE *fp_ptr = NULL;
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s Binary Data To File: %s (%u of %u):\n",
							 output,
							 full_path,
							 response.data_len,
							 response.buf_len);

			esif_ccb_fopen(&fp_ptr, full_path, (char*)"wb");

			if (fp_ptr) {
				fwrite(byte_ptr, 1, response.data_len, fp_ptr);
				fclose(fp_ptr);
			}
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
	}

	if (FORMAT_XML == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s</result>\n", output);
	}
exit:
	if (NULL != data_ptr) {
		esif_ccb_free(data_ptr);
	}
	return output;
}


// CPC Info
static char*esif_shell_cmd_infocpc (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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
	struct esif_cpc_primitive *primitive_ptr = NULL;
	struct esif_cpc_action *action_ptr = NULL;
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
	key		  = EsifData_CreateAs(ESIF_DATA_STRING, edp_name, 0, ESIFAUTOLEN);
	value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	esif_build_path(edp_full_path, PATH_STR_LEN, ESIF_DIR_DSP, edp_name);
	if ((ESIF_EDP_DV_PRIORITY == 1 || !esif_ccb_file_exists(edp_full_path)) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		esif_ccb_strcpy(edp_full_path, edp_name, PATH_STR_LEN);
		IOStream_SetMemory(io_ptr, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(io_ptr, edp_full_path, (char*)FILE_READ);
	}
	if (IOStream_Open(io_ptr) == 0) {
		struct edp_dir edp_dir;
		size_t r_bytes;

		/* FIND CPC within EDP file */
		r_bytes  = IOStream_Read(io_ptr, &edp_dir, sizeof(struct edp_dir));
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
	cpc_ptr = (struct esif_lp_cpc*)esif_ccb_malloc(cpc_size);
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
	if (cpc_ptr->header.cpc.signature != *(unsigned int*)CPC_SIGNATURE) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: signature validation failure not a cpc file\n",
						 ESIF_FUNC);
		goto exit;
	}

	// parse and dump
	byte_ptr = (u8*)&cpc_ptr->header.cpc.signature;
	if (NULL == byte_ptr ||
		cpc_ptr->number_of_basic_primitives > MAX_PRIMITIVES ||
		cpc_ptr->number_of_algorithms > MAX_ALGORITHMS ||
		cpc_ptr->number_of_domains > MAX_DOMAINS ||
		cpc_ptr->number_of_events > MAX_EVENTS) {
		goto exit;
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "CPC File Info For %s Size %u:\n\n",
						 edp_filename, cpc_size);
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s"
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
						 output,
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
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s  <primitives>\n", output);
	}

	primitive_ptr = (struct esif_cpc_primitive*)((u8*)cpc_ptr + sizeof(struct esif_lp_cpc));
	for (i = 0; i < cpc_ptr->number_of_basic_primitives; i++) {
		int show = 0;
		char temp_buf[1024];

		if (!strcmp(cpc_pattern, "")) {
			show = 1;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(1024, temp_buf, "%03d.%s.%03d %s actions %d %s\n",
							 primitive_ptr->tuple.id,
							 esif_primitive_domain_str(primitive_ptr->tuple.domain, cpc_domain_str, 8),
							 primitive_ptr->tuple.instance,
							 esif_primitive_opcode_str((enum esif_primitive_opcode)primitive_ptr->operation),
							 primitive_ptr->number_of_actions,
							 esif_primitive_str((enum esif_primitive_type)primitive_ptr->tuple.id));
			// TODO REGEXP Here For Now Simple Pattern Match
			if (0 == show && strstr(temp_buf, cpc_pattern)) {
				show = 1;
			}
			// Show Row?
			if (1 == show) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s", output, temp_buf);
			}

			action_ptr = (struct esif_cpc_action*)((u8*)primitive_ptr +
												   sizeof(struct esif_cpc_primitive));
			for (j = 0; j < primitive_ptr->number_of_actions; j++, action_ptr++) {
				if (!strcmp(cpc_pattern, "")) {
					show = 1;
				}
				esif_ccb_sprintf(1024, temp_buf, " %s param_valid %1d:%1d:%1d:%1d:%1d param %x:%x:%x:%x:%x\n",
								 esif_action_type_str(action_ptr->type),
								 action_ptr->param_valid[0],
								 action_ptr->param_valid[1],
								 action_ptr->param_valid[2],
								 action_ptr->param_valid[3],
								 action_ptr->param_valid[4],
								 action_ptr->param[0],
								 action_ptr->param[1],
								 action_ptr->param[2],
								 action_ptr->param[3],
								 action_ptr->param[4]);

				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				if (1 == show) {
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s", output, temp_buf);
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
							 primitive_ptr->tuple.id,
							 esif_primitive_str((enum esif_primitive_type)primitive_ptr->tuple.id),
							 esif_primitive_domain_str(primitive_ptr->tuple.domain, cpc_domain_str, 8),
							 primitive_ptr->tuple.instance,
							 primitive_ptr->operation,
							 esif_primitive_opcode_str((enum esif_primitive_opcode)primitive_ptr->operation),
							 primitive_ptr->number_of_actions);
			// TODO REGEXP Here For Now Simple Pattern Match
			if (0 == show && strstr(temp_buf, cpc_pattern)) {
				show = 1;
			}
			// Show Row?
			if (1 == show) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s", output, temp_buf);
			}

			action_ptr = (struct esif_cpc_action*)((u8*)primitive_ptr +
												   sizeof(struct esif_cpc_primitive));
			for (j = 0; j < primitive_ptr->number_of_actions; j++, action_ptr++) {
				if (!strcmp(cpc_pattern, "")) {
					show = 1;
				}
				esif_ccb_sprintf(1024, temp_buf,
								 "   <action>\n"
								 "      <type>%d</type>\n"
								 "      <typeStr>%s</typeStr>\n"
								 "      <numValidParam>%d</numValidParam>\n"
								 "      <paramValid>%1d:%1d:%1d:%1d:%1d</paramValid>\n"
								 "      <param[0]>%x</param[0]>\n"
								 "      <param[1]>%x</param[1]>\n"
								 "      <param[2]>%x</param[2]>\n"
								 "      <param[3]>%x</param[3]>\n"
								 "      <param[4]>%x</param[4]>\n"
								 "   </action>\n",
								 action_ptr->type,
								 esif_action_type_str(action_ptr->type),
								 action_ptr->num_valid_params,
								 action_ptr->param_valid[0],
								 action_ptr->param_valid[1],
								 action_ptr->param_valid[2],
								 action_ptr->param_valid[3],
								 action_ptr->param_valid[4],
								 action_ptr->param[0],
								 action_ptr->param[1],
								 action_ptr->param[2],
								 action_ptr->param[3],
								 action_ptr->param[4]);
				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				if (1 == show) {
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s", output, temp_buf);
				}
			}
		}

		// Next Primitive In CPC Set
		primitive_ptr = (struct esif_cpc_primitive*)((u8*)primitive_ptr + primitive_ptr->size);
	}

	if (FORMAT_XML == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s  </primitives>\n", output);
	}

	if (FORMAT_TEXT == g_format) {
		u8 algo_index   = 0;
		u8 domain_index = 0;
		u8 event_index  = 0;
		struct esif_fpc_algorithm *algo_ptr = (struct esif_fpc_algorithm*)primitive_ptr;
		struct esif_fpc_domain *domain_ptr  = NULL;
		struct esif_cpc_event *event_ptr    = NULL;

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s"
						 "ALGORITHMS:\n\n"
						 "Action Description           tempxform tc1      tc2      pxform timexform\n"
						 "------ --------------------- --------- -------- -------- ------ ---------\n"
						 , output);

		for (algo_index = 0; algo_index < cpc_ptr->number_of_algorithms; algo_index++) {
			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%s%-2d     %-21s %s(%d) %08x %08x %s(%d) %s(%d)\n",
							 output,
							 algo_ptr->action_type,
							 esif_action_type_str(algo_ptr->action_type),
							 esif_algorithm_type_str((enum esif_algorithm_type)(algo_ptr->temp_xform)),
							 algo_ptr->temp_xform,
							 algo_ptr->tempC1,
							 algo_ptr->tempC2,
							 esif_algorithm_type_str((enum esif_algorithm_type)(algo_ptr->power_xform)),
							 algo_ptr->power_xform,
							 esif_algorithm_type_str((enum esif_algorithm_type)(algo_ptr->time_xform)),
							 algo_ptr->time_xform);
			algo_ptr++;
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s"
						 "DOMAINS:\n\n"
						 "Name     Description          Domain   Priority Capability Type               \n"
						 "-------- -------------------- -------- -------- ---------- -------------------\n"
						 , output);


		domain_ptr = (struct esif_fpc_domain*)algo_ptr;
		for (domain_index = 0; domain_index < cpc_ptr->number_of_domains; domain_index++) {
			char qualifier_str[64];

			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%s%-8s %-20s %-8s %08x %08x   %s(%d)\n",
							 output,
							 domain_ptr->descriptor.name,
							 domain_ptr->descriptor.description,
							 esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifier_str, 64),
							 domain_ptr->descriptor.priority,
							 domain_ptr->capability_for_domain.capability_flags,
							 esif_domain_type_str(domain_ptr->descriptor.domainType), domain_ptr->descriptor.domainType);

			// esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s\n", output, esif_guid_print((esif_guid_t*) &domain_ptr->descriptor.guid, qualifier_str));
			domain_ptr++;
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s"
						 "Mapped Events:\n\n"
						 "Alias Event Identification             Group    Event Data           Event Description\n"
						 "----- -------------------------------- -------- -------------------- --------------------------------------\n"
						 , output);

		event_ptr = (struct esif_cpc_event*)domain_ptr;
		for (event_index = 0; event_index < cpc_ptr->number_of_events; event_index++) {
			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%s"
							 "%5s ",
							 output,
							 event_ptr->name);

			for (i = 0; i < 16; i++)
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%02x", output, event_ptr->event_key[i]);

			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%s "
							 "%-8s %-20s %s\n", output,
							 esif_event_group_enum_str(event_ptr->esif_group) + 17,
							 esif_data_type_str(event_ptr->esif_group_data_type),
							 esif_event_type_str(event_ptr->esif_event) + 11);


/*
            esif_ccb_sprintf(OUT_BUF_LEN, output,
                "%s"
                "%5s %08x       ACPI %s(%d) TBD(%d)\n",
                output,
                event_ptr->name,
                (u8) event_ptr->s[0],
                esif_event_type_str(event_ptr->esif_event), event_ptr->esif_event,
                event_ptr->esif_group);
 */

			// esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s\n", output, esif_guid_print((esif_guid_t*) &domain_ptr->descriptor.guid, qualifier_str));
			event_ptr++;
		}


		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s</cpcinfo>\n", output);
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
static char*esif_shell_cmd_load (EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char * *argv   = shell->argv;
	char *output   = shell->outbuf;
	char *filename = 0;
	char outline[MAX_LINE];
	int outline_len;
	char *ptr;
	int opt       = 1;
	char load_full_path[PATH_STR_LEN];
	int cat       = 0;
	char *checkDV = 0;	// Default Namespace to check ("_dsp", "_cmd", etc), if any
	char *path    = "";
	IOStreamPtr io_ptr = IOStream_Create();

	UNREFERENCED_PARAMETER(output);

	if (argc <= opt || io_ptr == NULL) {
		IOStream_Destroy(io_ptr);
		return NULL;
	}

	// Deduce cat and path options based on command
	if (esif_ccb_stricmp(argv[0], "cat") == 0) {
		cat  = 1;
		path = esif_build_path(load_full_path, PATH_STR_LEN, ESIF_DIR_CMD, NULL);
	} else if (esif_ccb_stricmp(argv[0], "cattst") == 0) {
		checkDV = ESIF_DSP_NAMESPACE;
		cat     = 1;
		path    = esif_build_path(load_full_path, PATH_STR_LEN, ESIF_DIR_DSP, NULL);
	} else if (esif_ccb_stricmp(argv[0], "load") == 0) {
		cat  = 0;
		path = esif_build_path(load_full_path, PATH_STR_LEN, ESIF_DIR_CMD, NULL);
	} else if (esif_ccb_stricmp(argv[0], "loadtst") == 0) {
		checkDV = ESIF_DSP_NAMESPACE;
		cat     = 0;
		path    = esif_build_path(load_full_path, PATH_STR_LEN, ESIF_DIR_DSP, NULL);
	} else if (esif_ccb_stricmp(argv[0], "proof") == 0) {
		cat  = 2;
		path = esif_build_path(load_full_path, PATH_STR_LEN, ESIF_DIR_CMD, NULL);
	} else if (esif_ccb_stricmp(argv[0], "prooftst") == 0) {
		checkDV = ESIF_DSP_NAMESPACE;
		cat     = 2;
		path    = esif_build_path(load_full_path, PATH_STR_LEN, ESIF_DIR_DSP, NULL);
	}

	// load file $1 $2 $3 $4 ...
	// Filename
	filename = argv[opt++];

	// Look for Script either on disk or in a DataVault (static or file), depending on priority setting
	if (checkDV && (ESIF_EDP_DV_PRIORITY == 1 || !esif_ccb_file_exists(filename))) {
		EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, checkDV, 0, ESIFAUTOLEN);
		EsifDataPtr key		  = EsifData_CreateAs(ESIF_DATA_STRING, filename, 0, ESIFAUTOLEN);
		EsifDataPtr value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		if (EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
			esif_ccb_strcpy(load_full_path, filename, PATH_STR_LEN);
			IOStream_SetMemory(io_ptr, (BytePtr)value->buf_ptr, value->data_len);
		}
		EsifData_Destroy(nameSpace);
		EsifData_Destroy(key);
		EsifData_Destroy(value);
	}
	if (IOStream_GetType(io_ptr) == StreamNull) {
		esif_ccb_sprintf(PATH_STR_LEN, load_full_path, "%s%s%s", path, ESIF_PATH_SEP, filename);
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
					replaced = esif_str_replace(outline, (char*)"$dst$", dst_str);
					if (replaced) {
						esif_ccb_strcpy(outline, replaced, MAX_LINE);
						esif_ccb_free(replaced);
					}
				}
			}

			if (cat) {	// proof or cat NOT Load
				printf("%s", outline);
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
				// 6715
				// if (g_errorlevel != 0)
				// break;
			}
		}
		IOStream_Close(io_ptr);
	}
	IOStream_Destroy(io_ptr);
	return output;
}


// Load CPC
static char*esif_shell_cmd_loadcpc (EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char * *argv   = shell->argv;
	char *output   = shell->outbuf;
	char *filename = 0;
	char cpc_full_path[PATH_STR_LEN];

	UNREFERENCED_PARAMETER(output);
	UNREFERENCED_PARAMETER(cpc_full_path);

	if (argc < 2) {
		return NULL;
	}

	filename = argv[1];

	// 10/14/2013 - DWP - Disabled. This is broken
	// esif_ccb_sprintf(PATH_STR_LEN, cpc_full_path, "%s.edp", esif_build_path(cpc_full_path, PATH_STR_LEN, ESIF_DIR_DSP, filename));
	// esif_send_dsp(cpc_full_path, (u8) g_dst);
	return NULL;
}


// Log
static char*esif_shell_cmd_log (EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char * *argv   = shell->argv;
	char *output   = shell->outbuf;
	char *filename = 0;
	char log_full_path[PATH_STR_LEN];

	if (argc < 2) {
		return NULL;
	}

	// Logfile
	filename = argv[1];

	esif_ccb_sprintf(PATH_STR_LEN, log_full_path, "%s",
					 esif_build_path(log_full_path, PATH_STR_LEN, ESIF_DIR_LOG, filename));
	esif_ccb_fopen(&g_debuglog, log_full_path, (char*)FILE_WRITE);

	esif_ccb_sprintf(OUT_BUF_LEN, output, "log %s\n", log_full_path);
	return output;
}


// Memstats
static char*esif_shell_cmd_memstats (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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
	*(u32*)(command_ptr + 1) = reset;
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
	data_ptr = (struct esif_command_get_memory_stats*)(command_ptr + 1);
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
						 "    <memTypeAllocs>%u</memTypeAllocs>\n"
						 "    <memTypeFrees>%u</memTypeFrees>\n"
						 "    <memTypeInuse>%u</memTypeInuse>\n"
						 "    <memTypeObjAllocs>%u</memTypeObjAllocs>\n"
						 "    <memTypeObjFrees>%u</memTypeObjFrees>\n"
						 "    <memTypeObjInuse>%u</memTypeObjInuse>\n"
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
						 data_ptr->stats.memPoolObjAllocs - data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memTypeAllocs,
						 data_ptr->stats.memTypeFrees,
						 data_ptr->stats.memTypeAllocs - data_ptr->stats.memTypeFrees,
						 data_ptr->stats.memTypeObjAllocs,
						 data_ptr->stats.memTypeObjFrees,
						 data_ptr->stats.memTypeObjAllocs - data_ptr->stats.memTypeObjFrees);
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%sMemory Pools:\n"
						 "Name                      Tag  Size Allocs       Frees        Inuse        Bytes    \n"
						 "------------------------- ---- ---- ------------ ------------ ------------ ---------\n", output);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s  <mempools>\n", output);
	}

	for (i = 0; i < ESIF_MEMPOOL_TYPE_MAX; i++) {
		char tag[8] = {0};
		esif_ccb_memcpy(tag, &data_ptr->mempool_stat[i].pool_tag, 4);
		if (0 == data_ptr->mempool_stat[i].pool_tag) {
			continue;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%-25s %s %-4d %-12u %-12u %-12u %-9u\n",
							 output,
							 data_ptr->mempool_stat[i].name,
							 tag,
							 data_ptr->mempool_stat[i].object_size,
							 data_ptr->mempool_stat[i].alloc_count,
							 data_ptr->mempool_stat[i].free_count,
							 data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count,
							 (data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count) *
							 data_ptr->mempool_stat[i].object_size);
		} else {// FORMAT_XML
			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%s    <mempool>\n"
							 "        <name>%s</name>\n"
							 "        <tag>%s</tag>\n"
							 "        <size>%d</size>\n"
							 "        <allocs>%d</allocs>\n"
							 "        <frees>%d</frees>\n"
							 "        <inuse>%d</inuse>\n"
							 "        <bytes>%d</bytes>\n"
							 "    </mempool>\n",
							 output,
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
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s\n"
						 "MemPoolObjFrees:  %u\n"
						 "MemPoolAllocs:    %u\n"
						 "MemPoolFrees:     %u\n"
						 "MemPoolInuse:     %u\n"
						 "MemPoolObjAllocs: %u\n"
						 "MemPoolObjFrees:  %u\n"
						 "MemPoolObjInuse:  %u\n\n",
						 output,
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolAllocs,
						 data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolAllocs - data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolObjAllocs,
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolObjAllocs - data_ptr->stats.memPoolObjFrees);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s  </mempools>\n", output);
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%sMemory Types:\n"
						 "Name                      Tag  Allocs       Frees        Inuse        \n"
						 "------------------------- ---- ------------ ------------ ------------ \n", output);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s  <memtypes>\n", output);
	}

	for (i = 0; i < ESIF_MEMTYPE_TYPE_MAX; i++) {
		char tag[8] = {0};
		esif_ccb_memcpy(tag, &data_ptr->memtype_stat[i].pool_tag, 4);
		if (0 == data_ptr->memtype_stat[i].pool_tag) {
			continue;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%-25s %s %-12u %-12u %-12u\n",
							 output,
							 data_ptr->memtype_stat[i].name,
							 "tag ",
							 data_ptr->memtype_stat[i].alloc_count,
							 data_ptr->memtype_stat[i].free_count,
							 data_ptr->memtype_stat[i].alloc_count -
							 data_ptr->memtype_stat[i].free_count);
		} else {// FORMAT_XML
			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%s    <memtype>\n"
							 "        <name>%s</name>\n"
							 "        <tag>%s</tag>\n"
							 "        <allocs>%d</allocs>\n"
							 "        <frees>%d</frees>\n"
							 "        <inuse>%d</inuse>\n"
							 "    </memtype>\n",
							 output,
							 data_ptr->memtype_stat[i].name,
							 tag,
							 data_ptr->memtype_stat[i].alloc_count,
							 data_ptr->memtype_stat[i].free_count,
							 data_ptr->memtype_stat[i].alloc_count - data_ptr->memtype_stat[i].free_count);
		}
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s\nMemTypeAllocs:    %u\n"
						 "MemTypeFrees:     %u\n"
						 "MemTypeInuse:     %u\n"
						 "MemTypeObjAllocs: %u\n"
						 "MemTypeObjFrees:  %u\n"
						 "MemTypeObjInuse:  %u\n\n",
						 output,
						 data_ptr->stats.memTypeAllocs,
						 data_ptr->stats.memTypeFrees,
						 data_ptr->stats.memTypeAllocs - data_ptr->stats.memTypeFrees,
						 data_ptr->stats.memTypeObjAllocs,
						 data_ptr->stats.memTypeObjFrees,
						 data_ptr->stats.memTypeObjAllocs - data_ptr->stats.memTypeObjFrees);

#ifdef ESIF_ATTR_MEMTRACE
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%sUser Memory Stats:\n\n"
						 "Allocs: %u\n"
						 "Frees : %u\n",
						 output,
						 atomic_read(&g_memtrace.allocs),
						 atomic_read(&g_memtrace.frees));
#endif
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s  </memtypes>\n"
						 "</memstats>\n",
						 output);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// No Log
static char*esif_shell_cmd_nolog (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	if (g_debuglog) {
		fclose(g_debuglog);
	}
	g_debuglog = 0;
	esif_ccb_sprintf(OUT_BUF_LEN, output, "nolog close log file\n");
	return output;
}


// Participant
static char*esif_shell_cmd_participantk (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	struct esif_command_get_participant_detail *data_ptr = NULL;
	struct esif_ipc_command *command_ptr = NULL;
	struct esif_ipc *ipc_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_participant_detail);
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
	*(u32*)(command_ptr + 1) = participant_id;
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
	data_ptr = (struct esif_command_get_participant_detail*)(command_ptr + 1);
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
						 esif_guid_print((esif_guid_t*)data_ptr->class_guid, guid_str),
						 data_ptr->flags,
						 esif_pm_participant_state_str((enum esif_pm_participant_state)data_ptr->state),
						 data_ptr->state,
						 data_ptr->timer_period);

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%sACPI Attributes\n"
						 "--------------------------------------------------------------------\n"
						 "Device:      %s\n"
						 "Scope:       %s\n"
						 "Unique ID:   0x%08x\n"
						 "Type:        0x%08x\n\n",
						 output,
						 data_ptr->acpi_device,
						 data_ptr->acpi_scope,
						 data_ptr->acpi_uid,
						 data_ptr->acpi_type);

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%sPCI Attributes\n"
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
						 output,
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
			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%sParticipant Capabilities: 0x%08x                                  \n"
							 "--------------------------------------------------------------------\n",
							 output,
							 data_ptr->capability);

			if (data_ptr->capability & ESIF_CAPABILITY_ACTIVE_CONTROL) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sACTIVE_CONTROL\t\tHas Fan Features\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_CTDP_CONTROL) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sCTDP_CONTROL\t\tHas CPU CTDP Controls\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_CORE_CONTROL) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sCORE_CONTROL\t\t\tHas CPU/Core Controls\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_CORE_CONTROL) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sCORE_CONTROL\t\t\tHas CPU/Core Controls\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_DISPLAY_CONTROL) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sDISPLAY_CONTROL\t\t\tHas Display Controls\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_PERF_CONTROL) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sPERF_CONTROL\t\tHas Performance Controls\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_POWER_CONTROL) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sPOWER_CONTROL\t\tHas RAPL Power Controls\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_POWER_STATUS) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sPOWER_STATUS\t\t\t\tHas RAPL Power Feature\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_TEMP_STATUS) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sTEMP_STATUS\t\tHas Temp Sensor\n", output);
			}

            if (data_ptr->capability & ESIF_CAPABILITY_TEMP_THRESHOLD) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sTEMP_THRESHOLD\t\tHas Temp Thresholds\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_UTIL_STATUS) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sUTIL_STATUS\t\t\tReports Device Utilization\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_PIXELCLOCK_CONTROL) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sPIXELCLOCK_CONTROL\t\t\tHas Pixel Clock Control\n", output);
			}

			if (data_ptr->capability & ESIF_CAPABILITY_PIXELCLOCK_STATUS) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sPIXELCLOCK_STATUS\t\t\tHas Pixel Clock Status\n", output);
			}

            if (data_ptr->capability & ESIF_CAPABILITY_RFPROFILE_STATUS) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sRFPROFILE_STATUS\t\t\tHas RF Profile Status\n", output);
			}

			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
		}

		if (1 == data_ptr->have_dsp) {
			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%sDevice Support Package(DSP):                  \n"
							 "------------------------------------------------\n"
							 "Code:         %s\n"
							 "Content Ver:  %d.%d\n"
							 "Type:         %s\n\n",
							 output,
							 data_ptr->dsp_code,
							 data_ptr->dsp_ver_major,
							 data_ptr->dsp_ver_minor,
							 "Compact Primitive Catalog (CPC)");
		}

		if (1 == data_ptr->have_cpc) {
			u8 *byte_ptr = (u8*)&data_ptr->cpc_signature;
			esif_ccb_sprintf(OUT_BUF_LEN, output,
							 "%sCompact Primitive Catalog(CPC):               \n"
							 "------------------------------------------------\n"
							 "Version:          %d\n"
							 "Signature:        %08X (%c%c%c%c)\n"
							 "Size:             %d\n"
							 "Primitive Count:  %d\n\n",
							 output,
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
			u8 *ptr = (u8*)&data_ptr->cpc_signature;
			esif_ccb_sprintf(10, sig_str, "%c%c%c%c", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3));
		}

		if (data_ptr->capability & ESIF_CAPABILITY_TEMP_STATUS) {
			esif_ccb_sprintf(capability_str_len, capability_str, "%sTEMP_SENSOR,", capability_str);
		}
		if (data_ptr->capability & ESIF_CAPABILITY_ACTIVE_CONTROL) {
			esif_ccb_sprintf(capability_str_len, capability_str, "%sCOOLING_DEVICE,", capability_str);
		}
		if (data_ptr->capability & ESIF_CAPABILITY_POWER_STATUS) {
			esif_ccb_sprintf(capability_str_len, capability_str, "%sRAPL_DEVICE", capability_str);
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
						 esif_guid_print((esif_guid_t*)data_ptr->class_guid, guid_str),
						 data_ptr->version,
						 data_ptr->state,
						 esif_pm_participant_state_str((enum esif_pm_participant_state)data_ptr->state),
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


// 6715
static char*esif_shell_cmd_participant (EsifShellCmdPtr shell)
{
	int argc         = shell->argc;
	char * *argv     = shell->argv;
	char *output     = shell->outbuf;
	UInt8 participant_id = 0;
	EsifUpPtr up_ptr = NULL;

	if (argc < 2) {
		return NULL;
	}

	participant_id = (UInt8) esif_atoi(argv[1]);

	up_ptr = EsifUpManagerGetAvailableParticipantByInstance(participant_id);
	if (NULL == up_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: participant not available\n", ESIF_FUNC);
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
						 "Flags:             0x%08x\n"
						 "Status:            %s(%d)\n",
						 participant_id,
						 up_ptr->fMetadata.fVersion,
						 up_ptr->fMetadata.fEnumerator,
						 up_ptr->fMetadata.fName,
						 up_ptr->fMetadata.fDesc,
						 up_ptr->fMetadata.fDriverName,
						 up_ptr->fMetadata.fDeviceName,
						 up_ptr->fMetadata.fDevicePath,
						 esif_guid_print((esif_guid_t*)up_ptr->fMetadata.fDriverType, guid_str),
						 up_ptr->fMetadata.fFlags,
						 esif_pm_participant_state_str((enum esif_pm_participant_state)1),
						 0);

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s\n"
						 "ACPI Attributes\n"
						 "--------------------------------------------------------------------\n"
						 "Device:      %s\n"
						 "Scope:       %s\n"
						 "Unique ID:   0x%08x\n"
						 "Type:        0x%08x\n",
						 output,
						 up_ptr->fMetadata.fAcpiDevice,
						 up_ptr->fMetadata.fAcpiScope,
						 up_ptr->fMetadata.fAcpiUID,
						 up_ptr->fMetadata.fAcpiType);

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s\n"
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
						 output,
						 up_ptr->fMetadata.fPciVendor,
						 up_ptr->fMetadata.fPciDevice,
						 up_ptr->fMetadata.fPciBus,
						 up_ptr->fMetadata.fPciBusDevice,
						 up_ptr->fMetadata.fPciFunction,
						 up_ptr->fMetadata.fPciRevision,
						 up_ptr->fMetadata.fPciClass,
						 up_ptr->fMetadata.fPciSubClass,
						 up_ptr->fMetadata.fPciProgIf);
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
						 "  <acpiUID>0x%08x</acpiUID>\n"
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
						// "  <dspCode>%s</dspCode>\n"
						// "  <dspContentVersion>%d.%d</dspContentVersion>\n"
						// "  <dspSlope>%d</dspSlope>\n"
						// "  <dspOffset>%d</dspOffset>\n"
						// "  <dspTJMax>%d</dspTJMax>\n"
						// "  <dspType>%s</dspType>\n"
						 "</participant>\n",
						 participant_id,
						 up_ptr->fMetadata.fEnumerator,
						 up_ptr->fMetadata.fName,
						 up_ptr->fMetadata.fDesc,
						 esif_guid_print((esif_guid_t*)up_ptr->fMetadata.fDriverType, guid_str),
						 up_ptr->fMetadata.fVersion,
						 1,
						 esif_pm_participant_state_str((enum esif_pm_participant_state)1),
						 up_ptr->fMetadata.fAcpiDevice,
						 up_ptr->fMetadata.fAcpiScope,
						 up_ptr->fMetadata.fAcpiUID,
						 up_ptr->fMetadata.fAcpiType,
						 up_ptr->fMetadata.fPciVendor,
						 up_ptr->fMetadata.fPciDevice,
						 up_ptr->fMetadata.fPciBus,
						 up_ptr->fMetadata.fPciBusDevice,
						 up_ptr->fMetadata.fPciFunction,
						 up_ptr->fMetadata.fPciRevision,
						 up_ptr->fMetadata.fPciClass,
						 up_ptr->fMetadata.fPciSubClass,
						 up_ptr->fMetadata.fPciProgIf);
		// up_ptr->fDspPtr->get_code(up_ptr->fDspPtr),
		// up_ptr->fDspPtr->get_ver_major(up_ptr->fDspPtr),
		// up_ptr->fDspPtr->get_ver_minor(up_ptr->fDspPtr),
		// up_ptr->fDspPtr->get_slope(up_ptr->fDspPtr),
		// up_ptr->fDspPtr->get_offset(up_ptr->fDspPtr),
		// up_ptr->fDspPtr->get_tjmax(up_ptr->fDspPtr),
		// "Full Primitive Catalog");
	}
exit:
	return output;
}


// RemARK
static char*esif_shell_cmd_rem (EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	// Do Nothing
	return NULL;
}


// Repeat
/*static*/ char*esif_shell_cmd_repeat (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_repeat = esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "repeat next command count=%d times\n", g_repeat);
	return output;
}


// Repeat Delay
/*static*/ char*esif_shell_cmd_repeatdelay (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_repeat_delay = esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "repeat delay = %d msec between each repeated command\n", g_repeat_delay);
	return output;
}


// Set Buffer
static char*esif_shell_cmd_setb (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_binary_buf_size = esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "binary_buf_size=%d\n", g_binary_buf_size);
	return output;
}


// Set Error level
static char*esif_shell_cmd_seterrorlevel (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_errorlevel = esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "seterrorlevel = %d\n", g_errorlevel);
	return output;
}


// Set Primitive
static char*esif_shell_cmd_setp (EsifShellCmdPtr shell)
{
	int argc            = shell->argc;
	char * *argv        = shell->argv;
	char *output        = shell->outbuf;
	enum esif_rc rc     = ESIF_OK;
	EsifString desc     = ESIF_NOT_AVAILABLE;
	u32 id = 0;
	u16 qualifier       = 0;
	char *qualifier_str = 0;
	u8 instance         = 0;
	u8 *data_ptr        = NULL;
	char *suffix        = 0;
	enum esif_data_type type = ESIF_DATA_VOID;
	int opt = 1;
	u32 data_len = 0;
	struct esif_data request;
	struct esif_data response = {ESIF_DATA_VOID, NULL, 0};
	u32 data, add_data;

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
	if (!suffix) {
		return NULL;
	}

	// Primitive ID
	id = esif_atoi(argv[opt++]);

	// Qualifier ID
	qualifier_str = argv[opt++];
	qualifier     = convert_string_to_short(qualifier_str);

	// Instance ID
	instance = (u8)esif_atoi(argv[opt++]);

	// Data Integer Only For Now TODO Complex Data
	data = esif_atoi(argv[opt++]);


	// Optional, Additional Argument, Integer Only For Now
	if (opt >= argc) {
		data_len = sizeof(ESIF_DATA_UINT32);
		data_ptr = (u8*)esif_ccb_malloc(data_len);
		if (NULL == data_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ccb_malloc failed for %u bytes\n",
							 ESIF_FUNC, data_len);
			goto exit;
		}

		// Command
		esif_ccb_memcpy((data_ptr), &data, sizeof(data));
	} else {
		data_len = sizeof(ESIF_DATA_UINT32) * 2;
		data_ptr = (u8*)esif_ccb_malloc(data_len);
		if (NULL == data_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ccb_malloc failed for %u bytes\n",
							 ESIF_FUNC, data_len);
			goto exit;
		}

		add_data = esif_atoi(argv[opt++]);

		// Command, req_data = {data, add_data}
		esif_ccb_memcpy((data_ptr), &data, sizeof(data));
		esif_ccb_memcpy((data_ptr + sizeof(data)), &add_data, sizeof(add_data));
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s ", esif_primitive_str((enum esif_primitive_type)id));

	request.type     = type;
	request.buf_len  = data_len;
	request.buf_ptr  = (void*)data_ptr;
	request.data_len = data_len;

	rc = EsifExecutePrimitive(
			(u8)g_dst,
			id,
			qualifier_str,
			instance,
			&request,
			&response);


	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s: setp%s %03u.%s.%03d error code = %s(%d)\n",
						 output, suffix, ESIF_FUNC, id, qualifier_str, instance,
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
		desc = "PERCENT";
		break;

	default:
		desc = "";
		break;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s setp%s(%03u,%s,%03d) value = 0x%08x %u %s\n", esif_primitive_str(
						 (enum esif_primitive_type)id), suffix, id, qualifier_str, instance, data, data, desc);

exit:

	if (NULL != data_ptr) {
		esif_ccb_free(data_ptr);
	}
	return output;
}


static char*esif_shell_cmd_set_osc (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	struct esif_data_complex_osc osc = {0};
	struct esif_data_complex_osc *ret_osc;
	const u32 data_len = sizeof(struct esif_data_complex_osc);
	u8 *data_ptr = NULL;
	u32 id;
	struct esif_data request;
	struct esif_data response;
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

	data_ptr = (u8*)esif_ccb_malloc(data_len);
	if (NULL == data_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ccb_malloc failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	request.type      = ESIF_DATA_STRUCTURE;
	request.buf_len   = data_len;
	request.buf_ptr   = (u8*)data_ptr;
	request.data_len  = data_len;

	response.type     = ESIF_DATA_STRUCTURE;
	response.buf_len  = data_len;
	response.buf_ptr  = (u8*)data_ptr;
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
	ret_osc = (struct esif_data_complex_osc*)(response.buf_ptr);

	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s: set_osc error code = %s(%d), status code = %d\n",
						 output, ESIF_FUNC,
						 esif_rc_str(rc), rc,
						 ret_osc->status);
		g_errorlevel = 6;
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s: set_osc returned status %u\n",
					 output, ESIF_FUNC, ret_osc->status);

exit:

	if (NULL != data_ptr) {
		esif_ccb_free(data_ptr);
	}
	return output;
}


static char*esif_shell_cmd_set_scp (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;

	struct esif_data_complex_scp scp = {0};
	const u32 data_len = sizeof(struct esif_data_complex_scp);
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

	data_ptr = (u8*)esif_ccb_malloc(data_len);
	if (NULL == data_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
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
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s: set_scp error code = %s(%d)\n",
						 output, ESIF_FUNC,
						 esif_rc_str(rc), rc);
		g_errorlevel = 6;
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "set_scp cooling mode %u, power limit %u, acoustic limit %u\n",
					 scp.cooling_mode, scp.power_limit, scp.acoustic_limit);
exit:

	if (NULL != data_ptr) {
		esif_ccb_free(data_ptr);
	}
	return output;
}


// Test
static char*esif_shell_cmd_test (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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
		data_ptr = (struct esif_command_get_participants*)(command_ptr + 1);

		if (start_id != stop_id - 1) {
			stop_id = data_ptr->count;
		}

		g_errorlevel = 0;
		// Loop thorugh from start to finish participants


		for (i = start_id; i < stop_id; i++) {
				#define COMMAND_LEN (32 + 1)
			char command[COMMAND_LEN];
			if (data_ptr->participant_info[i].state == ESIF_PM_PARTICIPANT_STATE_REGISTERED) {
				esif_ccb_sprintf(COMMAND_LEN, command, "dst %d", i);
				parse_cmd(command, ESIF_FALSE);

				esif_ccb_sprintf(COMMAND_LEN, command, "loadtst %s.tst", data_ptr->participant_info[i].dsp_code);
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%sRun Test: %s DUT %d\n", output, command, g_dst);
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
static char*esif_shell_cmd_timestamp (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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
static char*esif_shell_cmd_debugshow (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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
	data_ptr = (struct esif_command_get_debug_module_level*)(command_ptr + 1);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "Modules = 0x%08X\n", data_ptr->modules);

	// Once for each BIT
	for (i = 0; i < 32; i++) {
		u32 bit = 1;
		bit = bit << i;
		if (data_ptr->modules & 0x1) {
			state = (char*)"ENABLED";
		} else {
			state = (char*)"DISABLED";
		}

		data_ptr->modules = data_ptr->modules >> 1;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%sModule: %3s(%02u) Bit: 0x%08X State: %8s Level: 0x%08X\n",
						 output, esif_debug_mod_str((enum esif_debug_mod)i), i, bit, state, data_ptr->levels[i]);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// Apps
static char*esif_shell_cmd_about (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (g_format == FORMAT_TEXT) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\n"
						 "ESIF - Eco-System Independent Framework\n"
						 "(c) 2012-2013 Intel Corporation. All Rights Reserved.\n"
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
						 "3  erapi       ESIF Rest API             %s\n"
						 "\n"
						 "esif_lf - ESIF Lower Framework (LF) R0\n"
						 "ESIF Kernel Driver Information:\n"
						 "Version: %s\n"
						 "\n",
						 ESIF_UF_VERSION,
						 g_os,
						 g_esif_shell_version,
						 g_esif_etf_version,
						 g_esif_erapi_version,
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
						 "  <restVersion>%s</restVersion>\n"
						 "</about>\n",
						 ESIF_UF_VERSION,
						 g_esif_kernel_version,
						 g_os,
						 g_esif_shell_version,
						 g_esif_etf_version,
						 "g_esif_erapi_version");
	}

	return output;
}


// Exit
static char*esif_shell_cmd_exit (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	g_quit = 1;
	esif_ccb_sprintf(OUT_BUF_LEN, output, "exit\n");
	return output;
}


// Get Buffer Size
static char*esif_shell_cmd_getb (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "binary_buf_size=%d\n", g_binary_buf_size);
	return output;
}


// Get Error Level
static char*esif_shell_cmd_geterrorlevel (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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
static char*esif_shell_cmd_info (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	const u32 data_len = sizeof(struct esif_command_get_kernel_info);
	struct esif_ipc_command *command_ptr = NULL;
	struct esif_command_get_kernel_info *data_ptr = NULL;
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

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
	data_ptr = (struct esif_command_get_kernel_info*)(command_ptr + 1);

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Kernel Version = %s\n", data_ptr->ver_str);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<info>\n"
						 "  <kernelVersion>%s</kernelVersion>\n"
						 "</info>\n", data_ptr->ver_str);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// Backwards compatibility for calling from ipc_connect()
char*esif_cmd_info (char *output)
{
	char *argv[] = {"info", 0};
	EsifShellCmd shell;
	shell.argc   = sizeof(argv) / sizeof(char*);
	shell.argv   = argv;
	shell.outbuf = output;
	return esif_shell_cmd_info(&shell);
}


// IPC Mode
static char*esif_shell_cmd_ipcmode (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}
	g_ipcmode = esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "ipcmode=%d\n", g_ipcmode);
	return output;
}


// Participants
static char*esif_shell_cmd_participantsk (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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
	data_ptr = (struct esif_command_get_participants*)(command_ptr + 1);
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
		char *enum_str  = (char*)"NA";
		char *state_str = esif_pm_participant_state_str((enum esif_pm_participant_state)data_ptr->participant_info[i].state);

		if (data_ptr->participant_info[i].state >= ESIF_PM_PARTICIPANT_STATE_CREATED) {
			switch (data_ptr->participant_info[i].bus_enum) {
			case 0:
				enum_str = (char*)"ACPI";
				break;

			case 1:
				enum_str = (char*)"PCI";
				break;

			case 2:
				enum_str = (char*)"PLAT";
				break;

			case 3:
				enum_str = (char*)"CNJR";
				break;
			}
		}

		if (FORMAT_TEXT == g_format) {
			if (data_ptr->participant_info[i].state > ESIF_PM_PARTICIPANT_REMOVED) {
				esif_ccb_sprintf(OUT_BUF_LEN, output,
								 "%s  %-2d %-8s %-31s %4s  %d  %-12s",
								 output,
								 data_ptr->participant_info[i].id,
								 data_ptr->participant_info[i].name,
								 data_ptr->participant_info[i].desc,
								 enum_str,
								 data_ptr->participant_info[i].version,
								 state_str);

				if (data_ptr->participant_info[i].state == ESIF_PM_PARTICIPANT_STATE_REGISTERED) {
					esif_ccb_sprintf(OUT_BUF_LEN, output,
									 "%s %s(%d.%d)\n",
									 output,
									 data_ptr->participant_info[i].dsp_code,
									 data_ptr->participant_info[i].dsp_ver_major,
									 data_ptr->participant_info[i].dsp_ver_minor);
				} else {
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
				}
			}
		} else {// FORMAT_XML
			if (data_ptr->participant_info[i].state > ESIF_PM_PARTICIPANT_REMOVED) {
				esif_ccb_sprintf(OUT_BUF_LEN, output,
								 "%s<participant>\n"
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
								 output,
								 data_ptr->participant_info[i].id,
								 data_ptr->participant_info[i].name,
								 data_ptr->participant_info[i].desc,
								 data_ptr->participant_info[i].bus_enum,
								 enum_str,
								 data_ptr->participant_info[i].version,
								 data_ptr->participant_info[i].state,
								 state_str,
								 data_ptr->participant_info[i].dsp_code,
								 data_ptr->participant_info[i].dsp_ver_major,
								 data_ptr->participant_info[i].dsp_ver_minor);
			}
		}
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s</participants>\n", output);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// Participants
static char*esif_shell_cmd_participants (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	u8 i = 0;
	char *attribute = "";
	u8 domain_index = 0;

	// Qualifier
	if (argc > 1) {
		attribute = argv[1];
	}

	if (!strcmp(attribute, "acpi")) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nALL PARTICIPANTS: Count: %d ACPI CONFIGURATION\n\n"
						 "UPID LPID Name     Description  Enum Ver HID      SCOPE                          UID Type\n"
						 "---- ---- -------- ------------ ---- --- -------- ------------------------------ --- ----\n",
						 g_uppMgr.fEntryCount);
	} else if (!strcmp(attribute, "pci")) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "pci data\n");
	} else {
		if (g_format == FORMAT_XML) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "<result>\n");

			for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
				char desc[ESIF_DESC_LEN + 3] = "";	/* desc... */
				char *enum_str = "NA";
				char instance_str[8 + 1]     = "NA";

				u8 domainCount   = 1;
				EsifUpPtr up_ptr = EsifUpManagerGetAvailableParticipantByInstance(i);
				if (NULL == up_ptr || NULL == up_ptr->fDspPtr) {
					continue;
				}
				domainCount = (u8)up_ptr->fDspPtr->get_domain_count(up_ptr->fDspPtr);

				/* Truncate Description If Large */
				if (ESIF_SHELL_STRLEN(up_ptr->fMetadata.fDesc) > 12) {
					esif_ccb_strcpy(desc, up_ptr->fMetadata.fDesc, 9);
					esif_ccb_sprintf(ESIF_DESC_LEN, desc, "%s...", desc);
				} else {
					esif_ccb_strcpy(desc, up_ptr->fMetadata.fDesc, 12);
				}

				switch (up_ptr->fMetadata.fEnumerator) {
				case ESIF_PARTICIPANT_ENUM_ACPI:
					enum_str = (char*)"ACPI";
					break;

				case ESIF_PARTICIPANT_ENUM_PCI:
					enum_str = (char*)"PCI";
					break;

				case ESIF_PARTICIPANT_ENUM_PLAT:
					enum_str = (char*)"PLAT";
					break;

				case ESIF_PARTICIPANT_ENUM_CONJURE:
					enum_str = (char*)"CNJR";
					break;
				}

				if (up_ptr->fLpInstance != 255) {
					esif_ccb_sprintf(8, instance_str, "%-2d", up_ptr->fLpInstance);
				}

				esif_ccb_sprintf(OUT_BUF_LEN, output,
								 "%s  <participant>\n"
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
								 "    <acpiUID>%u</acpiUID>\n"
								 "    <acpiScope>%s</acpiScope>\n"
								 "    <acpiType>%u</acpiType>\n"
								 "    <domainCount>%u</domainCount>\n",
								 output,
								 up_ptr->fInstance,
								 instance_str,
								 up_ptr->fMetadata.fName,
								 up_ptr->fMetadata.fDesc,
								 desc,
								 up_ptr->fMetadata.fEnumerator,
								 enum_str,
								 up_ptr->fMetadata.fVersion,
								 up_ptr->fDspPtr->code_ptr,
								 up_ptr->fMetadata.fAcpiDevice,
								 up_ptr->fMetadata.fAcpiUID,
								 up_ptr->fMetadata.fAcpiScope,
								 up_ptr->fMetadata.fAcpiType,
								 domainCount);

				esif_ccb_sprintf(OUT_BUF_LEN, output,
								 "%s    <domains>\n",
								 output);

				/* Domain Kludge Here */
				for (domain_index = 0; domain_index < domainCount; domain_index++) {
					struct esif_fpc_domain *domain_ptr = up_ptr->fDspPtr->get_domain(up_ptr->fDspPtr, domain_index + 1);
					if (domain_ptr) {
						char guid_buf[ESIF_GUID_PRINT_SIZE];
						esif_ccb_sprintf(OUT_BUF_LEN, output,
										 "%s"
										 "      <domain>\n"
										 "        <id>%d</id>\n"
										 "        <version>1</version>\n"
										 "        <name>%s</name>\n"
										 "        <desc>%s</desc>\n"
										 "        <guid>%s</guid>\n"
										 "        <type>%d</type>\n"
										 "        <capability>0x%x</capability>\n"
										 "      </domain>\n",
										 output, domain_index, domain_ptr->descriptor.name,
										 domain_ptr->descriptor.description,
										 esif_guid_print((esif_guid_t*)domain_ptr->descriptor.guid, guid_buf),
										 domain_ptr->descriptor.domainType,
										 domain_ptr->capability_for_domain.capability_flags);
					} else {
						break;
					}
				}

				esif_ccb_sprintf(OUT_BUF_LEN, output,
								 "%s"
								 "    </domains>\n"
								 "  </participant>\n",
								 output);
			}

			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s</result>\n", output);
			return output;
		}


		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nALL PARTICIPANTS: Count: %d\n\n"
						 "UPID LPID Name     Description  Enum Ver Active DSP        DC\n"
						 "---- ---- -------- ------------ ---- --- ----------------- --\n",
						 g_uppMgr.fEntryCount);
	}

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		char desc[ESIF_DESC_LEN + 3] = "";	/* desc... */
		char *enum_str = "NA";
		char instance_str[8 + 1]     = "NA";

		u8 domainCount   = 1;
		EsifUpPtr up_ptr = EsifUpManagerGetAvailableParticipantByInstance(i);
		if (NULL == up_ptr || NULL == up_ptr->fDspPtr) {
			continue;
		}
		domainCount = (u8)up_ptr->fDspPtr->get_domain_count(up_ptr->fDspPtr);

		/* Truncate Description If Large */
		if (ESIF_SHELL_STRLEN(up_ptr->fMetadata.fDesc) > 12) {
			esif_ccb_strcpy(desc, up_ptr->fMetadata.fDesc, 9);
			esif_ccb_sprintf(ESIF_DESC_LEN, desc, "%s...", desc);
		} else {
			esif_ccb_strcpy(desc, up_ptr->fMetadata.fDesc, 12);
		}

		switch (up_ptr->fMetadata.fEnumerator) {
		case ESIF_PARTICIPANT_ENUM_ACPI:
			enum_str = (char*)"ACPI";
			break;

		case ESIF_PARTICIPANT_ENUM_PCI:
			enum_str = (char*)"PCI";
			break;

		case ESIF_PARTICIPANT_ENUM_PLAT:
			enum_str = (char*)"PLAT";
			break;

		case ESIF_PARTICIPANT_ENUM_CONJURE:
			enum_str = (char*)"CNJR";
			break;
		}

		if (up_ptr->fLpInstance != 255) {
			esif_ccb_sprintf(8, instance_str, "%-2d", up_ptr->fLpInstance);
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s %-2d   %s  %-8s %-12s %-4s  %d  ", output,
						 up_ptr->fInstance,
						 instance_str,
						 up_ptr->fMetadata.fName,
						 desc,
						 enum_str,
						 up_ptr->fMetadata.fVersion);


		if (!strcmp(attribute, "acpi")) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%-8s %-30s %-3d %-3d\n", output,
							 up_ptr->fMetadata.fAcpiDevice,
							 up_ptr->fMetadata.fAcpiScope,
							 up_ptr->fMetadata.fAcpiUID,
							 up_ptr->fMetadata.fAcpiType);
		} else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%-17s %-2d\n", output,
							 up_ptr->fDspPtr->code_ptr,
							 domainCount);
		}
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);


	return output;
}


// Quit
static char*esif_shell_cmd_quit (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	g_quit = 1;
	esif_ccb_sprintf(OUT_BUF_LEN, output, "quit\n");
	return output;
}


// Timer Start
static char*esif_shell_cmd_timerstart (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	esif_ccb_get_time(&g_timer);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "timerstart\n");
	return output;
}


// Timer Stop
static char*esif_shell_cmd_timerstop (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
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
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%sStart time: %06lu.%06lu\n", output,
					 g_timer.tv_sec,
					 g_timer.tv_usec);

	esif_ccb_sprintf(OUT_BUF_LEN, output, "%sStop time: %06lu.%06lu\n", output,
					 stop.tv_sec,
					 stop.tv_usec);

	timeval_subtract(&result, &stop, &g_timer);

	esif_ccb_sprintf(OUT_BUF_LEN, output, "%sTime: %06lu.%06lu (%lu seconds + %lu msec + %lu usec)\n", output,
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


static char*esif_shell_cmd_unloadcpc (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	u32 cpc_size = 0;	/* Indicates Unload */
	struct esif_ipc_primitive *primitive_ptr = NULL;
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_primitive(&primitive_ptr, cpc_size);

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	if (NULL == ipc_ptr || NULL == primitive_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, cpc_size);
		goto exit;
	}

	primitive_ptr->id       = 200;
	primitive_ptr->domain   = 0;
	primitive_ptr->instance = 0;
	primitive_ptr->src_id   = ESIF_INSTANCE_UF;
	primitive_ptr->dst_id   = (u8)g_dst;
	primitive_ptr->req_data_type   = ESIF_DATA_DSP;
	primitive_ptr->req_data_offset = 0;
	primitive_ptr->req_data_len    = cpc_size;
	primitive_ptr->rsp_data_type   = ESIF_DATA_VOID;
	primitive_ptr->rsp_data_offset = 0;
	primitive_ptr->rsp_data_len    = 0;

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != primitive_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: primitive error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(primitive_ptr->return_code), primitive_ptr->return_code);
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "cpc unload complete for dst = %d\n", g_dst);
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// Switch System between DPTF/ESIF
void cmd_app_subsystem (enum app_subsystem subsystem)
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
static char*esif_shell_cmd_conjure (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *lib_name;
	EsifCnjPtr a_conjure_ptr = NULL;
	u8 i = 0;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}

	lib_name = argv[1];

	/* Check to see if the participant is already loaded only one instance per particpipant lib allowed */
	if (NULL != esif_uf_conjure_get_instance_from_name(lib_name)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Conjure Library %s Already Started.\n", lib_name);
		goto exit;
	}

	for (i = 0; i < ESIF_MAX_CONJURES; i++)
		if (NULL == g_cnjMgr.fEnrtries[i].fLibNamePtr) {
			break;
		}


	if (ESIF_MAX_CONJURES == i) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Maximum Conjures Reached %u.\n", i);
		goto exit;
	} else {
		g_cnjMgr.fEntryCount++;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Conjure Sandbox %u Selected.\n", i);
	}

	a_conjure_ptr = &g_cnjMgr.fEnrtries[i];
	a_conjure_ptr->fLibNamePtr = (esif_string)esif_ccb_strdup(lib_name);

	rc = EsifConjureStart(a_conjure_ptr);
	if (ESIF_OK != rc) {
		/* Cleanup */
		esif_ccb_free(a_conjure_ptr->fLibNamePtr);
		memset(a_conjure_ptr, 0, sizeof(*a_conjure_ptr));
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Start Conjure Library: %s.\n", lib_name);
	} else {
		/* Proceed */
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%sStarted Conjure Library: %s Instance %u Max %u Running %u\n\n",
						 output, lib_name, i, ESIF_MAX_CONJURES, g_cnjMgr.fEntryCount);
	}

exit:
	return output;
}


static char*esif_shell_cmd_unconjure (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *lib_name  = 0;
	EsifCnjPtr a_conjure_ptr = NULL;

	if (argc < 2) {
		return NULL;
	}

	/* Parse The Library Name */
	lib_name = argv[1];

	a_conjure_ptr = esif_uf_conjure_get_instance_from_name(lib_name);
	if (NULL == a_conjure_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find Conjure Library: %s.\n", lib_name);
		goto exit;
	}

	rc = EsifConjureStop(a_conjure_ptr);
	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Stop Conjure Library: %s.\n", lib_name);
	} else {
		/* Proceed */
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%sStopped Conjure Library: %s\n\n",
						 output, lib_name);
		esif_ccb_free(a_conjure_ptr->fLibNamePtr);
		a_conjure_ptr->fLibNamePtr = NULL;
		g_cnjMgr.fEntryCount--;
	}

exit:

	return output;
}


// Rest Start
static char*esif_shell_cmd_reststart (EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char * *argv    = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	rc = EsifRestStart();
	if (ESIF_OK == rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "reststart completed\n");
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "reststart failed %s(%d)\n", esif_rc_str(rc), rc);
	}
	return output;
}


// Rest Stop
static char*esif_shell_cmd_reststop (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "reststop\n");
	EsifRestStop();
	return output;
}


// Rest Event
static char*esif_shell_cmd_restevent (EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char * *argv   = shell->argv;
	char *output   = shell->outbuf;
	UInt8 event_id = 0;

	if (argc < 2) {
		return NULL;
	}

	event_id = (u8)esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "send rest event = %d\n", event_id);
	return output;
}


////////////////////////////////////////////////////////

// From Regenerated autogen.h
static struct esif_data_type_map_t {
	enum esif_data_type type;
	esif_string name;
}

esif_data_type_map[] = {
	ENUM_ESIF_DATA_TYPE(ENUMLIST)
};
static const UInt32 esif_data_type_count = sizeof(esif_data_type_map) / sizeof(esif_data_type_map[0]);

// From Regenerated autogen.h
static struct esif_primitive_type_map_t {
	enum esif_primitive_type type;
	esif_string name;
}

esif_primitive_type_map[] = {
	ENUM_ESIF_PRIMITIVE_TYPE(ENUMLIST)
};
static const UInt32 esif_primitive_type_count = sizeof(esif_primitive_type_map) / sizeof(esif_primitive_type_map[0]);

// TODO: Sort map alphabetically and do a Binary Search
enum esif_data_type esif_data_type_string2enum (esif_string name)
{
	UInt32 j;
	for (j = 0; j < esif_data_type_count; j++)
		if (esif_ccb_stricmp(esif_data_type_map[j].name, name) == 0 || (esif_ccb_stricmp(esif_data_type_map[j].name + 10, name) == 0)) {
			return esif_data_type_map[j].type;
		}

	return ESIF_DATA_VOID;
}


// TODO: Sort map alphabetically and do a Binary Search
enum esif_primitive_type esif_primitive_type_string2enum (esif_string name)
{
	UInt32 j;
	for (j = 0; j < esif_primitive_type_count; j++)
		if (esif_ccb_stricmp(esif_primitive_type_map[j].name, name) == 0) {
			return esif_primitive_type_map[j].type;
		}

	return (enum esif_primitive_type)0;	// Warning: Unpredictable Results
}


#define ishexdigit(ch)  (((ch) >= '0' && (ch) <= '9') || (isalpha(ch) && toupper(ch) >= 'A' && toupper(ch) <= 'F'))
#define tohexdigit(ch)  ((ch) >= '0' && (ch) <= '9' ? ((ch) - '0') : (toupper(ch) - 'A' + 10))

struct esif_data_binary_fst_package {
	union esif_data_variant revision;
	union esif_data_variant control;
	union esif_data_variant speed;
};

struct esif_data_binary_bst_package {
	union esif_data_variant battery_state;
	union esif_data_variant battery_present_rate;
	union esif_data_variant battery_remaining_capacity;
	union esif_data_variant battery_present_voltage;
};

static char*esif_shell_cmd_status (EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char * *argv  = shell->argv;
	char *output  = shell->outbuf;
	eEsifError rc = ESIF_OK;
	UInt8 i, j = 0;

	char qualifier_str[32] = "D0";

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
// TODO JDH Sync UI To use one cannonical way this is a workaround for now.
#ifdef  ESIF_ATTR_WEBSOCKET
	esif_ccb_sprintf(OUT_BUF_LEN, output, "update:<status>\n");
#else
	esif_ccb_sprintf(OUT_BUF_LEN, output, "<status>\n");
#endif

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		u8 domain_count  = 1;
		struct esif_fpc_domain *domain_ptr = NULL;
		EsifUpPtr up_ptr = EsifUpManagerGetAvailableParticipantByInstance(i);
		if (NULL == up_ptr) {
			continue;
		}
		domain_count = (u8)up_ptr->fDspPtr->get_domain_count(up_ptr->fDspPtr);

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s"
						 "<stat>\n"
						 "  <id>%d</id>\n"
						 "  <name>%s</name>\n",
						 output,
						 i,
						 up_ptr->fMetadata.fName);

		for (j = 0; j < domain_count; j++) {
			domain_ptr = up_ptr->fDspPtr->get_domain(up_ptr->fDspPtr, j + 1);
			if (NULL == domain_ptr) {
				continue;
			}


			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s"
												  "  <domain>\n"
												  "    <name>%s</name>\n",
							 output,
							 domain_ptr->descriptor.name);

			if (up_ptr->fMetadata.fFlags & 0x1) {	// DPTF Zone / IETM Only
#ifdef ESIF_ATTR_OS_WINDOWS
				SYSTEM_POWER_STATUS ps = {0};
				GetSystemPowerStatus(&ps);

				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s"
													  "    <battery>%d</battery>\n",
								 output,
								 ps.BatteryLifePercent);
#endif
			}

			if (domain_ptr->capability_for_domain.capability_flags & ESIF_CAPABILITY_TEMP_STATUS) {
				UInt32 temp = 255;
				UInt32 aux0 = 255;
				UInt32 aux1 = 255;
				UInt32 hyst = 0;

				struct esif_data request = {ESIF_DATA_VOID, NULL, 0, 0};
				struct esif_data temp_response = {ESIF_DATA_TEMPERATURE, &temp, sizeof(temp), 4};
				struct esif_data aux0_response = {ESIF_DATA_TEMPERATURE, &aux0, sizeof(aux0), 4};
				struct esif_data aux1_response = {ESIF_DATA_TEMPERATURE, &aux1, sizeof(aux1), 4};
				struct esif_data hyst_response = {ESIF_DATA_UINT32, &hyst, sizeof(hyst), 4};/* Hysteresis is NOT Temp unitless */

				rc = EsifExecutePrimitive(i, GET_TEMPERATURE, esif_primitive_domain_str(domain_ptr->descriptor.domain,
																						qualifier_str,
																						32), 255, &request, &temp_response);
				rc = EsifExecutePrimitive(i, GET_TEMPERATURE_THRESHOLDS, esif_primitive_domain_str(domain_ptr->descriptor.domain,
																								   qualifier_str,
																								   32), 0, &request, &aux0_response);
				rc = EsifExecutePrimitive(i, GET_TEMPERATURE_THRESHOLDS, esif_primitive_domain_str(domain_ptr->descriptor.domain,
																								   qualifier_str,
																								   32), 1, &request, &aux1_response);
				rc = EsifExecutePrimitive(i, GET_TEMPERATURE_THRESHOLD_HYSTERESIS, esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifier_str, 32),
										  255, &request, &hyst_response);


				esif_ccb_sprintf(OUT_BUF_LEN, output,
								 "%s"
								 "    <temp>%d</temp>\n"
								 "    <tempAux0>%d</tempAux0>\n"
								 "    <tempAux1>%d</tempAux1>\n"
								 "    <tempHyst>%d</tempHyst>\n",
								 output,
								 temp,
								 aux0,
								 aux1,
								 hyst);
			}

			if (domain_ptr->capability_for_domain.capability_flags & ESIF_CAPABILITY_ACTIVE_CONTROL) {
				UInt32 fan_speed = 0;

				struct esif_data request  = {ESIF_DATA_VOID, NULL, 0, 0};
				struct esif_data response = {ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0};

				rc = EsifExecutePrimitive(i, GET_FAN_STATUS, esif_primitive_domain_str(domain_ptr->descriptor.domain,
																					   qualifier_str,
																					   32), 255, &request, &response);

				if (ESIF_OK == rc && response.buf_ptr) {
					struct esif_data_binary_fst_package *fst_ptr =
						(struct esif_data_binary_fst_package*)response.buf_ptr;

					fan_speed = (UInt32)fst_ptr->speed.integer.value;
				}
				esif_ccb_free(response.buf_ptr);

				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s"
													  "    <fanSpeed>%d</fanSpeed>\n",
								 output,
								 fan_speed);
			}

			if (domain_ptr->capability_for_domain.capability_flags & ESIF_CAPABILITY_BATTERY_STATUS) {
				UInt32 present_state      = 0;
				UInt32 present_rate       = 0;
				UInt32 present_capacity   = 0;
				UInt32 present_voltage    = 0;

				struct esif_data request  = {ESIF_DATA_VOID, NULL, 0, 0};
				struct esif_data response = {ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0};

				rc = EsifExecutePrimitive(i, GET_BATTERY_STATUS,
										  esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifier_str, 32), 255, &request, &response);

				if (ESIF_OK == rc && response.buf_ptr) {
					struct esif_data_binary_bst_package *bst_ptr =
						(struct esif_data_binary_bst_package*)response.buf_ptr;

					present_state    = (UInt32)bst_ptr->battery_state.integer.value;
					present_rate     = (UInt32)bst_ptr->battery_present_rate.integer.value;
					present_capacity = (UInt32)bst_ptr->battery_remaining_capacity.integer.value;
					present_voltage  = (UInt32)bst_ptr->battery_present_voltage.integer.value;
				}
				esif_ccb_free(response.buf_ptr);

				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s"
													  "    <batteryState>%d</batteryState>\n"
													  "    <batteryRate>%d</batteryRate>\n"
													  "    <batteryCapacity>%d</batteryCapacity>\n"
													  "    <batteryVoltage>%d</batteryVoltage>\n",
								 output,
								 present_state,
								 present_rate,
								 present_capacity,
								 present_voltage);
			}

			if (domain_ptr->capability_for_domain.capability_flags & ESIF_CAPABILITY_POWER_STATUS) {
				UInt32 power = 0;
				struct esif_data request  = {ESIF_DATA_VOID, NULL, 0, 0};
				struct esif_data response = {ESIF_DATA_POWER, &power, sizeof(power), 0};

				rc = EsifExecutePrimitive(i, GET_RAPL_POWER, esif_primitive_domain_str(domain_ptr->descriptor.domain,
																					   qualifier_str,
																					   32), 255, &request, &response);

				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s"
													  "    <power>%d</power>\n",
								 output,
								 power);
			}
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s"
												  "  </domain>\n",
							 output);
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s"
											  "</stat>\n",
						 output);
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s</status>\n", output);
	return output;
}


static char*esif_shell_cmd_domains (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	u8 domain_index;
	char qual_str[32];
	int i = 0;
	EsifUpPtr up_ptr = EsifUpManagerGetAvailableParticipantByInstance((UInt8)g_dst);

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s"
										  "\nDOMAINS:\n"
										  "ID Name     Description          Qual PRI Capability Device Type                      \n"
										  "-- -------- -------------------- ---- --- ---------- ---------------------------------\n",
					 output);

	if (NULL == up_ptr) {
		goto exit;
	}

	for (domain_index = 0; domain_index < up_ptr->fDspPtr->get_domain_count(up_ptr->fDspPtr); domain_index++) {
		struct esif_fpc_domain *domain_ptr = up_ptr->fDspPtr->get_domain(up_ptr->fDspPtr, domain_index + 1);
		if (NULL == domain_ptr) {
			continue;
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s"
						 "%02d %-8s %-20s %-4s %d   0x%08x %s(%d)\n",
						 output,
						 domain_index,
						 domain_ptr->descriptor.name,
						 domain_ptr->descriptor.description,
						 esif_primitive_domain_str(domain_ptr->descriptor.domain, qual_str, 32),
						 domain_ptr->descriptor.priority,
						 domain_ptr->capability_for_domain.capability_flags,
						 esif_domain_type_str((enum esif_domain_type)domain_ptr->descriptor.domainType), domain_ptr->descriptor.domainType);

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
		for (i = 0; i < 14; i++)/* Todo fix magic 14 */
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s%s(%d): %02X\n", output,
							 esif_capability_type_str((enum esif_capability_type)i), i, domain_ptr->capability_for_domain.capability_mask[i]);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", output);
exit:
	return output;
}


static char*esif_shell_cmd_pst (EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char * *argv  = shell->argv;
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
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "%s"
						 "  <ac%d>%d</ac%d>\n", output, i, *(UInt32*)response.buf_ptr, i);
	}

	temp = 255;
	rc   = EsifExecutePrimitive(participant_id, GET_TRIP_POINT_PASSIVE, qualifier_str, 255, &request, &response);
	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "%s"
					 "  <psv>%d</psv>\n", output, *(UInt32*)response.buf_ptr);

	temp = 255;
	rc   = EsifExecutePrimitive(participant_id, GET_TRIP_POINT_WARM, qualifier_str, 255, &request, &response);
	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "%s"
					 "  <wrm>%d</wrm>\n", output, *(UInt32*)response.buf_ptr);

	temp = 255;
	rc   = EsifExecutePrimitive(participant_id, GET_TRIP_POINT_CRITICAL, qualifier_str, 255, &request, &response);
	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "%s"
					 "  <crt>%d</crt>\n", output, *(UInt32*)response.buf_ptr);

	temp = 255;
	rc   = EsifExecutePrimitive(participant_id, GET_TRIP_POINT_HOT, qualifier_str, 255, &request, &response);
	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "%s"
					 "  <hot>%d</hot>\n", output, *(UInt32*)response.buf_ptr);

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "%s"
					 "</pst>\n", output);

	return output;
}


static char*esif_shell_cmd_event (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	eEsifEventType event_type;
	u8 participant_id   = 0;
	u16 domain_id       = 0;
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
		domain_id     = convert_string_to_short(qualifier_str);
	}
	EsifAppsEvent(participant_id, domain_id, event_type, NULL);

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nSEND EVENT %s(%d) PARTICIPANT %d DOMAIN %d\n", esif_event_type_str(event_type), event_type, participant_id, domain_id);

	return output;
}


static char*esif_shell_cmd_help (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	esif_ccb_sprintf(OUT_BUF_LEN, output, "ESIF CLI (c) Intel Corp. (c) 2013\n");
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%sHELP: * = REQUIRED\n"
										  "cmd line: esif [optional filename] Load And Execute Filename Then Exit\n"
										  "quit or exit                             Leave\n"
										  "format [xml|text (default)]              Command Output Format\n"
										  "info                                     Get Kernel Information\n"
										  "about                                    List ESIF Information/Apps\n"
										  "conjure [*particpant]                    Load Upper Framework Conjure Library\n"
										  "unconjure [*participant]                 Unload Upper Framework Conjure Library\n"
										  "conjures                                 List ESIF Conjure Libraries\n"
										  "infocpc  [*filename][pattern]            Get Dst CPC Information\n"
										  "cat      [*filename]                     Display Command File\n"
										  "load     [*filename]                     Load and Execute Command File\n"
										  "proof    [*filename]                     Prove Command File Replace Tokens\n"
										  "test     [*id | all]                     Test By ID or ALL Will Run All Tests\n"
										  "loadcpc  [*filename]                     Load CPC To Current Selected Participant\n"
										  "loadtst  [*filename]                     Manually Load Test\n"
										  "unloadcpc                                Unload CPC From Selected Participant\n"
										  "autocpc  [on|off]                        Automatically Load CPC When Possible\n"
										  "log      [*filename]                     Log To File If Debug On It Will Tee Output\n"
										  "nolog                                    Stop Logging To File\n"
										  "echo                                     Echo Command To File\n"
										  "rem                                      Comment / Remark\n"
										  "participants                             List Active Participants\n"
										  "participantsk                            List Kernel Participants\n"
										  "participant [*id]                        Get Participant Information\n"
										  "participantk [*id]                       Get Kernel Participant Information\n"
										  "domains                                  List Active Domains For Participant\n"
										  "soe[on|off]                              Stop On Error\n"
										  "seterrorlevel                            Set / Reset Error Level\n"
										  "geterrorlevel                            Get Current Error level\n"
										  "dst      [*id]                           Set Destination By ID\n"
										  "dstn     [*name]                         Set Destination By Name\n"
										  "mode     [mode]                          Mode 0=IOCTL 1=Read/Write 2 LAL\n"
										  "trace    [verbosity]                     Trace Level From 0 - N Least To Most Detail\n"
										  "getp     [id][qualifier][instance][test] Execute Get Primitive With Automatica Return Type and Size\n"
										  "getp_u32 [id][qualifier][instance][test] Execute U32 Get Primitive\n"
										  "getp_t   [id][qualifier][instance][test] Same as getp But Converts Temperature\n"
										  "getp_s   [id][qualifier][instance]       Same as getp But Return STRING\n"
										  "getp_b   [id][qualifier][instance]       Same as getp But Return BINARY DATA\n"
										  "getf_b   [*file]                         Same as getp_b But Reads Data From File\n"
										  "getp_bd  [id][qualifier][instance]       Same as getp But Dumps Returned BINARY DATA In Hex\n"
										  "getf_bd  [*file]                         Same as getp_bd But Reads Data From File\n"
										  "getp_bs  [id][qualifier][instance][file] Same as getp But Dumps Returned BINARY DATA To File\n"
										  "getp_bt  [id][qualifier][instance]       Same as getp But Dumps BINARY DATA As Table\n"
										  "getp_pw  [id][qualifier][instance]       Same as getp But Converts Power To milliwatts\n"
										  "setp_a   [id][qualifier][instance][data] Not Implemented Yet\n"
										  "setp     [id][qualifier][instance][data] Execute Set Primitive\n"
										  "setp_t   [id][qualifier][instance][temp] Execute Set Temperature Platform Normalization(c)\n"
										  "setp_pw  [id][qualifier][instance][power]Execute Set Power Platform Normalization(mw)\n"
										  "set_osc  [id][capablities]               Execute ACPI _OSC Command For Specified GUID Below\n"
										  "                                         Capabilities see APCI Spec\n"
										  "Active Policy 0 = {0xd6,0x41,0xa4,0x42,0x6a,0xae,0x2b,0x46,0xa8,0x4b,0x4a,0x8c,0xe7,0x90,0x27,0xd3}\n"
										  "Fail Case     1 = {0xde,0xad,0xbe,0xef,0xde,0xad,0xbe,0xef,0xde,0xad,0xbe,0xef,0xde,0xad,0xbe,0xef}\n"
										  "set_scp  [cooling mode][acoustic][power] Execute ACPI _SCP Command\n"
										  "setb     [buffer_size]                   Set Binary Buffer Size\n"
										  "getb                                     Get Binary Buffer Size\n"
										  "\n"
										  " Test:\n"
										  " -l <value>  This is the lower bounds of the testing range\n"
										  " -u <value>  This is the upper bounds of the testing range\n"
// " -f          This specifies that the test value should be\n"
// "             obtained from a file, rather than a DSP.\n"
// " -s <value>  This is used to seed primitives with a value. It can \n"
// "             also be used to write values to files (by using the -f\n"
// "             argument in conjunction with -s)\n"
										  " -b <value>  This is used for primitives that will point to a binary \n"
										  "             object for a return type. The value specified is the\n"
										  "             expected maximum size of the binary.\n"
// " -d <dir>    This is used to specifiy an export when using a binary.\n"
										  "\n"
										  "ipcauto                        Auto Connect/Retry\n"
										  "ipccon                         Force IPC Connect\n"
										  "ipcdis                         Force IPC Disconnection\n"
										  "\n"
										  "debugset  [modules]            Set Modules To Debug\n"
										  "debuglvl  [module][level]      Set Debug Level For Module\n"
										  "debugshow                      Show Debug Status\n"
										  "varset                         Assign Global Vars <INTERNAL USE ONLY!!!>\n"
										  "memstats [reset]               Show/Reset Memory Statistics\n"
										  "repeat [count]                 Repeat Next Command Next Command N Times\n"
										  "repeat_delay [+delay in ms]    Repeat Delay In msec\n"
										  "\n"
										  "timestamp  [on|off]            Show Execution Timestamp(s)\n"
										  "timerstart                     Start Interval Timer\n"
										  "timerstop                      Stop Interval Timer\n"
										  "\n"
										  "Where [modules] is a hex bit-mask (e.g. 0xfc), or list of module names\n"
										  "      [module]  is a module name\n"
										  "      [level] is a hex bit-mask of the module debugging level\n"
										  "\n"
										  "Script load parameters $1 ... $9 and Tokens $dst$\n"
										  "       proof to check parameter replacements\n"
										  "\n"
										  "APPLICATION MANAGEMENT:\n\n"
										  "appstart   [+application] Start an ESIF Application\n"
										  "appstop    [+application] Stop an ESIF Application\n"
										  "appselect  [+application] Select a running Application To Manage\n"
										  "appstatus  [+application] App Status\n"
										  "appenable  [+application] App Enable\n"
										  "appdisable [+application] App Disable\n"
										  "appinfo    [+application] App Information\n"
										  "appabout   [+application] App About\n"
										  "apps                      List all ESIF hosted Applications\n"
										  "dsps                      List all loaded DSPs\n"
										  "DSP Queries * For Wildcard Select DSP  Given Criteria:\n"
										  "dspqacpi   [*hid][*type][*uid][*scope]\n"
										  "\n"
										  "ACTION MANAGEMENT:\n\n"
										  "actionstart   [+action] Start a DSP Action\n"
										  "actionstop    [+action] Stop a DSP Action\n"
										  "actionstatus  [+action] Action Status\n"
										  "actionenable  [+action] Action Enable\n"
										  "actiondisable [+action] Action Disable\n"
										  "actioninfo    [+action] Action Information\n"
										  "actionabout   [+action] Action About\n"
										  "actions                 List all DSP Actions\n"
										  "\n"
										  "REST API:\n"
										  "reststart [*path]       Start Rest API\n"
										  "reststop                Stop Rest API\n"
										  "restevent [*id][data]   Send Rest Event\n"
										  "\n"
										  "CONFIG API:\n"
										  "cfgget   [@namespace] [*path] Get Configuration Data\n"
										  "cfgset   [@namespace] [*path] [ESIF_DATA_TYPE] [*value] [delete | persist [encrypt]] Set Configuraiton\n"
										  "\n"
										  "UI:\n"
										  "ui_getxslt           [+appname]                      Will Return XSLT Formatting information for Application\n"
										  "ui_getgroups         [+appname]                      Get A List Of UI Groups For Left Hand Tabs\n"
										  "ui_getmodulesingroup [+appname][+groupId]            Get A List Of Modules For The Specified Group\n"
										  "ui_getmoduledata     [+appname][+groupId][+moduleId] Get Data For The Specified App/Group/Module Tuple\n"
										  "EVENT API:\n"
										  "event [+eventType][participant][domain]\n\n"
					 , output);
	return output;
}


static char*esif_shell_cmd_ui_getxslt (EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char * *argv  = shell->argv;
	char *output  = shell->outbuf;
	EsifData data_status;
	eEsifError rc = ESIF_OK;
	EsifAppPtr a_app_ptr = &g_appMgr.fEntries[0];

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (NULL == a_app_ptr->fLibNamePtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find DEFAULT App\n");
		goto exit;
	}

	data_status.type     = ESIF_DATA_STRING;
	data_status.buf_ptr  = output;
	data_status.buf_len  = OUT_BUF_LEN;
	data_status.data_len = 0;

	rc = a_app_ptr->fInterface.fAppGetStatusFuncPtr(a_app_ptr->fHandle, eAppStatusCommandGetXSLT, 0, &data_status);
	if (ESIF_OK != rc) {
		goto exit;
	}
exit:
	return output;
}


static char*esif_shell_cmd_ui_getgroups (EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char * *argv  = shell->argv;
	char *output  = shell->outbuf;
	EsifData data_status;
	eEsifError rc = ESIF_OK;
	EsifAppPtr a_app_ptr = &g_appMgr.fEntries[0];

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (NULL == a_app_ptr->fLibNamePtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find DEFAULT App\n");
		goto exit;
	}

	data_status.type     = ESIF_DATA_STRING;
	data_status.buf_ptr  = output;
	data_status.buf_len  = OUT_BUF_LEN;
	data_status.data_len = 0;

	rc = a_app_ptr->fInterface.fAppGetStatusFuncPtr(a_app_ptr->fHandle, eAppStatusCommandGetGroups, 0, &data_status);
	if (ESIF_OK != rc) {
		goto exit;
	}
exit:
	return output;
}


static char*esif_shell_cmd_ui_getmodulesingroup (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	u8 gid = 0;
	char *appname_str    = 0;
	EsifData data_status;
	eEsifError rc        = ESIF_OK;
	EsifAppPtr a_app_ptr = &g_appMgr.fEntries[0];

	if (argc < 3) {
		return NULL;
	}
	appname_str = argv[1];
	gid = (u8)esif_atoi(argv[2]);

	if (NULL == a_app_ptr->fLibNamePtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find DEFAULT App\n");
		goto exit;
	}

	data_status.type     = ESIF_DATA_STRING;
	data_status.buf_ptr  = output;
	data_status.buf_len  = OUT_BUF_LEN;
	data_status.data_len = 0;

	rc = a_app_ptr->fInterface.fAppGetStatusFuncPtr(a_app_ptr->fHandle, eAppStatusCommandGetModulesInGroup, gid, &data_status);
	if (ESIF_OK != rc) {
		goto exit;
	}
exit:
	return output;
}


static char*esif_shell_cmd_ui_getmoduledata (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	u8 gid   = 0;
	u8 mid   = 0;
	u32 gmid = 0;
	char *appname_str = 0;

	EsifData data_status;
	eEsifError rc = ESIF_OK;
	EsifAppPtr a_app_ptr = &g_appMgr.fEntries[0];

	if (argc < 4) {
		return NULL;
	}

	appname_str = argv[1];
	gid  = (u8)esif_atoi(argv[2]);
	mid  = (u8)esif_atoi(argv[3]);

	gmid = gid;
	gmid = gmid << 16;
	gmid = gmid | mid;

	if (NULL == a_app_ptr->fLibNamePtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find DEFAULT App\n");
		goto exit;
	}

	data_status.type     = ESIF_DATA_STRING;
	data_status.buf_ptr  = output;
	data_status.buf_len  = OUT_BUF_LEN;
	data_status.data_len = 0;

	rc = a_app_ptr->fInterface.fAppGetStatusFuncPtr(a_app_ptr->fHandle, eAppStatusCommandGetModuleData, gmid, &data_status);
	if (ESIF_OK != rc) {
		goto exit;
	}
exit:
	return output;
}


// Run External Shell Command
static void esif_shell_exec_cmdshell (char *line)
{
	system(line);
	return;
}


///////////////////////////////////////////////////////////////////////////////
// DEBUG
///////////////////////////////////////////////////////////////////////////////

static void dump_table_hdr (struct esif_table_hdr *tab)
{
	CMD_DEBUG("Binary Dump For Table Header\n\n");
	CMD_DEBUG("binary_data_object[%u] = {\n", (int)sizeof(tab));

	CMD_DEBUG("    table header = {\n");
	CMD_DEBUG("       revision = %d,\n", tab->revision);
	CMD_DEBUG("       num of rows = %d,\n", tab->rows);
	CMD_DEBUG("       num of cols = %d,\n", tab->cols);
	CMD_DEBUG("}\n");
}


// Binary Data Variant Object
static void dump_binary_object (
	u8 *byte_ptr,
	u32 byte_len
	)
{
	union esif_data_variant *obj = (union esif_data_variant*)byte_ptr;
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
		case ESIF_DATA_TEMPERATURE:
		{
			CMD_DEBUG("    integer = { type = %s(%d) value = %lld (0x%llx) }\n",
					  esif_data_type_str(obj->integer.type),
					  obj->integer.type,
					  obj->integer.value,
					  obj->integer.value);
			obj = (union esif_data_variant*)((u8*)obj + sizeof(*obj));
			break;
		}

		case ESIF_DATA_STRING:
		{
			str = (char*)((u8*)obj + sizeof(*obj));

			CMD_DEBUG("    string = {\n");
			CMD_DEBUG("       type = %s(%d),\n", esif_data_type_str(obj->string.type), obj->string.type);
			CMD_DEBUG("       length = %d,\n", obj->string.length);
			CMD_DEBUG("       value = \"%s\"\n", str);
			CMD_DEBUG("    }\n");
			remain_bytes -= obj->string.length;
			obj = (union esif_data_variant*)((u8*)obj + (sizeof(*obj) + obj->string.length));
			break;
		}

		case ESIF_DATA_UNICODE:
		{
			u32 i = 0;
			str = (char*)((u8*)obj + sizeof(*obj));
			CMD_DEBUG("    unicode = {\n");
			CMD_DEBUG("       type = %s(%d),\n", esif_data_type_str(obj->string.type), obj->string.type);
			CMD_DEBUG("       length = %d,\n", obj->string.length);
			CMD_DEBUG("       value = ");

			for (i = 0; i < obj->string.length; i++)
				CMD_DEBUG("%02X ", (u8)str[i]);

			CMD_DEBUG("\n    }\n");
			remain_bytes -= obj->string.length;
			obj = (union esif_data_variant*)((u8*)obj + (sizeof(*obj) + obj->string.length));
			break;
		}

		case ESIF_DATA_BINARY:
		{
			u32 i = 0;
			str = (char*)((u8*)obj + sizeof(*obj));
			CMD_DEBUG("    binary = {\n");
			CMD_DEBUG("       type = %s(%d),\n", esif_data_type_str(obj->string.type), obj->string.type);
			CMD_DEBUG("       length = %d,\n", obj->string.length);
			CMD_DEBUG("       value = ");

			for (i = 0; i < obj->string.length; i++)
				CMD_DEBUG("%02X ", (u8)str[i]);

			CMD_DEBUG("\n    }\n");
			remain_bytes -= obj->string.length;
			obj = (union esif_data_variant*)((u8*)obj + (sizeof(*obj) + obj->string.length));
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
static void dump_binary_data (
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
		CMD_DEBUG("%02x", *((u8*)byte_ptr + i));

		if ('\r' == *((u8*)byte_ptr + i)) {
			CMD_DEBUG("(CR) ");
		} else if ('\n' == *((u8*)byte_ptr + i)) {
			CMD_DEBUG("(LF) ");
		} else {
			CMD_DEBUG("(%2c) ", *((u8*)byte_ptr + i));
		}
	}
	CMD_DEBUG("\n\n");
}


// Parse Command By Subsystem
EsifString parse_cmd (
	char *line,
	UInt8 isRest
	)
{
	char *in_str  = line;
	char *out_str = NULL;
	EsifAppPtr a_app_ptr = g_appMgr.fSelectedAppPtr;

#ifdef BIG_LOCK
	esif_ccb_mutex_lock(&g_shellLock);
#endif

	// Run Shell
	if (!strncmp(line, "!", 1)) {
		esif_shell_exec_cmdshell(line + 1);
		out_str = NULL;
		goto exit;
	}

	EqlParser_Init();	// Initialize Parser Class

	// The line Will Get Strtoken With \0. We Need To Save The Orginal
	// Command Line Context In Case Of Automatic Mode.
	esif_ccb_strcpy(&g_saved_command[0], line, sizeof(g_saved_command));

	line = esif_ccb_strtok(line, ESIF_SHELL_STRTOK_SEP, &g_line_context);
	if (line == NULL || *line == '#') {
		out_str = NULL;
		goto exit;
	}
	g_out_buf[0] = 0;

	//
	// Global Commands Always Available. May There Be Few
	//
	if (!strcmp(line, "appselect")) {
		esif_shell_exec_dispatch(line, g_out_buf);
		out_str = g_out_buf;
		goto exit;
	}

	if (NULL == a_app_ptr) {
		out_str = esif_shell_exec_dispatch(line, g_out_buf);
	} else {
		struct esif_data request;
		struct esif_data response;
		enum esif_rc rc = ESIF_OK;

		request.type      = ESIF_DATA_STRING;
		request.buf_ptr   = line;
		request.buf_len   = (u32)ESIF_SHELL_STRLEN(line);
		request.data_len  = (u32)ESIF_SHELL_STRLEN(line);

		response.type     = ESIF_DATA_STRING;
		response.buf_ptr  = g_out_buf;
		response.buf_len  = 64 * 1024;
		response.data_len = 0;

		rc = a_app_ptr->fInterface.fAppCommandFuncPtr(a_app_ptr->fHandle, &request, &response, g_line_context);

		if (ESIF_OK == rc) {
			out_str = g_out_buf;
		} else {
			out_str = NULL;
		}
	}
exit:
	esif_ccb_memset(in_str, 0, esif_ccb_strlen(in_str, MAX_LINE));
	if (NULL == out_str) {
		CMD_OUT("%s", "");
	}

	if ((NULL != out_str) && (out_str[0] != '\0')) {
		if (ESIF_FALSE == isRest) {
			CMD_OUT("%s", out_str);
		} else {
		}
	}
#ifdef BIG_LOCK
	esif_ccb_mutex_unlock(&g_shellLock);
#endif
	return out_str;
}


//////////////////////////////////////////////////////////////////////////////
// Alias Support

// EQL Wrapper
typedef void (*VoidFunc)();
typedef char*(*ArgvFunc)(EsifShellCmd*);
typedef char*(*EqlFunc)(char*, char*);

typedef enum FuncType_t {
	fnArgv,		// Use Command Line argc/argv Parser
	fnEQL,		// Use EQL Parser
	fnAlias,	// Use Command Line argc/argv Parser and Alias to an EQL Command
} FuncType;
typedef struct EsifShellMap_t {
	char *cmd;
	FuncType type;
	VoidFunc func;
} EsifShellMap;

// cfgget [@namespace] [*path] Get Configuraiton\n\n"
static esif_string esif_shell_cmd_cfgget (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	char nspace[ESIF_NAME_LEN] = "esif";	// Default NameSpace
	char path[ESIF_PATH_LEN];
	eEsifError rc;
	UInt32 ch;
	int opt = 1;

	ESIF_DATA(data_nspace, ESIF_DATA_STRING, nspace, sizeof(nspace));
	ESIF_DATA(data_path, ESIF_DATA_STRING, path, sizeof(path));
	ESIF_DATA(data_value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE);

	data_nspace.data_len = (u32)ESIF_SHELL_STRLEN(nspace) + 1;

	if (argc <= opt) {
		return NULL;
	}

	if ('@' == *argv[opt]) {
		esif_ccb_strcpy(nspace, &argv[opt++][1], sizeof(nspace));
		if (argc <= opt) {
			return NULL;
		}
	}
	esif_ccb_strcpy(path, argv[opt++], sizeof(path));
	data_path.data_len  = (u32)ESIF_SHELL_STRLEN(path) + 1;
	data_value.data_len = 0;

	rc = EsifConfigGet(&data_nspace, &data_path, &data_value);
	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s = %s\n", (esif_string)data_path.buf_ptr, esif_rc_str(rc));
	} else {
		esif_string tmpbuf = (esif_string)esif_ccb_malloc(OUT_BUF_LEN);
		size_t tmplen = 0;
		if (NULL == tmpbuf) {
			esif_ccb_free(data_value.buf_ptr);
			return output;
		}

		esif_ccb_sprintf(OUT_BUF_LEN, tmpbuf, "%s, len=%d, value=", esif_data_type_str(data_value.type), data_value.buf_len);
		tmplen = ESIF_SHELL_STRLEN(tmpbuf);
		switch (data_value.type) {
		case ESIF_DATA_BIT:
		case ESIF_DATA_INT8:
		case ESIF_DATA_UINT8:
			esif_ccb_sprintf(OUT_BUF_LEN - tmplen, tmpbuf + tmplen, "%u", (UInt32) * (UInt8*)data_value.buf_ptr);
			break;

		case ESIF_DATA_INT16:
		case ESIF_DATA_UINT16:
			esif_ccb_sprintf(OUT_BUF_LEN - tmplen, tmpbuf + tmplen, "%u", (UInt32) * (UInt16*)data_value.buf_ptr);
			break;

		case ESIF_DATA_INT32:
		case ESIF_DATA_UINT32:
		case ESIF_DATA_TEMPERATURE:
		case ESIF_DATA_PERCENT:
			esif_ccb_sprintf(OUT_BUF_LEN - tmplen, tmpbuf + tmplen, "%u", (UInt32) * (UInt32*)data_value.buf_ptr);
			break;

		case ESIF_DATA_INT64:
		case ESIF_DATA_UINT64:
			esif_ccb_sprintf(OUT_BUF_LEN - tmplen, tmpbuf + tmplen, "%llu", (UInt64) * (UInt64*)data_value.buf_ptr);
			break;

		case ESIF_DATA_STRING:
			esif_ccb_sprintf(OUT_BUF_LEN - tmplen, tmpbuf + tmplen, "\"%s\"", (esif_string)data_value.buf_ptr);
			break;

		// TODO:
		case ESIF_DATA_DSP:
		case ESIF_DATA_ENUM:
		case ESIF_DATA_GUID:
		case ESIF_DATA_HANDLE:
		case ESIF_DATA_INSTANCE:
		case ESIF_DATA_IPV4:
		case ESIF_DATA_IPV6:
		case ESIF_DATA_POINTER:
		case ESIF_DATA_POWER:
		case ESIF_DATA_QUALIFIER:
		case ESIF_DATA_REGISTER:
		case ESIF_DATA_STRUCTURE:
		case ESIF_DATA_TABLE:
		case ESIF_DATA_TIME:
		case ESIF_DATA_UNICODE:
		case ESIF_DATA_VOID:

		case ESIF_DATA_BINARY:
		case ESIF_DATA_BLOB:
		default:
			esif_ccb_sprintf(OUT_BUF_LEN - tmplen, tmpbuf + tmplen, "0x");
			tmplen = ESIF_SHELL_STRLEN(tmpbuf);
			for (ch = 0; NULL != data_value.buf_ptr && ch < data_value.data_len && tmplen + data_path.data_len + 8 < OUT_BUF_LEN; ch++) {
				esif_ccb_sprintf(OUT_BUF_LEN - tmplen, tmpbuf + tmplen, "%02X", (int)((UInt8*)(data_value.buf_ptr))[ch]);
				tmplen = ESIF_SHELL_STRLEN(tmpbuf);
			}
			break;
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s = %s\n", (esif_string)data_path.buf_ptr, tmpbuf);
		esif_ccb_free(tmpbuf);
	}
	esif_ccb_free(data_value.buf_ptr);
	return output;
}


// cfgset [@namespace] [*path] [ESIF_DATA_TYPE] [*value] [delete | persist [encrypt]] Set Configuraiton\n\n"
static esif_string esif_shell_cmd_cfgset (EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char * *argv = shell->argv;
	char *output = shell->outbuf;
	char nspace[ESIF_NAME_LEN] = "esif";	// Default NameSpace
	char path[ESIF_PATH_LEN];
	char value[MAX_PATH];
	UInt32 flags    = 0;
	UInt32 u32value = 0;
	UInt64 u64value = 0;
	eEsifError rc;
	enum esif_data_type type = ESIF_DATA_STRING;
	UInt32 bytes = 0;
	int opt = 1;

	ESIF_DATA(data_nspace, ESIF_DATA_STRING, nspace, sizeof(nspace));
	ESIF_DATA(data_path, ESIF_DATA_STRING, path, sizeof(path));
	ESIF_DATA(data_value, ESIF_DATA_STRING, value, sizeof(value));

	data_nspace.data_len = (u32)ESIF_SHELL_STRLEN(nspace) + 1;

	if (argc <= opt) {
		return NULL;
	}

	if ('@' == *argv[opt]) {
		// [@namespace]
		esif_ccb_strcpy(nspace, &argv[opt++][1], sizeof(nspace));
		if (argc <= opt) {
			return NULL;
		}
	}
	// [*path]
	esif_ccb_strcpy(path, argv[opt++], sizeof(path));
	data_path.data_len = (u32)ESIF_SHELL_STRLEN(path) + 1;
	if (argc <= opt) {
		return NULL;
	}

	// [ESIF_DATA_TYPE]
	if (esif_ccb_strnicmp(argv[opt], "ESIF_DATA_", 10) == 0) {
		esif_ccb_strcpy(value, argv[opt++], sizeof(value));
		type = esif_data_type_string2enum(value);
		if (argc <= opt) {
			return NULL;
		}
	}

	// [*value]
	data_value.type = type;
	switch (type) {
	case ESIF_DATA_BIT:
	case ESIF_DATA_INT8:
	case ESIF_DATA_UINT8:
		u32value = esif_atoi(argv[opt++]);
		data_value.buf_ptr = &u32value;
		data_value.buf_len = data_value.data_len = sizeof(UInt8);
		break;

	case ESIF_DATA_INT16:
	case ESIF_DATA_UINT16:
		u32value = esif_atoi(argv[opt++]);
		data_value.buf_ptr = &u32value;
		data_value.buf_len = data_value.data_len = sizeof(UInt16);
		break;

	case ESIF_DATA_INT32:
	case ESIF_DATA_UINT32:
	case ESIF_DATA_TEMPERATURE:
	case ESIF_DATA_PERCENT:
		u32value = esif_atoi(argv[opt++]);
		data_value.buf_ptr = &u32value;
		data_value.buf_len = data_value.data_len = sizeof(UInt32);
		break;

	case ESIF_DATA_INT64:
	case ESIF_DATA_UINT64:
		u64value = esif_atoi64(argv[opt++]);
		data_value.buf_ptr = &u64value;
		data_value.buf_len = data_value.data_len = sizeof(UInt64);
		break;

	// TODO:
	case ESIF_DATA_DSP:
	case ESIF_DATA_ENUM:
	case ESIF_DATA_GUID:
	case ESIF_DATA_HANDLE:
	case ESIF_DATA_INSTANCE:
	case ESIF_DATA_IPV4:
	case ESIF_DATA_IPV6:
	case ESIF_DATA_POINTER:
	case ESIF_DATA_POWER:
	case ESIF_DATA_QUALIFIER:
	case ESIF_DATA_REGISTER:
	case ESIF_DATA_STRUCTURE:
	case ESIF_DATA_TABLE:
	case ESIF_DATA_TIME:
	case ESIF_DATA_UNICODE:
	case ESIF_DATA_VOID:

	case ESIF_DATA_BINARY:
	case ESIF_DATA_BLOB:
		esif_ccb_strcpy(value, argv[opt++], sizeof(value));
		if (esif_ccb_strnicmp(value, "0x", 2) == 0) {
			for (bytes = 0; ishexdigit(value[bytes * 2 + 2]) && ishexdigit(value[bytes * 2 + 2 + 1]); bytes++) {
				value[bytes] = (UInt8)(tohexdigit(value[bytes * 2 + 2]) << 4 | tohexdigit(value[bytes * 2 + 2 + 1]));
				value[bytes * 2 + 2]     = '\0';
				value[bytes * 2 + 2 + 1] = '\0';
			}
		}
		data_value.buf_len = data_value.data_len = bytes;
		break;

	case ESIF_DATA_STRING:
	default:
		esif_ccb_strcpy(value, argv[opt++], sizeof(value));
		data_value.type     = ESIF_DATA_STRING;
		data_value.data_len = (u32)ESIF_SHELL_STRLEN(value) + 1;
		data_value.buf_len  = max(data_value.buf_len, data_value.data_len);
		break;
	}

	// [delete]
	if (argc > opt && esif_ccb_stricmp(argv[opt], "delete") == 0) {
		flags |= ESIF_SERVICE_CONFIG_DELETE;
		opt++;
	} else {
		// [persist]
		if (argc > opt && esif_ccb_stricmp(argv[opt], "persist") == 0) {
			flags |= ESIF_SERVICE_CONFIG_PERSIST;
			opt++;
		}
		// [encrypt]
		if (argc > opt && esif_ccb_stricmp(argv[opt], "encrypt") == 0) {
			flags |= ESIF_SERVICE_CONFIG_ENCRYPT;
			opt++;
		}
	}
	rc = EsifConfigSet(&data_nspace, &data_path, flags, &data_value);

	// Delete & Re-Add if buffer in NameSpace too small
	if (ESIF_E_NEED_LARGER_BUFFER == rc) {
		rc = EsifConfigSet(&data_nspace, &data_path, flags | ESIF_SERVICE_CONFIG_DELETE, &data_value);
		if (rc == ESIF_OK) {
			rc = EsifConfigSet(&data_nspace, &data_path, flags, &data_value);
		}
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));

	return output;
}


// Shell Command Wrapper
char*esif_shell_cmd_ipc_autoconnect (EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	ipc_autoconnect();
	return NULL;
}


// Shell Command Wrapper
char*esif_shell_cmd_ipc_connect (EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	ipc_connect();
	return NULL;
}


// Shell Command Wrapper
char*esif_shell_cmd_ipc_disconnect (EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	ipc_disconnect();
	return NULL;
}


// Parse and Execute a single EQL command
static esif_string esif_shell_exec_eql (
	esif_string line,
	esif_string output
	)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	UNREFERENCED_PARAMETER(output);

	line[ESIF_SHELL_STRLEN(line)] = ' ';// hack to work around strtok() call

	rc = EqlParser_ExecuteEql(line, 0);
	return NULL;
}


// EQL Alias Command Dispatcher
char*esif_shell_exec_alias (
	ArgvFunc fnPtr,
	EsifShellCmdPtr shell
	)
{
	int argc                = shell->argc;
	char * *argv            = shell->argv;
	char *output            = shell->outbuf;
	char aliascmd[MAX_LINE] = {0};
	eEsifError rc           = ESIF_E_UNSPECIFIED;
	EqlParserPtr parser     = 0;
	EqlCmdPtr eqlcmd        = 0;

	//// Create Alias Command ////

	// getp primitive_id qualifier instance
	if (argc > argc) {
		// placeholder
	} else if (esif_ccb_strnicmp(argv[0], "getp", 4) == 0 && argc >= 4) {
		char *datatype = 0;
		if (ESIF_SHELL_STRLEN(argv[0]) > 4 && argv[0][4] == '_') {
			if (esif_ccb_stricmp(&argv[0][5], "u32") == 0) {
				datatype = "UINT32";
			} else if (esif_ccb_stricmp(&argv[0][5], "t") == 0) {
				datatype = "TEMPERATURE";
			} else if (esif_ccb_stricmp(&argv[0][5], "s") == 0) {
				datatype = "STRING";
			} else if (esif_ccb_stricmp(&argv[0][5], "b") == 0) {
				datatype = "BINARY";
			} else if (esif_ccb_stricmp(&argv[0][5], "pw") == 0) {
				datatype = "POWER";
			}

			/* TODO:
			   "getp_bd  [id][qualifier][instance]       Same as getp But Dumps Returned BINARY DATA In Hex\n"
			   "getp_bs  [id][qualifier][instance][file] Same as getp But Dumps Returned BINARY DATA To File\n"
			   "getp_bt  [id][qualifier][instance]       Same as getp But Dumps BINARY DATA As Table\n"
			 */
		}
		esif_ccb_sprintf(MAX_LINE, aliascmd, "get primitive(%s,%s,%s)%s%s", argv[1], argv[2], argv[3], (datatype ? "AS " : ""), (datatype ? datatype : ""));
	}
	// cfgget [@namespace] [*path]
	else if (esif_ccb_strnicmp(argv[0], "cfgget", 6) == 0 && argc >= 2) {
		int arg = 1;
		char *namesp = (*argv[arg] == '@' ? &argv[arg++][1] : 0);
		esif_ccb_sprintf(MAX_LINE, aliascmd, "get config%s%s(\"%s\")",
						 (namesp ? ":" : ""), (namesp ? namesp : ""), argv[arg]);
	}
	// cfgset [@namespace] [*path] [ESIF_DATA_TYPE] [*value] [delete | persist [encrypt]] Set Configuraiton\n\n"
	else if (esif_ccb_strnicmp(argv[0], "cfgset", 6) == 0 && argc >= 3) {
		int arg = 1;
		int opt = 0;
		char *namesp = (*argv[arg] == '@' ? &argv[arg++][1] : 0);
		esif_ccb_sprintf(MAX_LINE, aliascmd, "set config%s%s(\"%s\")=", (namesp ? ":" : ""), (namesp ? namesp : ""), argv[arg++]);
		if (esif_ccb_strnicmp(argv[arg], "ESIF_DATA_", 10) == 0) {
			esif_ccb_sprintf(MAX_LINE - ESIF_SHELL_STRLEN(aliascmd), aliascmd + ESIF_SHELL_STRLEN(aliascmd), "{%s:%s}", argv[arg++], argv[arg++]);
		} else {
			esif_ccb_sprintf(MAX_LINE - ESIF_SHELL_STRLEN(aliascmd), aliascmd + ESIF_SHELL_STRLEN(aliascmd), "\"%s\"", argv[arg++]);
		}
		opt = arg;
		while (arg < argc)
			esif_ccb_sprintf(MAX_LINE - ESIF_SHELL_STRLEN(aliascmd), aliascmd + ESIF_SHELL_STRLEN(aliascmd), "%s%s", (arg > opt ? ", " : " "), argv[arg++]);
	}

	//// Execute Alias /////

	// Create our own parsing object so we can process the results
	if (*aliascmd) {
		if ((parser = EqlParser_Create()) == NULL) {
			return NULL;
		}
		eqlcmd = EqlParser_Parse(parser, aliascmd);
		if (eqlcmd) {
			rc = EqlCmd_Dispatch(eqlcmd, 0);
			if (rc == ESIF_OK) {
				int j;
				for (j = 0; j < eqlcmd->results->size; j++) {
					StringPtr result = EsifData_ToString(&eqlcmd->results->elements[j]);
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", result);
					esif_ccb_free(result);
				}
			} else {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
			}
		}
		EqlParser_Destroy(parser);
	} else {// Unknown Alias
		output = (*fnPtr)(shell);
	}
	return output;
}


// Shell Command Mapping. Keep this array sorted alphabetically to facilitate Binary Searches
static EsifShellMap ShellCommands[] = {
	{"about",                fnArgv, (VoidFunc)esif_shell_cmd_about               },
	{"actions",              fnArgv, (VoidFunc)esif_shell_cmd_actions             },
	{"actionstart",          fnArgv, (VoidFunc)esif_shell_cmd_actionstart         },
	{"actionstop",           fnArgv, (VoidFunc)esif_shell_cmd_actionstop          },
	{"apps",                 fnArgv, (VoidFunc)esif_shell_cmd_apps                },
	{"appselect",            fnArgv, (VoidFunc)esif_shell_cmd_appselect           },// Global Command
	{"appstart",             fnArgv, (VoidFunc)esif_shell_cmd_appstart            },
	{"appstop",              fnArgv, (VoidFunc)esif_shell_cmd_appstop             },
	{"autocpc",              fnArgv, (VoidFunc)esif_shell_cmd_autocpc             },
	{"cat",                  fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"cattst",               fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"cfgget",               fnArgv, (VoidFunc)esif_shell_cmd_cfgget              },// Alias for "get config(name)"
	{"cfgset",               fnArgv, (VoidFunc)esif_shell_cmd_cfgset              },// Alias for "set config(name)=value [options]"
	{"conjure",              fnArgv, (VoidFunc)esif_shell_cmd_conjure             },
	{"conjures",             fnArgv, (VoidFunc)esif_shell_cmd_conjures            },
	{"debuglvl",             fnArgv, (VoidFunc)esif_shell_cmd_debuglvl            },
	{"debugset",             fnArgv, (VoidFunc)esif_shell_cmd_debugset            },
	{"debugshow",            fnArgv, (VoidFunc)esif_shell_cmd_debugshow           },
	{"delete",               fnEQL,  (VoidFunc)esif_shell_exec_eql                },
	{"domains",              fnArgv, (VoidFunc)esif_shell_cmd_domains             },
	{"dspqacpi",             fnArgv, (VoidFunc)esif_shell_cmd_dspqacpi            },
	{"dsps",                 fnArgv, (VoidFunc)esif_shell_cmd_dsps                },
	{"dst",                  fnArgv, (VoidFunc)esif_shell_cmd_dst                 },
	{"dstn",                 fnArgv, (VoidFunc)esif_shell_cmd_dstn                },
	{"echo",                 fnArgv, (VoidFunc)esif_shell_cmd_echo                },
	{"event",                fnArgv, (VoidFunc)esif_shell_cmd_event               },
	{"exit",                 fnArgv, (VoidFunc)esif_shell_cmd_exit                },
	{"format",               fnArgv, (VoidFunc)esif_shell_cmd_format              },
	{"get",                  fnEQL,  (VoidFunc)esif_shell_exec_eql                },
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
	{"info",                 fnArgv, (VoidFunc)esif_shell_cmd_info                },
	{"infocpc",              fnArgv, (VoidFunc)esif_shell_cmd_infocpc             },
	{"ipcauto",              fnArgv, (VoidFunc)esif_shell_cmd_ipc_autoconnect     },
	{"ipccon",               fnArgv, (VoidFunc)esif_shell_cmd_ipc_connect         },
	{"ipcdis",               fnArgv, (VoidFunc)esif_shell_cmd_ipc_disconnect      },
	{"load",                 fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"loadcpc",              fnArgv, (VoidFunc)esif_shell_cmd_loadcpc             },
	{"loadtst",              fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"log",                  fnArgv, (VoidFunc)esif_shell_cmd_log                 },
	{"memstats",             fnArgv, (VoidFunc)esif_shell_cmd_memstats            },
	{"mode",                 fnArgv, (VoidFunc)esif_shell_cmd_ipcmode             },
	{"nolog",                fnArgv, (VoidFunc)esif_shell_cmd_nolog               },
	{"participant",          fnArgv, (VoidFunc)esif_shell_cmd_participant         },
	{"participantk",         fnArgv, (VoidFunc)esif_shell_cmd_participantk        },
	{"participants",         fnArgv, (VoidFunc)esif_shell_cmd_participants        },
	{"participantsk",        fnArgv, (VoidFunc)esif_shell_cmd_participantsk       },
	{"proof",                fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"prooftst",             fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"pst",                  fnArgv, (VoidFunc)esif_shell_cmd_pst                 },
	{"quit",                 fnArgv, (VoidFunc)esif_shell_cmd_quit                },
	{"rem",                  fnArgv, (VoidFunc)esif_shell_cmd_rem                 },
	{"repeat",               fnArgv, (VoidFunc)esif_shell_cmd_repeat              },
	{"repeat_delay",         fnArgv, (VoidFunc)esif_shell_cmd_repeatdelay         },
	{"restevent",            fnArgv, (VoidFunc)esif_shell_cmd_restevent           },
	{"reststart",            fnArgv, (VoidFunc)esif_shell_cmd_reststart           },
	{"reststop",             fnArgv, (VoidFunc)esif_shell_cmd_reststop            },
	{"set",                  fnEQL,  (VoidFunc)esif_shell_exec_eql                },
	{"set_osc",              fnArgv, (VoidFunc)esif_shell_cmd_set_osc             },
	{"set_scp",              fnArgv, (VoidFunc)esif_shell_cmd_set_scp             },
	{"setb",                 fnArgv, (VoidFunc)esif_shell_cmd_setb                },
	{"seterrorlevel",        fnArgv, (VoidFunc)esif_shell_cmd_seterrorlevel       },
	{"setp",                 fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"setp_pw",              fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"setp_t",               fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"soe",                  fnArgv, (VoidFunc)esif_shell_cmd_soe                 },
	{"status",               fnArgv, (VoidFunc)esif_shell_cmd_status              },
	{"test",                 fnArgv, (VoidFunc)esif_shell_cmd_test                },
	{"timerstart",           fnArgv, (VoidFunc)esif_shell_cmd_timerstart          },
	{"timerstop",            fnArgv, (VoidFunc)esif_shell_cmd_timerstop           },
	{"timestamp",            fnArgv, (VoidFunc)esif_shell_cmd_timestamp           },
	{"trace",                fnArgv, (VoidFunc)esif_shell_cmd_trace               },
	{"ui_getgroups",         fnArgv, (VoidFunc)esif_shell_cmd_ui_getgroups        },
	{"ui_getmoduledata",     fnArgv, (VoidFunc)esif_shell_cmd_ui_getmoduledata    },
	{"ui_getmodulesingroup", fnArgv, (VoidFunc)esif_shell_cmd_ui_getmodulesingroup},
	{"ui_getxslt",           fnArgv, (VoidFunc)esif_shell_cmd_ui_getxslt          },
	{"unconjure",            fnArgv, (VoidFunc)esif_shell_cmd_unconjure           },
	{"unloadcpc",            fnArgv, (VoidFunc)esif_shell_cmd_unloadcpc           },
	{"varset",               fnArgv, (VoidFunc)esif_shell_cmd_varset              },
};

// ESIF Command Dispatcher
char*esif_shell_exec_dispatch (
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
			char *argv[32] = {0};
			switch (ShellCommands[node].type) {
			// Use EQL Parser
			case fnEQL:
				rcStr = (*(EqlFunc)(ShellCommands[node].func))(line, output);
				break;

			// Use Command Line argc/argv Parser
			case fnArgv:
			case fnAlias:
				do {
					if (*cmd == ';') {	// break on comment
						break;
					}
					argv[argc++] = cmd;
				} while ((cmd = esif_ccb_strtok(NULL, ESIF_SHELL_STRTOK_SEP, &g_line_context)) != NULL);

				shell.argc   = argc;
				shell.argv   = argv;
				shell.outbuf = output;
				if (ShellCommands[node].type == fnArgv) {
					rcStr = (*(ArgvFunc)(ShellCommands[node].func))(&shell);
				} else {
					rcStr = esif_shell_exec_alias((ArgvFunc)ShellCommands[node].func, &shell);
				}
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


