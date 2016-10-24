/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "ParticipantServicesInterface.h"
#include "ControlBase.h"
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
#include "ParticipantGetSpecificInfoFactory.h"
#include "ParticipantSetSpecificInfoFactory.h"
#include "DomainPlatformPowerControlFactory.h"
#include "DomainPlatformPowerStatusFactory.h"
#include "DomainTemperatureFactory.h"
#include "DomainUtilizationFactory.h"
#include "DomainActiveControlBase.h"
#include "DomainConfigTdpControlBase.h"
#include "DomainCoreControlBase.h"
#include "DomainDisplayControlBase.h"
#include "DomainPerformanceControlBase.h"
#include "DomainPixelClockControlBase.h"
#include "DomainPixelClockStatusBase.h"
#include "DomainPowerControlBase.h"
#include "DomainPowerStatusBase.h"
#include "DomainPriorityBase.h"
#include "DomainRfProfileControlBase.h"
#include "DomainRfProfileStatusBase.h"
#include "DomainTemperatureBase.h"
#include "DomainUtilizationBase.h"
#include "DomainPlatformPowerControlBase.h"
#include "DomainPlatformPowerStatusBase.h"
#include "ControlFactoryList.h"
#include "XmlNode.h"
#include <memory>

class DomainControlList
{
public:

    DomainControlList(UIntN participantIndex, UIntN domainIndex,
        DomainFunctionalityVersions domainFunctionalityVersions,
        const ControlFactoryList& controlFactoryList,
        ParticipantServicesInterface* participantServices);
    ~DomainControlList(void);

    std::shared_ptr<DomainActiveControlBase> getActiveControl(void);
    std::shared_ptr<DomainConfigTdpControlBase> getConfigTdpControl(void);
    std::shared_ptr<DomainCoreControlBase> getCoreControl(void);
    std::shared_ptr<DomainDisplayControlBase> getDisplayControl(void);
    std::shared_ptr<DomainPerformanceControlBase> getPerformanceControl(void);
    std::shared_ptr<DomainPixelClockControlBase> getPixelClockControl(void);
    std::shared_ptr<DomainPixelClockStatusBase> getPixelClockStatusControl(void);
    std::shared_ptr<DomainPowerControlBase> getPowerControl(void);
    std::shared_ptr<DomainPowerStatusBase> getPowerStatusControl(void);
    std::shared_ptr<DomainPlatformPowerControlBase> getPlatformPowerControl(void);
    std::shared_ptr<DomainPlatformPowerStatusBase> getPlatformPowerStatusControl(void);
    std::shared_ptr<DomainPriorityBase> getDomainPriorityControl(void);
    std::shared_ptr<DomainRfProfileControlBase> getRfProfileControl(void);
    std::shared_ptr<DomainRfProfileStatusBase> getRfProfileStatusControl(void);
    std::shared_ptr<DomainTemperatureBase> getTemperatureControl(void);
    std::shared_ptr<DomainUtilizationBase> getUtilizationControl(void);

    void clearAllCachedData(void);
    std::shared_ptr<XmlNode> getXml();

private:

    // hide the copy constructor and = operator
    DomainControlList(const DomainControlList& rhs);
    DomainControlList& operator=(const DomainControlList& rhs);

    const UIntN m_participantIndex;
    const UIntN m_domainIndex;
    DomainFunctionalityVersions m_domainFunctionalityVersions;
    const ControlFactoryList m_controlFactoryList;
    ParticipantServicesInterface* m_participantServices;
    std::map<ControlFactoryType::Type, std::shared_ptr<ControlBase>> m_controlList;

    void makeAllControls();
    template<typename T> std::shared_ptr<T> makeControl(
        ControlFactoryType::Type factoryType, UInt8& controlVersion);
    template<typename T> void disableControlIfNotWorking(UInt8& controlVersion, 
        std::shared_ptr<T>& controlClass,
        std::shared_ptr<ControlFactoryInterface> factory);
    std::shared_ptr<ControlFactoryInterface> getFactory(ControlFactoryType::Type factoryType);
};