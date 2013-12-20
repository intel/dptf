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

#ifndef _ESIF_WS_VERSION_H_
#define _ESIF_WS_VERSION_H_

#define STR(arg) #arg
#define STR2(arg) STR(##arg)
#define EXPAND_TOSTR(arg) STR(arg)

#define ESIF_UF_VERSION_MAJOR 1
#define ESIF_UF_VERSION_MINOR 0
#define ESIF_UF_VERSION_HOTFIX 35
#define ESIF_UF_VERSION_BUILD 4

/* Build system will should set to 0 */
#define ESIF_SKU_TYPE 1

#if ESIF_SKU_TYPE == 0
#define ESIF_UF_VERSION EXPAND_TOSTR(ESIF_UF_VERSION_MAJOR.ESIF_UF_VERSION_MINOR.ESIF_UF_VERSION_HOTFIX.ESIF_UF_VERSION_BUILD)
#else
/* X denotes development */
#define ESIF_UF_VERSION EXPAND_TOSTR(X.ESIF_UF_VERSION_MAJOR.ESIF_UF_VERSION_MINOR.ESIF_UF_VERSION_HOTFIX.ESIF_UF_VERSION_BUILD)
#endif


#endif /* _ESIF_WS_VERSION_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
