/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_SHELL_
#define _ESIF_UF_SHELL_

#include "esif.h"

typedef struct EsifShellCmd_s {
	int   argc;
	char  **argv;
	char  *outbuf;
} EsifShellCmd, *EsifShellCmdPtr;


#ifdef __cplusplus
extern "C" {
#endif

eEsifError esif_uf_shell_init(void);
eEsifError esif_uf_shell_banner_init(void);
void esif_uf_shell_exit(void);
void esif_uf_shell_stop(void);

void esif_uf_shell_lock();
void esif_uf_shell_unlock();

eEsifError esif_shell_dispatch(int argc, char **argv, char **output_ptr);
eEsifError esif_shell_dispatch_cmd(const char *line, char **output_ptr);

char *esif_shell_resize(size_t buf_len);

void esif_shell_set_start_script(const char *script);
const char *esif_shell_get_start_script(void);

char *esif_shell_strtok(
	char *str,
	char *seps,
	char **context
);

char *esif_cmd_info(char *output);

// Dynamic Participants DataVault Definitions
#define DYNAMIC_PARTICIPANTS_OBJTYPE	"participant"
#define DYNAMIC_PARTICIPANTS_REVISION	"2"
#define DYNAMIC_PARTICIPANTS_DATAVAULT	"platform"
#define DYNAMIC_PARTICIPANTS_KEYSPACE	"/participants/"
#define DYNAMIC_PARTICIPANTS_KEYSPEC	DYNAMIC_PARTICIPANTS_KEYSPACE "*"
#define STARTUP_AUTOEXEC_DATAVAULT		DYNAMIC_PARTICIPANTS_DATAVAULT
#define STARTUP_AUTOEXEC_KEYSPEC		"/autoexec/*"

extern esif_error_t CreateParticipantFromJson(esif_string jsonStr);
extern esif_error_t CreateDynamicParticipants();
extern esif_error_t CreatePcieParticipant();
extern esif_error_t CreateDisplayParticipant();
extern esif_error_t CreateVpuParticipant();
extern esif_error_t CreateWifiParticipant();
extern esif_error_t CreateIdg2Participant();
extern esif_error_t CreateMcpParticipant();
extern esif_error_t CreateIvramParticipant();
extern esif_error_t CreateIpuParticipant();
extern esif_error_t CreateAudioParticipant();
extern esif_error_t CreateMemoryParticipant();

#ifdef __cplusplus
}
#endif

#endif /* _ESIF_UF_SHELL_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
