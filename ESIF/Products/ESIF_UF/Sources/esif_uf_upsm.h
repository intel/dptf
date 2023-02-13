/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "esif_ccb_rc.h"

#ifdef __cplusplus
extern "C" {
#endif
	esif_error_t EsifUpsm_Init(void);
	void EsifUpsm_Exit(void);

	esif_error_t EsifUpsm_Start(void);
	void EsifUpsm_Stop(void);

	esif_error_t EsifUpsm_Enable(void);
	void EsifUpsm_Disable(void);

#ifdef __cplusplus
}
#endif


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

