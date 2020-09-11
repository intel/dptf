/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_SENSOR_MANAGER_LIN_
#define _ESIF_UF_SENSOR_MANAGER_LIN_

#include "esif_event.h"
#ifdef __cplusplus
extern "C" {
#endif

void EsifSensorMgr_Init();
void EsifSensorMgr_Exit();

eEsifError esif_register_sensor_lin(eEsifEventType eventType);
eEsifError esif_unregister_sensor_lin(eEsifEventType eventType);
eEsifError register_for_system_metric_notification_lin(esif_guid_t *guid);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_SENSOR_MANAGER_LIN_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

