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

#include "DptfManager.h"
#include "EsifServices.h"
#include "Policy.h"
#include "PolicyServicesInterfaceContainer.h"
#include "PolicyServicesDomainActiveControl.h"
#include "PolicyServicesDomainConfigTdpControl.h"
#include "PolicyServicesDomainCoreControl.h"
#include "PolicyServicesDomainDisplayControl.h"
#include "PolicyServicesDomainPerformanceControl.h"
#include "PolicyServicesDomainPixelClockControl.h"
#include "PolicyServicesDomainPixelClockStatus.h"
#include "PolicyServicesDomainPowerControl.h"
#include "PolicyServicesDomainPowerStatus.h"
#include "PolicyServicesDomainPriority.h"
#include "PolicyServicesDomainRfProfileControl.h"
#include "PolicyServicesDomainRfProfileStatus.h"
#include "PolicyServicesDomainTemperature.h"
#include "PolicyServicesDomainUtilization.h"
#include "PolicyServicesParticipantGetSpecificInfo.h"
#include "PolicyServicesParticipantProperties.h"
#include "PolicyServicesParticipantSetSpecificInfo.h"
#include "PolicyServicesPlatformConfigurationData.h"
#include "PolicyServicesPlatformNotification.h"
#include "PolicyServicesPlatformPowerState.h"
#include "PolicyServicesPolicyEventRegistration.h"
#include "PolicyServicesPolicyInitiatedCallback.h"
#include "PolicyServicesMessageLogging.h"

Policy::Policy(DptfManager* dptfManager) : m_dptfManager(dptfManager), m_theRealPolicy(nullptr),
    m_theRealPolicyCreated(false), m_policyIndex(Constants::Invalid), m_esifLibrary(nullptr),
    m_createPolicyInstanceFuncPtr(nullptr), m_destroyPolicyInstanceFuncPtr(nullptr)
{
}

Policy::~Policy(void)
{
}

void Policy::createPolicy(const std::string& policyFileName, UIntN newPolicyIndex,
    const SupportedPolicyList& supportedPolicyList)
{
    // If an exception is thrown while trying to create the policy, the PolicyManager will
    // delete the Policy instance and remove the policy completely.

    m_policyFileName = policyFileName;
    m_policyIndex = newPolicyIndex;

    // Load the .dll/.so.  If it can't load the library it will throw an exception and we're done.
    m_esifLibrary = new EsifLibrary(m_policyFileName);
    m_esifLibrary->load();

    // Make sure all of the required 'C' functions are exposed.  If there is a problem getting the function pointer
    // an exception is thrown by the EsifLibrary class.
    m_createPolicyInstanceFuncPtr = (CreatePolicyInstanceFuncPtr)m_esifLibrary->getFunctionPtr("CreatePolicyInstance");
    m_destroyPolicyInstanceFuncPtr = (DestroyPolicyInstanceFuncPtr)m_esifLibrary->getFunctionPtr("DestroyPolicyInstance");

    // Call the function that is exposed in the .dll/.so and ask it to create an instance of the class
    m_theRealPolicy = m_createPolicyInstanceFuncPtr();
    if (m_theRealPolicy == nullptr)
    {
        std::stringstream message;
        message << "Error while trying to create the policy instance: " << m_policyFileName;
        throw dptf_exception(message.str());
    }

    // Now we need to check the guid provided by the policy and make sure it is in the list of policies that are to
    // be loaded.

    m_guid = m_theRealPolicy->getGuid();

    Bool policySupported = supportedPolicyList.isPolicyValid(m_guid);
    if (policySupported == false)
    {
        std::stringstream message;
        message << "Policy [" << m_policyFileName << "] will not be loaded.  GUID not in supported policy list [" << m_guid << "]";
        m_dptfManager->getEsifServices()->writeMessageWarning(message.str());
        throw policy_not_in_idsp_list();
    }

    // create all of the classes necessary to fill in the policy services interface container
    createPolicyServices();

    m_theRealPolicy->create(true, m_policyServices, newPolicyIndex);
    m_theRealPolicyCreated = true;

    m_policyName = m_theRealPolicy->getName();
}

void Policy::destroyPolicy()
{
    try
    {
        if ((m_theRealPolicy != nullptr) &&
            (m_theRealPolicyCreated == true))
        {
            m_theRealPolicy->destroy();
        }
    }
    catch (...)
    {
    }

    destroyPolicyServices();

    // Call the function that is exposed in the .dll/.so and ask it to destroy the instance of the class
    if ((m_destroyPolicyInstanceFuncPtr != nullptr) &&
        (m_theRealPolicy != nullptr))
    {
        m_destroyPolicyInstanceFuncPtr(m_theRealPolicy);
    }

    if (m_esifLibrary != nullptr)
    {
        m_esifLibrary->unload();
    }

    DELETE_MEMORY_TC(m_esifLibrary);
}

