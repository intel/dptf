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

#pragma once

#include "DomainActiveControlInterface.h"
#include "DomainConfigTdpControlInterface.h"
#include "DomainCoreControlInterface.h"
#include "DomainDisplayControlInterface.h"
#include "DomainPerformanceControlInterface.h"
#include "DomainPixelClockControlInterface.h"
#include "DomainPixelClockStatusInterface.h"
#include "DomainPowerControlInterface.h"
#include "DomainPowerStatusInterface.h"
#include "DomainPriorityInterface.h"
#include "DomainRfProfileControlInterface.h"
#include "DomainRfProfileStatusInterface.h"
#include "DomainTemperatureInterface.h"
#include "DomainUtilizationInterface.h"
#include "ParticipantGetSpecificInfoInterface.h"
#include "ParticipantPropertiesInterface.h"
#include "ParticipantSetSpecificInfoInterface.h"
#include "PlatformConfigurationDataInterface.h"
#include "PlatformNotificationInterface.h"
#include "PlatformPowerStateInterface.h"
#include "PolicyEventRegistrationInterface.h"
#include "PolicyInitiatedCallbackInterface.h"
#include "MessageLoggingInterface.h"
#include "PolicyMessage.h"

struct PolicyServicesInterfaceContainer
{
    PolicyServicesInterfaceContainer(void);

    DomainActiveControlInterface* domainActiveControl;
    DomainConfigTdpControlInterface* domainConfigTdpControl;
    DomainCoreControlInterface* domainCoreControl;
    DomainDisplayControlInterface* domainDisplayControl;
    DomainPerformanceControlInterface* domainPerformanceControl;
    DomainPixelClockControlInterface* domainPixelClockControl;
    DomainPixelClockStatusInterface* domainPixelClockStatus;
    DomainPowerControlInterface* domainPowerControl;
    DomainPowerStatusInterface* domainPowerStatus;
    DomainPriorityInterface* domainPriority;
    DomainRfProfileControlInterface* domainRfProfileControl;
    DomainRfProfileStatusInterface* domainRfProfileStatus;
    DomainTemperatureInterface* domainTemperature;
    DomainUtilizationInterface* domainUtilization;
    ParticipantGetSpecificInfoInterface* participantGetSpecificInfo;
    ParticipantPropertiesInterface* participantProperties;
    ParticipantSetSpecificInfoInterface* participantSetSpecificInfo;
    PlatformConfigurationDataInterface* platformConfigurationData;
    PlatformNotificationInterface* platformNotification;
    PlatformPowerStateInterface* platformPowerState;
    PolicyEventRegistrationInterface* policyEventRegistration;
    PolicyInitiatedCallbackInterface* policyInitiatedCallback;
    MessageLoggingInterface* messageLogging;
};