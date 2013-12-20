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
#pragma once

#include "Dptf.h"
#include "ClassFactories.h"
#include "ParticipantServicesInterface.h"
#include "ComponentExtendedInterface.h"
#include "Guid.h"
#include "DomainType.h"
#include "DomainFunctionalityVersions.h"
#include "DomainActiveControlFactory.h"
#include "DomainConfigTdpControlFactory.h"
#include "DomainCoreControlFactory.h"
#include "DomainDisplayControlFactory.h"
#include "DomainPerformanceControlFactory.h"
#include "DomainPowerControlFactory.h"
#include "DomainPowerStatusFactory.h"
#include "DomainPriorityFactory.h"
#include "DomainTemperatureFactory.h"
#include "DomainUtilizationFactory.h"
#include "ConfigTdpDataSyncInterface.h"

class UnifiedDomain
{
public:

    UnifiedDomain(const Guid& guid, UIntN participantIndex, UIntN domainIndex, Bool domainEnabled, DomainType::Type domainType, std::string domainName,
        std::string domainDescription, DomainFunctionalityVersions domainFunctionalityVersions,
        const ClassFactories& classFactories, ParticipantServicesInterface* participantServicesInterface);
    ~UnifiedDomain(void);

    Guid getGuid(void);
    Bool isEnabled(void);
    void enable(void);
    void disable(void);

    DomainType::Type getDomainType(void);
    std::string getName(void);
    std::string getDescription(void);
    DomainFunctionalityVersions getDomainFunctionalityVersions(void);
    XmlNode* getXml();

    void clearAllCachedData(void);

    // active control
    DomainActiveControlInterface* getActiveControlInterfacePtr(void);
    ComponentExtendedInterface* getActiveControlInterfaceExPtr(void);

    // configTdp control
    DomainConfigTdpControlInterface* getConfigTdpControlInterfacePtr(void);
    ComponentExtendedInterface* getConfigTdpControlInterfaceExPtr(void);

    // core control
    DomainCoreControlInterface* getCoreControlInterfacePtr(void);
    ComponentExtendedInterface* getCoreControlInterfaceExPtr(void);

    // display control
    DomainDisplayControlInterface* getDisplayControlInterfacePtr(void);
    ComponentExtendedInterface* getDisplayControlInterfaceExPtr(void);

    // domain priority
    DomainPriorityInterface* getDomainPriorityInterfacePtr(void);
    ComponentExtendedInterface* getDomainPriorityInterfaceExPtr(void);

    // performance control
    DomainPerformanceControlInterface* getPerformanceControlInterfacePtr(void);
    ComponentExtendedInterface* getPerformanceControlInterfaceExPtr(void);
    ConfigTdpDataSyncInterface* getPerformanceControlConfigTdpSyncInterfacePtr(void);

    // power control
    DomainPowerControlInterface* getPowerControlInterfacePtr(void);
    ComponentExtendedInterface* getPowerControlInterfaceExPtr(void);
    ConfigTdpDataSyncInterface* getPowerControlConfigTdpSyncInterfacePtr(void);

    // power status
    DomainPowerStatusInterface* getPowerStatusInterfacePtr(void);
    ComponentExtendedInterface* getPowerStatusInterfaceExPtr(void);

    // temperature
    DomainTemperatureInterface* getTemperatureInterfacePtr(void);
    ComponentExtendedInterface* getTemperatureInterfaceExPtr(void);

    // utilization
    DomainUtilizationInterface* getUtilizationInterfacePtr(void);
    ComponentExtendedInterface* getUtilizationInterfaceExPtr(void);

private:

    // hide the copy constructor and = operator
    UnifiedDomain(const UnifiedDomain& rhs);
    UnifiedDomain& operator=(const UnifiedDomain& rhs);

    Guid m_guid;
    UIntN m_participantIndex;
    UIntN m_domainIndex;
    Bool m_enabled;
    DomainType::Type m_domainType;
    std::string m_name;
    std::string m_description;
    DomainFunctionalityVersions m_domainFunctionalityVersions;
    ParticipantServicesInterface* m_participantServicesInterface;

    // Interface pointers to the classes that will do the actual work for the domain.
    DomainActiveControlInterface* m_activeControl;
    DomainConfigTdpControlInterface* m_configTdpControl;
    DomainCoreControlInterface* m_coreControl;
    DomainDisplayControlInterface* m_displayControl;
    DomainPriorityInterface* m_domainPriority;
    DomainPerformanceControlInterface* m_performanceControl;
    DomainPowerControlInterface* m_powerControl;
    DomainPowerStatusInterface* m_powerStatus;
    DomainTemperatureInterface* m_temperature;
    DomainUtilizationInterface* m_utilization;

    // use the class factories to create the needed objects
    void createActiveControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createConfigTdpControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createCoreControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createDisplayControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createDomainPriorityObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createPerformanceControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createPowerControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createPowerStatusObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createTemperatureObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createUtilizationObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);

    void throwIfDomainNotEnabled(void);
};