Guid Policy::getGuid(void)
{
    return m_guid;
}

void Policy::bindParticipant(UIntN participantIndex)
{
    m_theRealPolicy->bindParticipant(participantIndex);
}

void Policy::unbindParticipant(UIntN participantIndex)
{
    m_theRealPolicy->unbindParticipant(participantIndex);
}

void Policy::bindDomain(UIntN participantIndex, UIntN domainIndex)
{
    m_theRealPolicy->bindDomain(participantIndex, domainIndex);
}

void Policy::unbindDomain(UIntN participantIndex, UIntN domainIndex)
{
    m_theRealPolicy->unbindDomain(participantIndex, domainIndex);
}

void Policy::enable(void)
{
    m_theRealPolicy->enable();
}

void Policy::disable(void)
{
    m_theRealPolicy->disable();
}

std::string Policy::getName(void) const
{
    return m_policyName;
}

std::string Policy::getStatusAsXml(void) const
{
    return m_theRealPolicy->getStatusAsXml();
}

void Policy::executeConnectedStandbyEntry(void)
{
    if (isEventRegistered(PolicyEvent::DptfConnectedStandbyEntry))
    {
        m_theRealPolicy->connectedStandbyEntry();
    }
}

void Policy::executeConnectedStandbyExit(void)
{
    if (isEventRegistered(PolicyEvent::DptfConnectedStandbyExit))
    {
        m_theRealPolicy->connectedStandbyExit();
    }
}

void Policy::executeSuspend(void)
{
    if (isEventRegistered(PolicyEvent::DptfSuspend))
    {
        m_theRealPolicy->suspend();
    }
}

void Policy::executeResume(void)
{
    if (isEventRegistered(PolicyEvent::DptfResume))
    {
        m_theRealPolicy->resume();
    }
}

void Policy::executeDomainConfigTdpCapabilityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainConfigTdpCapabilityChanged))
    {
        m_theRealPolicy->domainConfigTdpCapabilityChanged(participantIndex);
    }
}

void Policy::executeDomainCoreControlCapabilityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainCoreControlCapabilityChanged))
    {
        m_theRealPolicy->domainCoreControlCapabilityChanged(participantIndex);
    }
}

void Policy::executeDomainDisplayControlCapabilityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainDisplayControlCapabilityChanged))
    {
        m_theRealPolicy->domainDisplayControlCapabilityChanged(participantIndex);
    }
}

void Policy::executeDomainDisplayStatusChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainDisplayStatusChanged))
    {
        m_theRealPolicy->domainDisplayStatusChanged(participantIndex);
    }
}

void Policy::executeDomainPerformanceControlCapabilityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainPerformanceControlCapabilityChanged))
    {
        m_theRealPolicy->domainPerformanceControlCapabilityChanged(participantIndex);
    }
}

void Policy::executeDomainPerformanceControlsChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainPerformanceControlsChanged))
    {
        m_theRealPolicy->domainPerformanceControlsChanged(participantIndex);
    }
}

void Policy::executeDomainPowerControlCapabilityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainPowerControlCapabilityChanged))
    {
        m_theRealPolicy->domainPowerControlCapabilityChanged(participantIndex);
    }
}

void Policy::executeDomainPriorityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainPriorityChanged))
    {
        m_theRealPolicy->domainPriorityChanged(participantIndex);
    }
}

void Policy::executeDomainRadioConnectionStatusChanged(UIntN participantIndex,
    RadioConnectionStatus::Type radioConnectionStatus)
{
    if (isEventRegistered(PolicyEvent::DomainRadioConnectionStatusChanged))
    {
        m_theRealPolicy->domainRadioConnectionStatusChanged(participantIndex, radioConnectionStatus);
    }
}

void Policy::executeDomainRfProfileChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainRfProfileChanged))
    {
        m_theRealPolicy->domainRfProfileChanged(participantIndex);
    }
}

void Policy::executeDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainTemperatureThresholdCrossed))
    {
        m_theRealPolicy->domainTemperatureThresholdCrossed(participantIndex);
    }
}

void Policy::executeParticipantSpecificInfoChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::ParticipantSpecificInfoChanged))
    {
        m_theRealPolicy->participantSpecificInfoChanged(participantIndex);
    }
}

void Policy::executePolicyActiveRelationshipTableChanged(void)
{
    if (isEventRegistered(PolicyEvent::PolicyActiveRelationshipTableChanged))
    {
        m_theRealPolicy->activeRelationshipTableChanged();
    }
}

void Policy::executePolicyCoolingModeAcousticLimitChanged(CoolingModeAcousticLimit::Type acousticLimit)
{
    if (isEventRegistered(PolicyEvent::PolicyCoolingModeAcousticLimitChanged))
    {
        m_theRealPolicy->coolingModeAcousticLimitChanged(acousticLimit);
    }
}

void Policy::executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode)
{
    if (isEventRegistered(PolicyEvent::PolicyCoolingModePolicyChanged))
    {
        m_theRealPolicy->coolingModePolicyChanged(coolingMode);
    }
}

void Policy::executePolicyCoolingModePowerLimitChanged(CoolingModePowerLimit::Type powerLimit)
{
    if (isEventRegistered(PolicyEvent::PolicyCoolingModePowerLimitChanged))
    {
        m_theRealPolicy->coolingModePowerLimitChanged(powerLimit);
    }
}

void Policy::executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName)
{
    if (isEventRegistered(PolicyEvent::PolicyForegroundApplicationChanged))
    {
        m_theRealPolicy->foregroundApplicationChanged(foregroundApplicationName);
    }
}

void Policy::executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2)
{
    m_theRealPolicy->policyInitiatedCallback(policyDefinedEventCode, param1, param2);
}

void Policy::executePolicyOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel)
{
    if (isEventRegistered(PolicyEvent::PolicyOperatingSystemConfigTdpLevelChanged))
    {
        m_theRealPolicy->operatingSystemConfigTdpLevelChanged(configTdpLevel);
    }
}

void Policy::executePolicyOperatingSystemLpmModeChanged(UIntN lpmMode)
{
    if (isEventRegistered(PolicyEvent::PolicyOperatingSystemLpmModeChanged))
    {
        m_theRealPolicy->operatingSystemLpmModeChanged(lpmMode);
    }
}

void Policy::executePolicyPassiveTableChanged(void)
{
    if (isEventRegistered(PolicyEvent::PolicyPassiveTableChanged))
    {
        m_theRealPolicy->passiveTableChanged();
    }
}

void Policy::executePolicyPlatformLpmModeChanged()
{
    if (isEventRegistered(PolicyEvent::PolicyPlatformLpmModeChanged))
    {
        m_theRealPolicy->platformLpmModeChanged();
    }
}

void Policy::executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation)
{
    if (isEventRegistered(PolicyEvent::PolicySensorOrientationChanged))
    {
        m_theRealPolicy->sensorOrientationChanged(sensorOrientation);
    }
}

void Policy::executePolicySensorProximityChanged(SensorProximity::Type sensorProximity)
{
    if (isEventRegistered(PolicyEvent::PolicySensorProximityChanged))
    {
        m_theRealPolicy->sensorProximityChanged(sensorProximity);
    }
}

void Policy::executePolicySensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation)
{
    if (isEventRegistered(PolicyEvent::PolicySensorSpatialOrientationChanged))
    {
        m_theRealPolicy->sensorSpatialOrientationChanged(sensorSpatialOrientation);
    }
}

void Policy::executePolicyThermalRelationshipTableChanged(void)
{
    if (isEventRegistered(PolicyEvent::PolicyThermalRelationshipTableChanged))
    {
        m_theRealPolicy->thermalRelationshipTableChanged();
    }
}

void Policy::registerEvent(PolicyEvent::Type policyEvent)
{
    m_registeredEvents.set(policyEvent);
}

void Policy::unregisterEvent(PolicyEvent::Type policyEvent)
{
    m_registeredEvents.reset(policyEvent);
}

Bool Policy::isEventRegistered(PolicyEvent::Type policyEvent)
{
    return m_registeredEvents.test(policyEvent);
}

