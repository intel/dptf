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

extern int g_debug;
extern FILE* g_debuglog;

//
// output to screen and optional log file
//
#ifdef ESIF_ATTR_OS_WINDOWS

    #define BIG_LOCK

    enum esif_rc write_to_srvr_cnsl_intfc(char * pFormat, ...);

    #define LOCAL_DEBUG(__msg, ...)  if(g_debug) printf(__msg, ##__VA_ARGS__); \
        if(g_debuglog && g_debuglog != stdout) fprintf(g_debuglog, __msg, ##__VA_ARGS__);
    #define CMD_OUT(__msg, ...)  if(g_debug) write_to_srvr_cnsl_intfc(__msg, ##__VA_ARGS__); \
        if(g_debuglog && g_debuglog != stdout) fprintf(g_debuglog, __msg, ##__VA_ARGS__);
    #define CMD_DEBUG LOCAL_DEBUG

#else  // !ESIF_ATTR_OS_WINDOWS

    #define LOCAL_DEBUG(__msg, ...)  if(g_debug) printf(__msg, ##__VA_ARGS__); \
        if(g_debuglog && g_debuglog != stdout) fprintf(g_debuglog, __msg, ##__VA_ARGS__);
    #define CMD_OUT(__msg, ...)  if(g_debug) printf(__msg, ##__VA_ARGS__); \
        if(g_debuglog && g_debuglog != stdout) fprintf(g_debuglog, __msg, ##__VA_ARGS__);
    #define CMD_DEBUG LOCAL_DEBUG

#endif //  !ESIF_ATTR_OS_WINDOWS

#define OUT_BUF_LEN 64*1024

#define ESIF_DIR_DSP "dsp"
#define ESIF_DIR_CMD "cmd"
#define ESIF_DIR_LOG "log"
#define ESIF_DIR_BIN "bin"

#ifdef _WIN64
#define ESIF_DIR_PRG "ufx64"
#else
#define ESIF_DIR_PRG "ufx86"
#endif

#define ESIF_DIR_UI  "esif_ui"
#define ESIF_DIR_REST "esif_cmd"

esif_string esif_build_path(esif_string buffer, u32 buf_len, esif_string dir, esif_string filename);

//
// Format
//
enum output_format {
    FORMAT_TEXT = 0, // Text
    FORMAT_XML       // XML
};
extern enum output_format g_format;

#define MAX_LINE 256

//
// Functions
//
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
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);

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

#endif // _ESIF_UF_