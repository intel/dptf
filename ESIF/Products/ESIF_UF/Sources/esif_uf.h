/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_
#define _ESIF_UF_

#include "esif.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DCFG Bitmask Options. Default Values must always be 0 for backwards compatibility */
typedef union DCfgOptions_u {
	struct {
		UInt32 GenericUIAccessControl : 1;			// 0:0 - 0 = Enable [default], 1 = Disable access
		UInt32 RestrictedUIAccessControl : 1;		// 1:1 - 0 = Enable [default], 1 = Disable access
		UInt32 ShellAccessControl : 1;				// 2:2 - 0 = Enable [default], 1 = Disable access
		UInt32 EnvMonitoringReportControl : 1;		// 3:3 - 0 = Report is allowed [default], 1 = No environmental monitoring report to Microsoft
		UInt32 ThermalMitigationReportControl : 1;	// 4:4 - 0 = No mitigation report to Microsoft [default], 1 = Report is allowed
		UInt32 ThermalPolicyReportControl : 1;		// 5:5 - 0 = No thermal policy report to Microsoft [default], 1 = Report is allowed
		UInt32 Reserved : 26;						// 31:6  Reserved
	} opt;
	UInt32 asU32;
} DCfgOptions;

extern DCfgOptions DCfg_Get();
extern void DCfg_Set(DCfgOptions opt);

typedef eEsifError (*EsifInitFunc)();
typedef void (*EsifExitFunc)();

typedef struct EsifInitTableEntry_s {
	EsifInitFunc initFunc;
	EsifExitFunc exitFunc;
	esif_flags_t flags;
} EsifInitTableEntry, *EsifInitTableEntryPtr;

// ESIF Init Table Entry flags:
#define ESIF_INIT_FLAG_NONE					0
#define	ESIF_INIT_FLAG_IGNORE_ERROR			1
#define	ESIF_INIT_FLAG_CHECK_STOP_AFTER		2

// ESIF Log Types
typedef enum eEsifLogType {
	ESIF_LOG_EVENTLOG     = 0,	// Overrides System Event Logger (Windows=EventLog, Linux=syslog)
	ESIF_LOG_DEBUGGER     = 1,	// Overrides System Debug Logger (Windows=OutputDebugString, Linux=syslog)
	ESIF_LOG_SHELL        = 2,	// CMD_OUT output when shell log enabled (log/nolog)
	ESIF_LOG_TRACE        = 3,	// ESIF_TRACE_* output when trace log enabled
	ESIF_LOG_UI           = 4,	// UI output when ui log enabled
	ESIF_LOG_PARTICIPANT  = 5,  // Participant Log
} EsifLogType;

#define MAX_ESIFLOG		6	// Max Log Types


// Log File API
extern int EsifLogFile_Open(EsifLogType type, const char *filename, int append);
extern int EsifLogFile_Close(EsifLogType type);
extern int EsifLogFile_IsOpen(EsifLogType type);
extern int EsifLogFile_Write(EsifLogType type, const char *fmt, ...);
extern int EsifLogFile_WriteArgs(EsifLogType type, const char *fmt, va_list args);
extern int EsifLogFile_WriteArgsAppend(EsifLogType type, const char *append, const char *fmt, va_list args);
extern esif_string EsifLogFile_GetFullPath(esif_string buffer, size_t buf_len, const char *filename);
extern void EsifLogFile_DisplayList(void);
extern esif_string EsifLogFile_GetFileNameFromType(EsifLogType logType);

extern EsifLogType EsifLogType_FromString(const char *name);

// Console Output API
#define CMD_WRITETO_CONSOLE	0x1		// Write to Console or Client
#define CMD_WRITETO_LOGFILE	0x2		// Write to Log File, if open

extern int EsifConsole_WriteTo(u32 writeto, const char *format, ...);
extern int EsifConsole_WriteConsole(const char *format, va_list args);
extern int EsifConsole_WriteLogFile(const char *format, va_list args);

// Write to console and optional shell log
#define CMD_OUT(format, ...)		EsifConsole_WriteTo(CMD_WRITETO_CONSOLE|CMD_WRITETO_LOGFILE, format, ##__VA_ARGS__)
#define CMD_DEBUG(format, ...)		EsifConsole_WriteTo(CMD_WRITETO_CONSOLE|CMD_WRITETO_LOGFILE, format, ##__VA_ARGS__)

// Write to console only
#define CMD_CONSOLE(format, ...)	EsifConsole_WriteTo(CMD_WRITETO_CONSOLE, format, ##__VA_ARGS__)

// Write to optional shell log only
#define CMD_LOGFILE(format, ...)	EsifConsole_WriteTo(CMD_WRITETO_LOGFILE, format, ##__VA_ARGS__)

extern  UInt32 g_outbuf_len;				// Current (or Default) Size of ESIF Shell Output Buffer
#define OUT_BUF_LEN			g_outbuf_len	// Alias for backwards compatibility
#define OUT_BUF_LEN_DEFAULT	(64 * 1024)		// Default size for ESIF Shell Output Buffer

