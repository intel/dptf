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
#include "DptfManager.h"
#include "Policy.h"
#include "PolicyServicesInterfaceContainer.h"
#include "PolicyServicesDomainActiveControl.h"
#include "PolicyServicesDomainConfigTdpControl.h"
#include "PolicyServicesDomainCoreControl.h"
#include "PolicyServicesDomainDisplayControl.h"
#include "PolicyServicesDomainPerformanceControl.h"
#include "PolicyServicesDomainPowerControl.h"
#include "PolicyServicesDomainPowerStatus.h"
#include "PolicyServicesDomainPriority.h"
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
    m_policyIndex(Constants::Invalid), m_esifLibrary(nullptr),
    m_createPolicyInstanceFuncPtr(nullptr), m_destroyPolicyInstanceFuncPtr(nullptr)
{
}

Policy::~Policy(void)
{
}

void Policy::createPolicy(const std::string& policyFileName, UIntN newPolicyIndex,
    const SupportedPolicyList& supportedPolicyList)
{
    //
    // FIXME:  clean this up...
    //

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

    //throw implement_me();
    //FIXME:  for initial development skip this check for the guid
    //Bool policySupported = supportedPolicyList.isPolicyValid(m_guid);
    //if (policySupported == false)
    //{
    //    throw dptf_exception(".....
    //}

    // FIXME:  move this to class factory
    PolicyServicesInterfaceContainer policyServices;
    policyServices.domainActiveControl = new PolicyServicesDomainActiveControl(m_dptfManager, m_policyIndex);
    policyServices.domainConfigTdpControl = new PolicyServicesDomainConfigTdpControl(m_dptfManager, m_policyIndex);
    policyServices.domainCoreControl = new PolicyServicesDomainCoreControl(m_dptfManager, m_policyIndex);
    policyServices.domainDisplayControl = new PolicyServicesDomainDisplayControl(m_dptfManager, m_policyIndex);
    policyServices.domainPerformanceControl = new PolicyServicesDomainPerformanceControl(m_dptfManager, m_policyIndex);
    policyServices.domainPowerControl = new PolicyServicesDomainPowerControl(m_dptfManager, m_policyIndex);
    policyServices.domainPowerStatus = new PolicyServicesDomainPowerStatus(m_dptfManager, m_policyIndex);
    policyServices.domainPriority = new PolicyServicesDomainPriority(m_dptfManager, m_policyIndex);
    policyServices.domainTemperature = new PolicyServicesDomainTemperature(m_dptfManager, m_policyIndex);
    policyServices.domainUtilization = new PolicyServicesDomainUtilization(m_dptfManager, m_policyIndex);
    policyServices.participantGetSpecificInfo = new PolicyServicesParticipantGetSpecificInfo(m_dptfManager, m_policyIndex);
    policyServices.participantProperties = new PolicyServicesParticipantProperties(m_dptfManager, m_policyIndex);
    policyServices.participantSetSpecificInfo = new PolicyServicesParticipantSetSpecificInfo(m_dptfManager, m_policyIndex);
    policyServices.platformConfigurationData = new PolicyServicesPlatformConfigurationData(m_dptfManager, m_policyIndex);
    policyServices.platformNotification = new PolicyServicesPlatformNotification(m_dptfManager, m_policyIndex);
    policyServices.platformPowerState = new PolicyServicesPlatformPowerState(m_dptfManager, m_policyIndex);
    policyServices.policyEventRegistration = new PolicyServicesPolicyEventRegistration(m_dptfManager, m_policyIndex);
    policyServices.policyInitiatedCallback = new PolicyServicesPolicyInitiatedCallback(m_dptfManager, m_policyIndex);
    policyServices.messageLogging = new PolicyServicesMessageLogging(m_dptfManager, m_policyIndex);

    m_theRealPolicy->create(true, policyServices, newPolicyIndex);

    m_policyName = m_theRealPolicy->getName();
}

