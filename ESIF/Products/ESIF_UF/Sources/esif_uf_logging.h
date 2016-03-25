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

#pragma once
#include "esif_ccb.h"
#include "esif_sdk.h"
#include "esif_uf_shell.h"

#define MAX_LOG_LINE	24 * 1024
#define DEFAULT_STATUS_LOG_INTERVAL 5000
#define MIN_STATUS_LOG_INTERVAL 800
#define MIN_STATUS_LOG_INTERVAL_ADJUSTED 1
#define MIN_STATUS_LOG_SCHEDULE 800


typedef struct DataLogParticipant_s {
	UInt8 participantId;
	int participantNumFields;
} DataLogParticipant, *DataLogParticipantPtr;


#ifdef __cplusplus
extern "C" {
#endif

void EsifDataLogStop();
char *EsifShellCmdDataLog(EsifShellCmdPtr shell);

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

