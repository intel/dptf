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
#include "Guid.h"
#include "SupportedPolicyList.h"
#include "PolicyInterface.h"
#include "EsifLibrary.h"
#include <bitset>

class DptfManager;

class Policy final
{
public:

    Policy(DptfManager* dptfManager);
    ~Policy(void);

    void createPolicy(const std::string& policyFileName, UIntN newPolicyIndex,
        const SupportedPolicyList& supportedPolicyList);
    void destroyPolicy(void);

    Guid getGuid(void);

    void bindParticipant(UIntN participantIndex);
    void unbindParticipant(UIntN participantIndex);
    void bindDomain(UIntN participantIndex, UIntN domainIndex);
    void unbindDomain(UIntN participantIndex, UIntN domainIndex);

    void enable(void);
    void disable(void);

    std::string getName(void) const;
    std::string getStatusAsXml(void) const;

    // Event handlers

    void executeConnectedStandbyEntry(void);
    void executeConnectedStandbyExit(void);
    void executeSuspend(void);
    void executeResume(void);
    void executeDomainConfigTdpCapabilityChanged(UIntN participantIndex);
    void executeDomainCoreControlCapabilityChanged(UIntN participantIndex);
    void executeDomainDisplayControlCapabilityChanged(UIntN participantIndex);
    void executeDomainDisplayStatusChanged(UIntN participantIndex);
    void executeDomainPerformanceControlCapabilityChanged(UIntN participantIndex);
    void executeDomainPerformanceControlsChanged(UIntN participantIndex);
    void executeDomainPowerControlCapabilityChanged(UIntN participantIndex);
    void executeDomainPriorityChanged(UIntN participantIndex);
    void executeDomainRadioConnectionStatusChanged(UIntN participantIndex, RadioConnectionStatus::Type radioConnectionStatus);
    void executeDomainRfProfileChanged(UIntN participantIndex);
    void executeDomainTemperatureThresholdCrossed(UIntN participantIndex);
    void executeParticipantSpecificInfoChanged(UIntN participantIndex);
    void executePolicyActiveRelationshipTableChanged(void);
    void executePolicyCoolingModeAcousticLimitChanged(CoolingModeAcousticLimit::Type acousticLimit);
    void executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode);
    void executePolicyCoolingModePowerLimitChanged(CoolingModePowerLimit::Type powerLimit);
    void executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName);
    void executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2);
    void executePolicyOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel);
    void executePolicyOperatingSystemLpmModeChanged(UIntN lpmMode);
    void executePolicyPassiveTableChanged(void);
    void executePolicyPlatformLpmModeChanged(void);
    void executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation);
    void executePolicySensorProximityChanged(SensorProximity::Type sensorProximity);
    void executePolicySensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation);
    void executePolicyThermalRelationshipTableChanged(void);

private:

    // hide the copy constructor and assignment operator.
    Policy(const Policy& rhs);
    Policy& operator=(const Policy& rhs);

    // Pointer to the DPTF manager class which has access to most of the manager components.
    DptfManager* m_dptfManager;

    // The instance returned when we called CreatePolicyInstance on the .dll/.so
    PolicyInterface* m_theRealPolicy;
    Bool m_theRealPolicyCreated;

    // The index assigned by the policy manager
    UIntN m_policyIndex;

    // The guid retrieved when the file was loaded
    Guid m_guid;

    // The name of the policy.
    std::string m_policyName;

    // full path to the file that contains this policy
    std::string m_policyFileName;

    // functionality to load/unload .dll/.so and retrieve function pointers
    EsifLibrary* m_esifLibrary;

    // function pointers exposed in the .dll/.so
    CreatePolicyInstanceFuncPtr m_createPolicyInstanceFuncPtr;
    DestroyPolicyInstanceFuncPtr m_destroyPolicyInstanceFuncPtr;

    // Policy services should be used as an interface instead of an interface container.
    // Because of this design decision the code is split between the shared lib and
    // the manager which is why the following functions are here.  This should be improved
    // in the future.
    PolicyServicesInterfaceContainer m_policyServices;
    void createPolicyServices(void);
    void destroyPolicyServices(void);

    // track the events that will be forwarded to the policy
    std::bitset<PolicyEvent::Max> m_registeredEvents;

    // Event Registration.  This is private as the policy manager must handle registering and unregistering
    // the event with ESIF.  This is necessary since multiple policies may register for an event such as
    // PolicyEvent::PolicyForegroundApplicationChanged.  We can't have two policies register for this event and
    // one go away and unregister for both of them.
    friend class PolicyManager;
    void registerEvent(PolicyEvent::Type policyEvent);
    void unregisterEvent(PolicyEvent::Type policyEvent);
    Bool isEventRegistered(PolicyEvent::Type policyEvent);
};