void Policy::createPolicyServices(void)
{
    m_policyServices.domainActiveControl = new PolicyServicesDomainActiveControl(m_dptfManager, m_policyIndex);
    m_policyServices.domainConfigTdpControl = new PolicyServicesDomainConfigTdpControl(m_dptfManager, m_policyIndex);
    m_policyServices.domainCoreControl = new PolicyServicesDomainCoreControl(m_dptfManager, m_policyIndex);
    m_policyServices.domainDisplayControl = new PolicyServicesDomainDisplayControl(m_dptfManager, m_policyIndex);
    m_policyServices.domainPerformanceControl = new PolicyServicesDomainPerformanceControl(m_dptfManager, m_policyIndex);
    m_policyServices.domainPixelClockControl = new PolicyServicesDomainPixelClockControl(m_dptfManager, m_policyIndex);
    m_policyServices.domainPixelClockStatus = new PolicyServicesDomainPixelClockStatus(m_dptfManager, m_policyIndex);
    m_policyServices.domainPowerControl = new PolicyServicesDomainPowerControl(m_dptfManager, m_policyIndex);
    m_policyServices.domainPowerStatus = new PolicyServicesDomainPowerStatus(m_dptfManager, m_policyIndex);
    m_policyServices.domainPriority = new PolicyServicesDomainPriority(m_dptfManager, m_policyIndex);
    m_policyServices.domainRfProfileControl = new PolicyServicesDomainRfProfileControl(m_dptfManager, m_policyIndex);
    m_policyServices.domainRfProfileStatus = new PolicyServicesDomainRfProfileStatus(m_dptfManager, m_policyIndex);
    m_policyServices.domainTemperature = new PolicyServicesDomainTemperature(m_dptfManager, m_policyIndex);
    m_policyServices.domainUtilization = new PolicyServicesDomainUtilization(m_dptfManager, m_policyIndex);
    m_policyServices.participantGetSpecificInfo = new PolicyServicesParticipantGetSpecificInfo(m_dptfManager, m_policyIndex);
    m_policyServices.participantProperties = new PolicyServicesParticipantProperties(m_dptfManager, m_policyIndex);
    m_policyServices.participantSetSpecificInfo = new PolicyServicesParticipantSetSpecificInfo(m_dptfManager, m_policyIndex);
    m_policyServices.platformConfigurationData = new PolicyServicesPlatformConfigurationData(m_dptfManager, m_policyIndex);
    m_policyServices.platformNotification = new PolicyServicesPlatformNotification(m_dptfManager, m_policyIndex);
    m_policyServices.platformPowerState = new PolicyServicesPlatformPowerState(m_dptfManager, m_policyIndex);
    m_policyServices.policyEventRegistration = new PolicyServicesPolicyEventRegistration(m_dptfManager, m_policyIndex);
    m_policyServices.policyInitiatedCallback = new PolicyServicesPolicyInitiatedCallback(m_dptfManager, m_policyIndex);
    m_policyServices.messageLogging = new PolicyServicesMessageLogging(m_dptfManager, m_policyIndex);
}

void Policy::destroyPolicyServices(void)
{
    DELETE_MEMORY_TC(m_policyServices.domainActiveControl);
    DELETE_MEMORY_TC(m_policyServices.domainConfigTdpControl);
    DELETE_MEMORY_TC(m_policyServices.domainCoreControl);
    DELETE_MEMORY_TC(m_policyServices.domainDisplayControl);
    DELETE_MEMORY_TC(m_policyServices.domainPerformanceControl);
    DELETE_MEMORY_TC(m_policyServices.domainPixelClockControl);
    DELETE_MEMORY_TC(m_policyServices.domainPixelClockStatus);
    DELETE_MEMORY_TC(m_policyServices.domainPowerControl);
    DELETE_MEMORY_TC(m_policyServices.domainPowerStatus);
    DELETE_MEMORY_TC(m_policyServices.domainPriority);
    DELETE_MEMORY_TC(m_policyServices.domainRfProfileControl);
    DELETE_MEMORY_TC(m_policyServices.domainRfProfileStatus);
    DELETE_MEMORY_TC(m_policyServices.domainTemperature);
    DELETE_MEMORY_TC(m_policyServices.domainUtilization);
    DELETE_MEMORY_TC(m_policyServices.participantGetSpecificInfo);
    DELETE_MEMORY_TC(m_policyServices.participantProperties);
    DELETE_MEMORY_TC(m_policyServices.participantSetSpecificInfo);
    DELETE_MEMORY_TC(m_policyServices.platformConfigurationData);
    DELETE_MEMORY_TC(m_policyServices.platformNotification);
    DELETE_MEMORY_TC(m_policyServices.platformPowerState);
    DELETE_MEMORY_TC(m_policyServices.policyEventRegistration);
    DELETE_MEMORY_TC(m_policyServices.policyInitiatedCallback);
    DELETE_MEMORY_TC(m_policyServices.messageLogging);
}