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
#include "DomainPerformanceControl_003.h"
#include <sstream>
#include "XmlNode.h"

DomainPerformanceControl_003::DomainPerformanceControl_003(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
    initializeDataStructures();
}

DomainPerformanceControl_003::~DomainPerformanceControl_003(void)
{
    clearCachedData();
}

PerformanceControlStaticCaps DomainPerformanceControl_003::getPerformanceControlStaticCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    return *m_performanceControlStaticCaps;  //This is hard-coded to FALSE in 7.0
}

PerformanceControlDynamicCaps DomainPerformanceControl_003::getPerformanceControlDynamicCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    return *m_performanceControlDynamicCaps;
}

PerformanceControlStatus DomainPerformanceControl_003::getPerformanceControlStatus(UIntN participantIndex,
    UIntN domainIndex)
{
    if (m_currentPerformanceControlIndex == Constants::Invalid)
    {
        throw dptf_exception("No performance control has been set.  No status available.");
    }

    return *m_performanceControlStatus;
}

PerformanceControlSet DomainPerformanceControl_003::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    return *m_performanceControlSet;
}

void DomainPerformanceControl_003::setPerformanceControl(UIntN participantIndex, UIntN domainIndex,
    UIntN performanceControlIndex)
{
    checkAndCreateControlStructures(domainIndex);

    verifyPerformanceControlIndex(performanceControlIndex);

    m_participantServicesInterface->primitiveExecuteSetAsUInt32(
        esif_primitive_type::SET_PERF_PRESENT_CAPABILITY, // SET_PERF_SUPPORT_STATE
        performanceControlIndex,
        domainIndex);

    // Refresh the status
    m_currentPerformanceControlIndex = performanceControlIndex;

    delete m_performanceControlStatus;
    m_performanceControlStatus = new PerformanceControlStatus(m_currentPerformanceControlIndex);
}

void DomainPerformanceControl_003::initializeDataStructures( void )
{
    m_performanceControlDynamicCaps = nullptr;
    m_performanceControlStaticCaps = nullptr;
    m_performanceControlSet = nullptr;

    m_currentPerformanceControlIndex = Constants::Invalid;
    m_performanceControlStatus = new PerformanceControlStatus(m_currentPerformanceControlIndex);
}

void DomainPerformanceControl_003::clearCachedData(void)
{
    delete m_performanceControlDynamicCaps;
    delete m_performanceControlStaticCaps;
    delete m_performanceControlSet;
    delete m_performanceControlStatus;

    initializeDataStructures();
}

void DomainPerformanceControl_003::createPerformanceControlSet(UIntN domainIndex)
{
    UInt32 dataLength = 0;
    DptfMemory binaryData(Constants::DefaultBufferSize);

    //Build GFX performance table
    m_participantServicesInterface->primitiveExecuteGet(
        esif_primitive_type::GET_PERF_SUPPORT_STATES,
        ESIF_DATA_BINARY,
        binaryData,
        binaryData.getSize(),
        &dataLength,
        domainIndex);

    m_performanceControlSet = new PerformanceControlSet(BinaryParse::processorGfxPstates(dataLength, binaryData));

    if (m_performanceControlSet->getCount() == 0)
    {
        throw dptf_exception("GFX P-state set is empty.  \
                                 Impossible if we support performance controls.");
    }

    binaryData.deallocate();
}

void DomainPerformanceControl_003::createPerformanceControlDynamicCaps(UIntN domainIndex)
{
    if (m_performanceControlSet == nullptr)
    {
        createPerformanceControlSet(domainIndex);
    }

    m_performanceControlDynamicCaps = new PerformanceControlDynamicCaps(m_performanceControlSet->getCount() - 1, 0);
}

void DomainPerformanceControl_003::verifyPerformanceControlIndex(UIntN performanceControlIndex)
{
    if (performanceControlIndex >= m_performanceControlSet->getCount())
    {
        std::stringstream infoMessage;

        infoMessage << "Control index out of control set bounds." << std::endl
                    << "Desired Index : " << performanceControlIndex << std::endl
                    << "PerformanceControlSet size :" << m_performanceControlSet->getCount() << std::endl;

        throw dptf_exception(infoMessage.str());
    }

    if (performanceControlIndex < m_performanceControlDynamicCaps->getCurrentUpperLimitIndex() ||
        performanceControlIndex > m_performanceControlDynamicCaps->getCurrentLowerLimitIndex())
    {
        std::stringstream infoMessage;

        infoMessage << "Got a performance control index that was outside the allowable range." << std::endl
                    << "Desired Index : " << performanceControlIndex << std::endl
                    << "Upper Limit Index : " << m_performanceControlDynamicCaps->getCurrentUpperLimitIndex() << std::endl
                    << "Lower Limit Index : " << m_performanceControlDynamicCaps->getCurrentLowerLimitIndex() << std::endl;

        throw dptf_exception(infoMessage.str());
    }
}

void DomainPerformanceControl_003::checkAndCreateControlStructures(UIntN domainIndex)
{
    if (m_performanceControlSet == nullptr)
    {
        createPerformanceControlSet(domainIndex);
    }

    if (m_performanceControlDynamicCaps == nullptr)
    {
        createPerformanceControlDynamicCaps(domainIndex);
    }

    if (m_performanceControlStaticCaps == nullptr)
    {
        createPerformanceControlStaticCaps();
    }
}

void DomainPerformanceControl_003::createPerformanceControlStaticCaps()
{
    m_performanceControlStaticCaps = new PerformanceControlStaticCaps(false);
}

XmlNode* DomainPerformanceControl_003::getXml(UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    XmlNode* root = XmlNode::createWrapperElement("performance_control");
    root->addChild(m_performanceControlStatus->getXml());
    root->addChild(m_performanceControlDynamicCaps->getXml());
    root->addChild(m_performanceControlStaticCaps->getXml());
    root->addChild(m_performanceControlSet->getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "003"));

    return root;
}

void DomainPerformanceControl_003::updateBasedOnConfigTdpInformation(UIntN participantIndex, UIntN domainIndex, 
    ConfigTdpControlSet configTdpControlSet, ConfigTdpControlStatus configTdpControlStatus)
{
    throw not_implemented();
}
