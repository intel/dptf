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

#ifndef _ESIF_UF_CNJ_
#define _ESIF_UF_CNJ_

#include "esif.h"
#include "esif_uf_conjure_iface.h"

/* Conjure Manager Entry */
typedef struct _t_EsifCnj {
	void  *fHandle;				/* The Conjure Handle Opaque To Us */
	EsifConjureInterface  fInterface;			/* The Conjure Interface */
	EsifString fLibNamePtr;			/* The Name Of The Library To Load */
} EsifCnj, *EsifCnjPtr, **EsifCnjPtrLocation;

/* Init / Exit */
eEsifError EsifCnjInit (void);
void EsifCnjExit (void);

#endif	// _ESIF_UF_CNJ_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

