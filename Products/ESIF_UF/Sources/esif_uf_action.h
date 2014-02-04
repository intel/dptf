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

#ifndef _ESIF_UF_ACTION_
#define _ESIF_UF_ACTION_

#include "esif.h"
#include "esif_uf_action_iface.h"

/* Map App Data To ESIF Prticipants */
typedef struct _t_EsifAct {
	void  *fHandle;			/* The Action Handle Opaque To Us */
	EsifActInterface  fInterface;		/* The Action Interface */
	EsifString fLibNamePtr;		/* The Name Of The Library To Load */
	esif_lib_t fLibHandle;		/* Loadable Library Handle */
} EsifAct, *EsifActPtr, **EsifActPtrLocation;

/* Control */
eEsifError EsifActStart(EsifActPtr actPtr);
eEsifError EsifActStop(EsifActPtr actPtr);

/* Init/Exit */
eEsifError EsifActInit(void);
void EsifActExit(void);

eEsifError EsifActConfigInit(void);
void EsifActConfigExit(void);

eEsifError EsifActConstInit(void);
void EsifActConstExit(void);

eEsifError EsifActSystemInit(void);
void EsifActSystemExit(void);

#endif	// _ESIF_UF_ACTION_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
