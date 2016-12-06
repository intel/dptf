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

#include "DomainControlList.h"
#include "StatusFormat.h"
using namespace std;

DomainControlList::DomainControlList(UIntN participantIndex, UIntN domainIndex,
    DomainFunctionalityVersions domainFunctionalityVersions,
    const ControlFactoryList& controlFactoryList,
    std::shared_ptr<ParticipantServicesInterface> participantServices) :
    m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_domainFunctionalityVersions(domainFunctionalityVersions),
    m_controlFactoryList(controlFactoryList),
    m_participantServices(participantServices)
{
    makeAllControls();
}

DomainControlList::~DomainControlList(void)
{

}

void DomainControlList::makeAllControls()
{
    // if an error is thrown we don't want to catch it as the domain can't be created anyway.
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::Active,
        makeControl<DomainActiveControlBase>(ControlFactoryType::Active,
        m_domainFunctionalityVersions.activeControlVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::ConfigTdp,
        makeControl<DomainConfigTdpControlBase>(ControlFactoryType::ConfigTdp,
        m_domainFunctionalityVersions.configTdpControlVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::Core,
        makeControl<DomainCoreControlBase>(ControlFactoryType::Core,
        m_domainFunctionalityVersions.coreControlVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::Display,
        makeControl<DomainDisplayControlBase>(ControlFactoryType::Display,
        m_domainFunctionalityVersions.displayControlVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::Performance,
        makeControl<DomainPerformanceControlBase>(ControlFactoryType::Performance,
        m_domainFunctionalityVersions.performanceControlVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::PixelClockControl,
        makeControl<DomainPixelClockControlBase>(ControlFactoryType::PixelClockControl,
        m_domainFunctionalityVersions.pixelClockControlVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::PixelClockStatus,
        makeControl<DomainPixelClockStatusBase>(ControlFactoryType::PixelClockStatus,
        m_domainFunctionalityVersions.pixelClockStatusVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::PowerControl,
        makeControl<DomainPowerControlBase>(ControlFactoryType::PowerControl,
        m_domainFunctionalityVersions.powerControlVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::PowerStatus,
        makeControl<DomainPowerStatusBase>(ControlFactoryType::PowerStatus,
        m_domainFunctionalityVersions.powerStatusVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::Priority,
        makeControl<DomainPriorityBase>(ControlFactoryType::Priority,
        m_domainFunctionalityVersions.domainPriorityVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::RfProfileControl,
        makeControl<DomainRfProfileControlBase>(ControlFactoryType::RfProfileControl,
        m_domainFunctionalityVersions.rfProfileControlVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::RfProfileStatus,
        makeControl<DomainRfProfileStatusBase>(ControlFactoryType::RfProfileStatus,
        m_domainFunctionalityVersions.rfProfileStatusVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::Temperature,
        makeControl<DomainTemperatureBase>(ControlFactoryType::Temperature,
        m_domainFunctionalityVersions.temperatureVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::Utilization,
        makeControl<DomainUtilizationBase>(ControlFactoryType::Utilization,
        m_domainFunctionalityVersions.utilizationVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::PlatformPower,
        makeControl<DomainPlatformPowerControlBase>(ControlFactoryType::PlatformPower,
        m_domainFunctionalityVersions.platformPowerControlVersion)));
    m_controlList.insert(pair<ControlFactoryType::Type, std::shared_ptr<ControlBase>>(
        ControlFactoryType::PlatformPowerStatus,
        makeControl<DomainPlatformPowerStatusBase>(ControlFactoryType::PlatformPowerStatus,
        m_domainFunctionalityVersions.platformPowerStatusVersion)));
}

template<typename T>
std::shared_ptr<T> DomainControlList::makeControl(
    ControlFactoryType::Type factoryType, UInt8& controlVersion)
{
    auto factory = m_controlFactoryList.getFactory(factoryType);
    std::shared_ptr<T> control(dynamic_cast<T*>(factory->make(
        m_participantIndex, m_domainIndex, controlVersion, m_participantServices)));
    return control;
}

std::shared_ptr<XmlNode> DomainControlList::getXml()
{
    auto domain = XmlNode::createWrapperElement("domain_controls");
    for (auto control = m_controlList.begin(); control != m_controlList.end(); control++)
    {
        try
        {
            domain->addChild(control->second->getXml(m_domainIndex));
        }
        catch (not_implemented)
        {
            // if not implemented, then eat the error.
        }
        catch (dptf_exception ex)
        {
            m_participantServices->writeMessageWarning(
                ParticipantMessage(FLF, "Unable to get " + control->second->getName() + " control XML status: " +
                ex.getDescription()));
        }
        catch (...)
        {
            m_participantServices->writeMessageWarning(
                ParticipantMessage(FLF, "Unable to get " + control->second->getName() + " control XML status."));
        }
    }
    return domain;
}

void DomainControlList::clearAllCachedData(void)
{
    for (auto control = m_controlList.begin(); control != m_controlList.end(); control++)
    {
        control->second->clearCachedData();
    }
}

std::shared_ptr<DomainActiveControlBase> DomainControlList::getActiveControl(void)
{
    return dynamic_pointer_cast<DomainActiveControlBase>(
        m_controlList.at(ControlFactoryType::Active));
}

std::shared_ptr<DomainConfigTdpControlBase> DomainControlList::getConfigTdpControl(void)
{
    return dynamic_pointer_cast<DomainConfigTdpControlBase>(
        m_controlList.at(ControlFactoryType::ConfigTdp));
}

std::shared_ptr<DomainCoreControlBase> DomainControlList::getCoreControl(void)
{
    return dynamic_pointer_cast<DomainCoreControlBase>(
        m_controlList.at(ControlFactoryType::Core));
}

std::shared_ptr<DomainDisplayControlBase> DomainControlList::getDisplayControl(void)
{
    return dynamic_pointer_cast<DomainDisplayControlBase>(
        m_controlList.at(ControlFactoryType::Display));
}

std::shared_ptr<DomainPerformanceControlBase> DomainControlList::getPerformanceControl(void)
{
    return dynamic_pointer_cast<DomainPerformanceControlBase>(
        m_controlList.at(ControlFactoryType::Performance));
}

std::shared_ptr<DomainPixelClockControlBase> DomainControlList::getPixelClockControl(void)
{
    return dynamic_pointer_cast<DomainPixelClockControlBase>(
        m_controlList.at(ControlFactoryType::PixelClockControl));
}

std::shared_ptr<DomainPixelClockStatusBase> DomainControlList::getPixelClockStatusControl(void)
{
    return dynamic_pointer_cast<DomainPixelClockStatusBase>(
        m_controlList.at(ControlFactoryType::PixelClockStatus));
}

std::shared_ptr<DomainPowerControlBase> DomainControlList::getPowerControl(void)
{
    return dynamic_pointer_cast<DomainPowerControlBase>(
        m_controlList.at(ControlFactoryType::PowerControl));
}

std::shared_ptr<DomainPowerStatusBase> DomainControlList::getPowerStatusControl(void)
{
    return dynamic_pointer_cast<DomainPowerStatusBase>(
        m_controlList.at(ControlFactoryType::PowerStatus));
}

std::shared_ptr<DomainPlatformPowerControlBase> DomainControlList::getPlatformPowerControl(void)
{
    return dynamic_pointer_cast<DomainPlatformPowerControlBase>(
        m_controlList.at(ControlFactoryType::PlatformPower));
}

std::shared_ptr<DomainPlatformPowerStatusBase> DomainControlList::getPlatformPowerStatusControl(void)
{
    return dynamic_pointer_cast<DomainPlatformPowerStatusBase>(
        m_controlList.at(ControlFactoryType::PlatformPowerStatus));
}

std::shared_ptr<DomainPriorityBase> DomainControlList::getDomainPriorityControl(void)
{
    return dynamic_pointer_cast<DomainPriorityBase>(
        m_controlList.at(ControlFactoryType::Priority));
}

std::shared_ptr<DomainRfProfileControlBase> DomainControlList::getRfProfileControl(void)
{
    return dynamic_pointer_cast<DomainRfProfileControlBase>(
        m_controlList.at(ControlFactoryType::RfProfileControl));
}

std::shared_ptr<DomainRfProfileStatusBase> DomainControlList::getRfProfileStatusControl(void)
{
    return dynamic_pointer_cast<DomainRfProfileStatusBase>(
        m_controlList.at(ControlFactoryType::RfProfileStatus));
}

std::shared_ptr<DomainTemperatureBase> DomainControlList::getTemperatureControl(void)
{
    return dynamic_pointer_cast<DomainTemperatureBase>(
        m_controlList.at(ControlFactoryType::Temperature));
}

std::shared_ptr<DomainUtilizationBase> DomainControlList::getUtilizationControl(void)
{
    return dynamic_pointer_cast<DomainUtilizationBase>(
        m_controlList.at(ControlFactoryType::Utilization));
}