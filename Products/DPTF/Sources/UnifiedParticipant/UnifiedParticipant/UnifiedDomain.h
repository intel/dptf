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
#include "ClassFactories.h"
#include "ParticipantServicesInterface.h"
#include "ComponentExtendedInterface.h"
#include "Guid.h"
#include "DomainType.h"
#include "ConfigTdpDataSyncInterface.h"
#include "DomainFunctionalityVersions.h"
#include "DomainActiveControlFactory.h"
#include "DomainConfigTdpControlFactory.h"
#include "DomainCoreControlFactory.h"
#include "DomainDisplayControlFactory.h"
#include "DomainPerformanceControlFactory.h"
#include "DomainPixelClockControlFactory.h"
#include "DomainPixelClockStatusFactory.h"
#include "DomainPowerControlFactory.h"
#include "DomainPowerStatusFactory.h"
#include "DomainPriorityFactory.h"
#include "DomainRfProfileControlFactory.h"
#include "DomainRfProfileStatusFactory.h"
#include "DomainTemperatureFactory.h"
#include "DomainUtilizationFactory.h"

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

    // performance control
    DomainPerformanceControlInterface* getPerformanceControlInterfacePtr(void);
    ComponentExtendedInterface* getPerformanceControlInterfaceExPtr(void);
    ConfigTdpDataSyncInterface* getPerformanceControlConfigTdpSyncInterfacePtr(void);

    // Pixel Clock Control
    DomainPixelClockControlInterface* getPixelClockControlInterfacePtr(void);
    ComponentExtendedInterface* getPixelClockControlInterfaceExPtr(void);

    // Pixel Clock Status
    DomainPixelClockStatusInterface* getPixelClockStatusInterfacePtr(void);
    ComponentExtendedInterface* getPixelClockStatusInterfaceExPtr(void);

    // power control
    DomainPowerControlInterface* getPowerControlInterfacePtr(void);
    ComponentExtendedInterface* getPowerControlInterfaceExPtr(void);

    // power status
    DomainPowerStatusInterface* getPowerStatusInterfacePtr(void);
    ComponentExtendedInterface* getPowerStatusInterfaceExPtr(void);

    // priority
    DomainPriorityInterface* getDomainPriorityInterfacePtr(void);
    ComponentExtendedInterface* getDomainPriorityInterfaceExPtr(void);

    // RF Profile Control
    DomainRfProfileControlInterface* getRfProfileControlInterfacePtr(void);
    ComponentExtendedInterface* getRfProfileControlInterfaceExPtr(void);

    // RF Profile Status
    DomainRfProfileStatusInterface* getRfProfileStatusInterfacePtr(void);
    ComponentExtendedInterface* getRfProfileStatusInterfaceExPtr(void);

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
    DomainPerformanceControlInterface* m_performanceControl;
    DomainPixelClockControlInterface* m_pixelClockControl;
    DomainPixelClockStatusInterface* m_pixelClockStatus;
    DomainPowerControlInterface* m_powerControl;
    DomainPowerStatusInterface* m_powerStatus;
    DomainPriorityInterface* m_domainPriority;
    DomainRfProfileControlInterface* m_rfProfileControl;
    DomainRfProfileStatusInterface* m_rfProfileStatus;
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
    void createPerformanceControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createPixelClockControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createPixelClockStatusObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createPowerControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createPowerStatusObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createDomainPriorityObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createRfProfileControlObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createRfProfileStatusObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createTemperatureObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);
    void createUtilizationObject(const ClassFactories& classFactories,
        ParticipantServicesInterface* participantServicesInterface);

    void throwIfDomainNotEnabled(void);
};