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

#ifndef _ESIF_UF_CFGMGR_
#define _ESIF_UF_CFGMGR_

#include "esif_uf.h"

eEsifError EsifCfgMgrInit(void);
void EsifCfgMgrExit(void);

//////////////////////////
// Backwards Compatibility
//////////////////////////

eEsifError EsifConfigInit(esif_string name);
void EsifConfigExit(esif_string name);

/* Get */
eEsifError EsifConfigGet(EsifDataPtr nameSpace, EsifDataPtr path, EsifDataPtr value);

/* Set */
eEsifError EsifConfigSet(EsifDataPtr nameSpace, EsifDataPtr path, esif_flags_t flags, EsifDataPtr value);

/* Iterate First */
eEsifError EsifConfigFindFirst(EsifDataPtr nameSpace, EsifDataPtr path, EsifDataPtr value, UInt32 *context);

/* Iterate Next */
eEsifError EsifConfigFindNext(EsifDataPtr nameSpace, EsifDataPtr path, EsifDataPtr value, UInt32 *context);

esif_flags_t EsifConfigFlags_Set(esif_flags_t bitmask, esif_string optname);


#endif /* _ESIF_UF_CFGMGR */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

