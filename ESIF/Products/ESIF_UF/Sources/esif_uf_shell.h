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
void esif_uf_shell_exit(void);

void esif_uf_shell_lock(void);
void esif_uf_shell_unlock(void);

char *esif_shell_resize(size_t buf_len);

EsifString esif_cmd_info(EsifString output);

#ifdef __cplusplus
}
#endif

#endif /* _ESIF_UF_SHELL_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
