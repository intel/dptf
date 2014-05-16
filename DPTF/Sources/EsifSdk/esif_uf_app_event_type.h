/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

//#ifndef _ESIF_UF_APP_EVENT_TYPE_H_
//#define _ESIF_UF_APP_EVENT_TYPE_H_
//
//#include "esif.h"
//
///* App Event Type */
//
//typedef enum _t_eAppEventType {
//    /* App Events */
//    eEventAppConnectedStandbyEntry              = 0,    /* Enter connected standby */
//    eEventAppConnectedStandbyExit               = 1,    /* Exit connected standby */
//    eEventAppActiveRelationshipTableChanged     = 2,    /* Active relationship table changed */
//    eEventAppThermalRelationshipTableChanged    = 3,    /* Thermal relationship table changed */
//    eEventAppForegroundChanged                  = 4,    /* Foreground application changed */
//    eEventAppSuspend                            = 5,    /* Suspend Upper Framework */
//    eEventAppResume                             = 6,    /* Resume Upper Framework */
//
//    /* Domain Events */
//    eEventDomainCTDPCapabilityChanged           = 7,    /* Config TDP Capability changed (Configurable TDP) */
//    eEventDomainCoreCapabilityChanged           = 8,    /* For future use */
//    eEventDomainDisplayCapabilityChanged        = 9,    /* Display control upper/lower limits changed. */
//    eEventDomainDisplayStatusChanged            = 10,    /* Current Display brightness status has changed */
//    eEventDomainPerfCapabilityChanged           = 11,    /* Performance Control Upper/Lower Limits Changed */
//    eEventDomainPerfControlChanged              = 12,   /* For future use */
//    eEventDomainPowerCapabilityChanged          = 13,   /* Power Control Capability changed */
//    eEventDomainPowerThresholdCrossed           = 14,   /* programmable Threshold Power Event */
//    eEventDomainPriorityChanged                 = 15,   /* Domain priority has changed. */
//    eEventDomainTempThresholdCrossed            = 16,   /* Programmable Threshold Temp Event */
//
//    /* Participant Events */
//    eEventParticipantSpecificInfoChanged        = 17,   /* Participant Specific Information Changed. */
//    eEventMax                                   = 18    /* Max */
//} eEsifAppEventType;
//
//#endif /* _ESIF_UF_APP_EVENT_TYPE_H_ */