#define ENUM_TO_STRING_LEN 12

// ESIF Path Types (** = Read/Write, all others Read-only)
typedef enum e_esif_pathtype {
	ESIF_PATHTYPE_HOME,		// Home directory
	ESIF_PATHTYPE_TEMP,		// Temp Directory **
	ESIF_PATHTYPE_DV,		// DataVault Directory **
	ESIF_PATHTYPE_LOG,		// Log Files **
	ESIF_PATHTYPE_BIN,		// Binary output files **
	ESIF_PATHTYPE_LOCK,		// Lock Files **
	ESIF_PATHTYPE_EXE,		// Binary Executables (.EXE and ELF binaries)
	ESIF_PATHTYPE_DLL,		// Dynamically Loadable Libraries (.DLL and .so)
	ESIF_PATHTYPE_DPTF,		// DPTF Policy Path sent via CreateAppData
	ESIF_PATHTYPE_DSP,		// DSP and EDP files
	ESIF_PATHTYPE_CMD,		// CMD scripts
	ESIF_PATHTYPE_UI,		// UI and HTML files
} esif_pathtype;

esif_string esif_build_path(
	esif_string buffer,
	size_t buf_len,
	esif_pathtype type,
	esif_string filename,
	esif_string ext);

//
// Format
//
enum output_format {
	FORMAT_TEXT = 0,// Text
	FORMAT_XML		// XML
};

extern enum output_format g_format;

#define MAX_LINE 256

// Functions
unsigned int esif_atoi(const esif_string value);

//
// Utilities
//
enum esif_rc esif_shell_execute(const char *command);
char *esif_shell_exec_command(const char *line, size_t buf_len, UInt8 IsRest, UInt8 showOutput);
char *parse_cmd(const char *line, UInt8 IsRest, UInt8 showOutput);
UInt16 domain_str_to_short(esif_string two_character_string);
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);

/*
 * GUID - String requires a 37 byte buffer minimum
 */
#define ESIF_GUID_PRINT_SIZE 64
static ESIF_INLINE esif_string esif_guid_print(
	esif_guid_t *guid,
	esif_string buf
	)
{
	u8 *ptr = (u8 *)guid;
	esif_ccb_sprintf(ESIF_GUID_PRINT_SIZE,
			 buf,
			 "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			 *ptr,
			 *(ptr + 1),
			 *(ptr + 2),
			 *(ptr + 3),
			 *(ptr + 4),
			 *(ptr + 5),
			 *(ptr + 6),
			 *(ptr + 7),
			 *(ptr + 8),
			 *(ptr + 9),
			 *(ptr + 10),
			 *(ptr + 11),
			 *(ptr + 12),
			 *(ptr + 13),
			 *(ptr + 14),
			 *(ptr + 15)
			 );
	return buf;
}


static ESIF_INLINE void esif_copy_shorts_to_bytes(
	UInt8 *byte_ptr,
	UInt16 *short_ptr,
	size_t count
	)
{
	if ((NULL == byte_ptr) || (NULL == short_ptr)) {
		goto exit;
	}
	while(count--) {
		*byte_ptr++ = (UInt8)*short_ptr++;
	}
exit:
	return;
}



//
// DSP
//

/*
 * DSP Version is the File Format version of the DSP source file and the generated EDP file. 
 * It is not the same as the major/minor version in the H record, which is the content version:
 *  1. Major.Minor match required between .dsp file and dsp_compiler to generate .edp
 *  2. Major match required between .edp file and esif_uf (Major embedded in .edp)
 * Major Version should change when .edp, .fpc, .cpc, or .dsp file format(s) change
 * Minor Version should change when only .dsp file format changes and .edp is unaffected
 */
#define ESIF_DSP_VERSION	"3.0"	// DSP Version [Major=EDP Version, Minor=DSP Revision]
#define ESIF_EDP_SIGNATURE	"@EDP"	// EDP File Signature (converted to UINT)
#define ESIF_CPC_SIGNATURE	"@CPC"	// CPC File Signature (converted to UINT)

struct edp_dir;
Bool esif_verify_edp(struct edp_dir *edp, size_t size);
enum esif_rc esif_send_dsp(esif_string filename, UInt8 dst);

/* Init / Exit */
eEsifError esif_uf_init(void);
void esif_uf_stop_init(void);
void esif_uf_exit(void);
extern int g_stopEsifUfInit;

enum esif_rc esif_main_init(esif_string path_list);
void esif_main_exit(void);

/* Path Management */
enum esif_rc esif_pathlist_set(esif_pathtype type, esif_string value);
esif_string esif_pathlist_get(esif_pathtype type);
int esif_pathlist_count(void);

/* OS Specific Init / Exit */
eEsifError esif_uf_os_init(void);
void esif_uf_os_exit(void);

// Web
extern eEsifError EsifWebStart();
extern void EsifWebStop(void);
extern int EsifWebIsStarted();
extern void EsifWebSetIpaddrPort(const char *ipaddr, u32 port, Bool restricted);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