void Policy::destroyPolicy()
{
    try
    {
        m_theRealPolicy->destroy();
    }
    catch (...)
    {
    }

    // Call the function that is exposed in the .dll/.so and ask it to destroy the instance of the class
    m_destroyPolicyInstanceFuncPtr(m_theRealPolicy);
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

void Policy::executeDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainTemperatureThresholdCrossed))
    {
        m_theRealPolicy->domainTemperatureThresholdCrossed(participantIndex);
    }
}

void Policy::executeDomainPowerControlCapabilityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainPowerControlCapabilityChanged))
    {
        m_theRealPolicy->domainPowerControlCapabilityChanged(participantIndex);
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

void Policy::executeDomainCoreControlCapabilityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainCoreControlCapabilityChanged))
    {
        m_theRealPolicy->domainCoreControlCapabilityChanged(participantIndex);
    }
}

void Policy::executeDomainConfigTdpCapabilityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainConfigTdpCapabilityChanged))
    {
        m_theRealPolicy->domainConfigTdpCapabilityChanged(participantIndex);
    }
}

void Policy::executeDomainPriorityChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::DomainPriorityChanged))
    {
        m_theRealPolicy->domainPriorityChanged(participantIndex);
    }
}

void Policy::executeParticipantSpecificInfoChanged(UIntN participantIndex)
{
    if (isEventRegistered(PolicyEvent::ParticipantSpecificInfoChanged))
    {
        m_theRealPolicy->participantSpecificInfoChanged(participantIndex);
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

void Policy::executePolicyActiveRelationshipTableChanged(void)
{
    if (isEventRegistered(PolicyEvent::PolicyActiveRelationshipTableChanged))
    {
        m_theRealPolicy->activeRelationshipTableChanged();
    }
}

void Policy::executePolicyThermalRelationshipTableChanged(void)
{
    if (isEventRegistered(PolicyEvent::PolicyThermalRelationshipTableChanged))
    {
        m_theRealPolicy->thermalRelationshipTableChanged();
    }
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

void Policy::executePolicyOperatingSystemLpmModeChanged(UIntN lpmMode)
{
    if (isEventRegistered(PolicyEvent::PolicyOperatingSystemLpmModeChanged))
    {
        m_theRealPolicy->operatingSystemLpmModeChanged(lpmMode);
    }
}

void Policy::executePolicyPlatformLpmModeChanged()
{
    if (isEventRegistered(PolicyEvent::PolicyPlatformLpmModeChanged))
    {
        m_theRealPolicy->platformLpmModeChanged();
    }
}

void Policy::executePolicyOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel)
{
    if (isEventRegistered(PolicyEvent::PolicyOperatingSystemConfigTdpLevelChanged))
    {
        m_theRealPolicy->operatingSystemConfigTdpLevelChanged(configTdpLevel);
    }
}

void Policy::executePolicyCoolingModePowerLimitChanged(CoolingModePowerLimit::Type powerLimit)
{
    if (isEventRegistered(PolicyEvent::PolicyCoolingModePowerLimitChanged))
    {
        m_theRealPolicy->coolingModePowerLimitChanged(powerLimit);
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

void Policy::executePolicyPassiveTableChanged(void)
{
    if (isEventRegistered(PolicyEvent::PolicyPassiveTableChanged))
    {
        m_theRealPolicy->passiveTableChanged();
    }
}

void Policy::executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation)
{
    if (isEventRegistered(PolicyEvent::PolicySensorOrientationChanged))
    {
        m_theRealPolicy->sensorOrientationChanged(sensorOrientation);
    }
}

void Policy::executePolicySensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation)
{
    if (isEventRegistered(PolicyEvent::PolicySensorSpatialOrientationChanged))
    {
        m_theRealPolicy->sensorSpatialOrientationChanged(sensorSpatialOrientation);
    }
}

void Policy::executePolicySensorProximityChanged(SensorProximity::Type sensorProximity)
{
    if (isEventRegistered(PolicyEvent::PolicySensorProximityChanged))
    {
        m_theRealPolicy->sensorProximityChanged(sensorProximity);
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