/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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
#include "esif_ccb_lock.h"

#define BIG_LOCK

// Temporary Workaround: Implment global Shell lock to troubleshoot mixed output
#ifdef  BIG_LOCK
extern esif_ccb_mutex_t g_shellLock;
#endif

// ESIF Log Types
typedef enum eEsifLogType {
    ESIF_LOG_EVENTLOG = 0,  // Overrides System Event Logger (Windows=EventLog, Linux=syslog)
    ESIF_LOG_DEBUGGER = 1,  // Overrides System Debug Logger (Windows=OutputDebugString, Linux=syslog)
    ESIF_LOG_SHELL    = 2,  // CMD_OUT output when shell log enabled (log/nolog)
    ESIF_LOG_TRACE    = 3,  // ESIF_TRACE_* output when trace log enabled
    ESIF_LOG_UI       = 4,  // UI output when ui log enabled
} EsifLogType;
#define MAX_ESIFLOG     5   // Max Log Types

// Log File API
extern int EsifLogFile_Open(EsifLogType type, const char *filename);
extern int EsifLogFile_Close(EsifLogType type);
extern int EsifLogFile_IsOpen(EsifLogType type);
extern int EsifLogFile_Write(EsifLogType type, const char *fmt, ...);
extern int EsifLogFile_WriteArgs(EsifLogType type, const char *fmt, va_list args);
extern esif_string EsifLogFile_GetFullPath(esif_string buffer, size_t buf_len, const char *filename);
extern void EsifLogFile_DisplayList(void);

extern EsifLogType EsifLogType_FromString(const char *name);

// Console Output API
#define CMD_WRITETO_CONSOLE 0x1     // Write to Console or Client
#define CMD_WRITETO_LOGFILE 0x2     // Write to Log File, if open

extern int EsifConsole_WriteTo(u32 writeto, const char *format, ...);
extern int EsifConsole_WriteConsole(const char *format, va_list args);
extern int EsifConsole_WriteLogFile(const char *format, va_list args);

// Write to console and optional shell log
#define CMD_OUT(format, ...)        EsifConsole_WriteTo(CMD_WRITETO_CONSOLE|CMD_WRITETO_LOGFILE, format, ##__VA_ARGS__)
#define CMD_DEBUG(format, ...)      EsifConsole_WriteTo(CMD_WRITETO_CONSOLE|CMD_WRITETO_LOGFILE, format, ##__VA_ARGS__)

// Write to console only
#define CMD_CONSOLE(format, ...)    EsifConsole_WriteTo(CMD_WRITETO_CONSOLE, format, ##__VA_ARGS__)

// Write to optional shell log only
#define CMD_LOGFILE(format, ...)    EsifConsole_WriteTo(CMD_WRITETO_LOGFILE, format, ##__VA_ARGS__)


#define OUT_BUF_LEN 64 * 1024

#if defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_ANDROID) || \
    defined(ESIF_ATTR_OS_CHROME)
#define ESIF_DIR_DSP "/usr/share/dptf/dsp"
#define ESIF_DIR_CMD "/usr/share/dptf/cmd"
#else
#define ESIF_DIR_DSP "dsp"
#define ESIF_DIR_CMD "cmd"
#endif

#define ESIF_DIR_LOG "log"
#define ESIF_DIR_BIN "bin"

//
// Set Architecture
//
#if defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_ANDROID) || \
    defined(ESIF_ATTR_OS_CHROME)
// No differentiation for 32-bit or 64-bit *nix build
#define ESIF_DIR_PRG NULL
#define ESIF_DIR_DPTF_POL "/usr/share/dptf"
#define ESIF_DIR_UI "/usr/share/dptf/"
#elif defined(ESIF_ATTR_64BIT)
#define ESIF_DIR_PRG "ufx64"
#define ESIF_DIR_DPTF_POL "ufx64"
#else
#define ESIF_DIR_PRG "ufx86"
#define ESIF_DIR_DPTF_POL "ufx86"
#endif

#define ESIF_DIR_REST "esif_cmd"

esif_string esif_build_path(esif_string buffer, u32 buf_len, esif_string dir, esif_string filename);

//
// Format
//
enum output_format {
    FORMAT_TEXT = 0,// Text
    FORMAT_XML      // XML
};

extern enum output_format g_format;

#define MAX_LINE 256

// Functions
unsigned int esif_atoi(const esif_string value);

//
// SubSystem
//
enum app_subsystem {
    SUBSYSTEM_ESIF = 0
};

extern enum app_subsystem subsystem;
void cmd_app_subsystem(const enum app_subsystem subsystem);

//
// Utilities
//
EsifString parse_cmd(EsifString line, UInt8 IsRest);
UInt16 convert_string_to_short(esif_string two_character_string);
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);

//
// DSP
//

int esif_send_dsp(esif_string filename, UInt8 dst);

/* Init / Exit */
enum esif_rc esif_uf_init(esif_string home_dir);
void esif_uf_exit(void);

/* OS Specific Init / Exit */
enum esif_rc esif_uf_os_init(void);
void esif_uf_os_exit(void);

// Web
extern eEsifError EsifWebStart();
extern void EsifWebStop();
extern int EsifWebIsStarted();
extern void EsifWebSetIpaddrPort(const char *ipaddr, u32 port);

#endif // _ESIF_UF_