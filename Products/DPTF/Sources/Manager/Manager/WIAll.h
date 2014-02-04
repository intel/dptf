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

#include "WIDomainAllocate.h"
#include "WIDomainConfigTdpCapabilityChanged.h"
#include "WIDomainCoreControlCapabilityChanged.h"
#include "WIDomainCreate.h"
#include "WIDomainDestroy.h"
#include "WIDomainDisplayControlCapabilityChanged.h"
#include "WIDomainDisplayStatusChanged.h"
#include "WIDomainPerformanceControlCapabilityChanged.h"
#include "WIDomainPerformanceControlsChanged.h"
#include "WIDomainPowerControlCapabilityChanged.h"
#include "WIDomainPriorityChanged.h"
#include "WIDomainRadioConnectionStatusChanged.h"
#include "WIDomainRfProfileChanged.h"
#include "WIDomainTemperatureThresholdCrossed.h"
#include "WIDptfConnectedStandbyEntry.h"
#include "WIDptfConnectedStandbyExit.h"
#include "WIDptfGetStatus.h"
#include "WIParticipantAllocate.h"
#include "WIParticipantCreate.h"
#include "WIParticipantDestroy.h"
#include "WIParticipantSpecificInfoChanged.h"
#include "WIPolicyActiveRelationshipTableChanged.h"
#include "WIPolicyCoolingModeAcousticLimitChanged.h"
#include "WIPolicyCoolingModePolicyChanged.h"
#include "WIPolicyCoolingModePowerLimitChanged.h"
#include "WIPolicyCreate.h"
#include "WIPolicyDestroy.h"
#include "WIPolicyForegroundApplicationChanged.h"
#include "WIPolicyInitiatedCallback.h"
#include "WIPolicyOperatingSystemConfigTdpLevelChanged.h"
#include "WIPolicyOperatingSystemLpmModeChanged.h"
#include "WIPolicyPassiveTableChanged.h"
#include "WIPolicyPlatformLpmModeChanged.h"
#include "WIPolicySensorOrientationChanged.h"
#include "WIPolicySensorProximityChanged.h"
#include "WIPolicySensorSpatialOrientationChanged.h"
#include "WIPolicyThermalRelationshipTableChanged.h"