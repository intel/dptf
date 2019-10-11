/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "Dptf.h"
#include "DomainActivityStatusInterface.h"
#include "DomainConfigTdpControlInterface.h"
#include "DomainCoreControlInterface.h"
#include "DomainDisplayControlInterface.h"
#include "DomainEnergyControlInterface.h"
#include "DomainPeakPowerControlInterface.h"
#include "DomainPerformanceControlInterface.h"
#include "DomainPowerControlInterface.h"
#include "DomainPowerStatusInterface.h"
#include "DomainPriorityInterface.h"
#include "DomainRfProfileControlInterface.h"
#include "DomainRfProfileStatusInterface.h"
#include "DomainUtilizationInterface.h"
#include "ParticipantGetSpecificInfoInterface.h"
#include "ParticipantPropertiesInterface.h"
#include "ParticipantSetSpecificInfoInterface.h"
#include "PlatformConfigurationDataInterface.h"
#include "PlatformPowerStateInterface.h"
#include "PolicyEventRegistrationInterface.h"
#include "PolicyInitiatedCallbackInterface.h"
#include "MessageLoggingInterface.h"
#include "PolicyMessage.h"
#include "PolicyWorkloadHintConfigurationInterface.h"
#include "DomainSystemPowerControlInterface.h"
#include "DomainPlatformPowerStatusInterface.h"
#include "PlatformStateInterface.h"
#include "DptfServiceRequestInterface.h"

struct PolicyServicesInterfaceContainer
{
	PolicyServicesInterfaceContainer(void);

	DomainActivityStatusInterface* domainActivityStatus;
	DomainConfigTdpControlInterface* domainConfigTdpControl;
	DomainCoreControlInterface* domainCoreControl;
	DomainDisplayControlInterface* domainDisplayControl;
	DomainEnergyControlInterface* domainEnergyControl;
	DomainPeakPowerControlInterface* domainPeakPowerControl;
	DomainPerformanceControlInterface* domainPerformanceControl;
	DomainPowerControlInterface* domainPowerControl;
	DomainPowerStatusInterface* domainPowerStatus;
	DomainSystemPowerControlInterface* domainSystemPowerControl;
	DomainPlatformPowerStatusInterface* domainPlatformPowerStatus;
	DomainPriorityInterface* domainPriority;
	DomainRfProfileControlInterface* domainRfProfileControl;
	DomainRfProfileStatusInterface* domainRfProfileStatus;
	DomainUtilizationInterface* domainUtilization;
	ParticipantGetSpecificInfoInterface* participantGetSpecificInfo;
	ParticipantPropertiesInterface* participantProperties;
	ParticipantSetSpecificInfoInterface* participantSetSpecificInfo;
	PlatformConfigurationDataInterface* platformConfigurationData;
	PlatformPowerStateInterface* platformPowerState;
	PolicyEventRegistrationInterface* policyEventRegistration;
	PolicyInitiatedCallbackInterface* policyInitiatedCallback;
	MessageLoggingInterface* messageLogging;
	PolicyWorkloadHintConfigurationInterface* workloadHintConfiguration;
	PlatformStateInterface* platformState;
	DptfServiceRequestInterface* serviceRequest;
};
