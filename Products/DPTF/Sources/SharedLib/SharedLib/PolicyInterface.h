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

#include "Dptf.h"
#include "PolicyServicesInterfaceContainer.h"
#include "esif.h"
#include <string>
#include "CoolingModeAcousticLimit.h"
#include "CoolingModePowerLimit.h"
#include "CoolingMode.h"
#include "SensorOrientation.h"
#include "SensorProximity.h"
#include "SensorSpatialOrientation.h"
#include "RadioConnectionStatus.h"

class dptf_export PolicyInterface
{
public:

    virtual ~PolicyInterface()
    {
    };

    //
    // This is the main entry point for bringing up a policy.  If a policy chooses not to load itself,
    // it should throw an exception.  In this case the policy will not get called again.
    //
    virtual void create(
        Bool enabled,
        PolicyServicesInterfaceContainer policyServicesInterfaceContainer,
        UIntN policyIndex) = 0;

    //
    // In response to this call, the policy must clean up all of its internal data structures.
    //
    virtual void destroy(void) = 0;

    //
    // This function is called by the DPTF framework whenever it receives a participant
    // ready notification for the first time.  No domain is available when bindParticipant is called.
    //
    virtual void bindParticipant(UIntN participantIndex) = 0;

    //
    // This function is called when the participant interface De-negotiation happens at
    // the DPTF framework level. meaning, the participant is being unloaded etc.
    //
    virtual void unbindParticipant(UIntN participantIndex) = 0;

    //
    // This function is called by the DPTF framework whenever it receives a new domain.
    //
    virtual void bindDomain(UIntN participantIndex, UIntN domainIndex) = 0;

    //
    // This function is called by the DPTF framework when a domain is being removed.
    //
    virtual void unbindDomain(UIntN participantIndex, UIntN domainIndex) = 0;

    //
    // This function is called if the framework wants to enable this specific policy.  This may
    // happen for debug and validation purposes.
    //
    virtual void enable(void) = 0;

    //
    // This function is called if the framework wants to disable this specific policy.  This may
    // happen for debug and validation purposes.
    //
    // When a policy is disabled, it should clear anything that it 'set', for example, the
    // aux trip points should be cleared and the fan turned off.
    //
    virtual void disable(void) = 0;

    virtual Guid getGuid(void) const = 0;
    virtual std::string getName(void) const = 0;
    virtual std::string getStatusAsXml(void) const = 0;

    // DPTF Event handlers
    virtual void connectedStandbyEntry(void) = 0;
    virtual void connectedStandbyExit(void) = 0;
    virtual void suspend(void) = 0;
    virtual void resume(void) = 0;

    // Participant/Domain Event Handlers
    virtual void domainConfigTdpCapabilityChanged(UIntN participantIndex) = 0;
    virtual void domainCoreControlCapabilityChanged(UIntN participantIndex) = 0;
    virtual void domainDisplayControlCapabilityChanged(UIntN participantIndex) = 0;
    virtual void domainDisplayStatusChanged(UIntN participantIndex) = 0;
    virtual void domainPerformanceControlCapabilityChanged(UIntN participantIndex) = 0;
    virtual void domainPerformanceControlsChanged(UIntN participantIndex) = 0;
    virtual void domainPowerControlCapabilityChanged(UIntN participantIndex) = 0;
    virtual void domainPriorityChanged(UIntN participantIndex) = 0;
    virtual void domainRadioConnectionStatusChanged(UIntN participantIndex,
        RadioConnectionStatus::Type radioConnectionStatus) = 0;
    virtual void domainRfProfileChanged(UIntN participantIndex) = 0;
    virtual void domainTemperatureThresholdCrossed(UIntN participantIndex) = 0;
    virtual void participantSpecificInfoChanged(UIntN participantIndex) = 0;

    // Policy Event Handlers
    virtual void activeRelationshipTableChanged(void) = 0;
    virtual void coolingModeAcousticLimitChanged(CoolingModeAcousticLimit::Type acousticLimit) = 0;
    virtual void coolingModePolicyChanged(CoolingMode::Type coolingMode) = 0;
    virtual void coolingModePowerLimitChanged(CoolingModePowerLimit::Type powerLimit) = 0;
    virtual void foregroundApplicationChanged(const std::string& foregroundApplicationName) = 0;
    virtual void policyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) = 0;
    virtual void operatingSystemConfigTdpLevelChanged(UIntN configTdpLevel) = 0;
    virtual void operatingSystemLpmModeChanged(UIntN lpmMode) = 0;
    virtual void passiveTableChanged(void) = 0;
    virtual void platformLpmModeChanged(void) = 0;
    virtual void sensorOrientationChanged(SensorOrientation::Type sensorOrientation) = 0;
    virtual void sensorProximityChanged(SensorProximity::Type sensorProximity) = 0;
    virtual void sensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation) = 0;
    virtual void thermalRelationshipTableChanged(void) = 0;
};

//
// The following functions must be exposed externally by the .dll/.so
//
extern "C"
{
    typedef PolicyInterface* (*CreatePolicyInstanceFuncPtr)(void);
    dptf_export PolicyInterface* CreatePolicyInstance(void);

    typedef void (*DestroyPolicyInstanceFuncPtr)(PolicyInterface* policyInterface);
    dptf_export void DestroyPolicyInstance(PolicyInterface* policyInterface);
}