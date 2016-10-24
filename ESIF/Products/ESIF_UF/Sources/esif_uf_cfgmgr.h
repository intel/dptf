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

#ifndef _ESIF_UF_CFGMGR_
#define _ESIF_UF_CFGMGR_

#include "esif_uf.h"
#include "esif_lib_datacache.h"

#ifdef __cplusplus
extern "C" {
#endif

eEsifError EsifCfgMgrInit(void);
void EsifCfgMgrExit(void);

//////////////////////////
// Backwards Compatibility
//////////////////////////

/* Get */
eEsifError EsifConfigGet(EsifDataPtr nameSpace, EsifDataPtr path, EsifDataPtr value);
eEsifError EsifConfigGetItem(EsifDataPtr nameSpace, EsifDataPtr path, EsifDataPtr value, esif_flags_t *flagsPtr);

/* Set */
eEsifError EsifConfigSet(EsifDataPtr nameSpace, EsifDataPtr path, esif_flags_t flags, EsifDataPtr value);
eEsifError EsifConfigDelete(EsifDataPtr nameSpace, EsifDataPtr path);

/* Iterate Context type */
typedef esif_string EsifConfigFindContext, *EsifConfigFindContextPtr;

/* Iterate First */
eEsifError EsifConfigFindFirst(EsifDataPtr nameSpace, EsifDataPtr path, EsifDataPtr value, EsifConfigFindContextPtr context);

/* Iterate Next */
eEsifError EsifConfigFindNext(EsifDataPtr nameSpace, EsifDataPtr path, EsifDataPtr value, EsifConfigFindContextPtr context);

/* Iterate Close */
void EsifConfigFindClose(EsifConfigFindContextPtr context);

esif_flags_t EsifConfigFlags_Set(esif_flags_t bitmask, esif_string optname);

/* Copy/Merge */
eEsifError EsifConfigCopy(EsifDataPtr nameSpaceFrom, EsifDataPtr nameSpaceTo, EsifDataPtr keyspecs, esif_flags_t flags, Bool replaceKeys, UInt32 *keycount);

#ifdef __cplusplus
}
#endif

#endif /* _ESIF_UF_CFGMGR */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

