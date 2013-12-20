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
#include "DomainPerformanceControl_001.h"
#include <sstream>
#include "XmlNode.h"

// Generic Participant Performance Controls

DomainPerformanceControl_001::DomainPerformanceControl_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
    initializeDataStructures();
}

DomainPerformanceControl_001::~DomainPerformanceControl_001(void)
{
    clearCachedData();
}

PerformanceControlStaticCaps DomainPerformanceControl_001::getPerformanceControlStaticCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    return *m_performanceControlStaticCaps;  //This is hard-coded to FALSE in 7.0
}

PerformanceControlDynamicCaps DomainPerformanceControl_001::getPerformanceControlDynamicCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    return *m_performanceControlDynamicCaps;
}

PerformanceControlStatus DomainPerformanceControl_001::getPerformanceControlStatus(UIntN participantIndex,
    UIntN domainIndex)
{
    if (m_performanceControlStatus == nullptr)
    {
        throw dptf_exception("No performance control has been set.  No status available.");
    }

    return *m_performanceControlStatus;
}

PerformanceControlSet DomainPerformanceControl_001::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    return *m_performanceControlSet;
}

void DomainPerformanceControl_001::setPerformanceControl(UIntN participantIndex, UIntN domainIndex,
    UIntN performanceControlIndex)
{
    checkAndCreateControlStructures(domainIndex);

    if (m_performanceControlStatus != nullptr && performanceControlIndex == m_performanceControlStatus->getCurrentControlSetIndex())
    {
        m_participantServicesInterface->writeMessageInfo(
            ParticipantMessage(FLF, "Requested limit = current limit.  Ignoring."));
        return;
    }

    verifyPerformanceControlIndex(performanceControlIndex);

    m_participantServicesInterface->primitiveExecuteSetAsUInt32(
        esif_primitive_type::SET_PERF_PRESENT_CAPABILITY,
        performanceControlIndex,
        domainIndex);

    // Refresh the status
    delete m_performanceControlStatus;
    m_performanceControlStatus = new PerformanceControlStatus(performanceControlIndex);
}

void DomainPerformanceControl_001::initializeDataStructures(void)
{
    m_performanceControlSet = nullptr;
    m_performanceControlDynamicCaps = nullptr;
    m_performanceControlStatus = nullptr;
    m_performanceControlStaticCaps = nullptr;
}

void DomainPerformanceControl_001::clearCachedData(void)
{
    delete m_performanceControlSet;
    delete m_performanceControlDynamicCaps;
    delete m_performanceControlStatus;
    delete m_performanceControlStaticCaps;

    initializeDataStructures();
}

void DomainPerformanceControl_001::createPerformanceControlSet(UIntN domainIndex)
{
    UInt32 dataLength = 0;
    DptfMemory binaryData(Constants::DefaultBufferSize);

    //Build PPSS table
    m_participantServicesInterface->primitiveExecuteGet(
        esif_primitive_type::GET_PERF_SUPPORT_STATES,
        ESIF_DATA_BINARY,
        binaryData,
        binaryData.getSize(),
        &dataLength,
        domainIndex);

    m_performanceControlSet = new PerformanceControlSet(BinaryParse::genericPpssObject(dataLength, binaryData));

    if (m_performanceControlSet->getCount() == 0)
    {
        binaryData.deallocate();

        throw dptf_exception("P-state set is empty.  \
                                 Impossible if we support performance controls.");
    }

    binaryData.deallocate();
}

void DomainPerformanceControl_001::createPerformanceControlDynamicCaps(UIntN domainIndex)
{
    //Get dynamic caps
    UInt32 lowerLimitIndex;
    UInt32 upperLimitIndex;

    try
    {
        lowerLimitIndex =
            m_participantServicesInterface->primitiveExecuteGetAsUInt32(
                esif_primitive_type::GET_PERF_PSTATE_DEPTH_LIMIT,
                domainIndex);
    }
    catch (...)
    {
        // If PPDL is not supported, default to Pn.
        lowerLimitIndex = m_performanceControlSet->getCount() - 1;
    }

    try
    {
        // If PPPC is not supported, default to P0
        upperLimitIndex =
            m_participantServicesInterface->primitiveExecuteGetAsUInt32(
                esif_primitive_type::GET_PARTICIPANT_PERF_PRESENT_CAPABILITY,
                domainIndex);
    }
    catch (...)
    {
        upperLimitIndex = 0;
    }

    if (m_performanceControlSet == nullptr)
    {
        createPerformanceControlSet(domainIndex);
    }

    if (upperLimitIndex >= (m_performanceControlSet->getCount()) ||
        lowerLimitIndex >= (m_performanceControlSet->getCount()))
    {
        throw dptf_exception("Retrieved control index out of control set bounds.");
    }

    if (upperLimitIndex > lowerLimitIndex)
    {
        lowerLimitIndex = m_performanceControlSet->getCount() - 1;
        m_participantServicesInterface->writeMessageWarning(
            ParticipantMessage(FLF, "Limit index mismatch, ignoring lower limit."));
    }

    m_performanceControlDynamicCaps = new PerformanceControlDynamicCaps(lowerLimitIndex, upperLimitIndex);
}

void DomainPerformanceControl_001::verifyPerformanceControlIndex(UIntN performanceControlIndex)
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

void DomainPerformanceControl_001::checkAndCreateControlStructures(UIntN domainIndex)
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

XmlNode* DomainPerformanceControl_001::getXml(UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    XmlNode* root = XmlNode::createWrapperElement("performance_control");
    root->addChild(m_performanceControlStatus->getXml());
    root->addChild(m_performanceControlDynamicCaps->getXml());
    root->addChild(m_performanceControlStaticCaps->getXml());
    root->addChild(m_performanceControlSet->getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    return root;
}

void DomainPerformanceControl_001::createPerformanceControlStaticCaps()
{
    m_performanceControlStaticCaps = new PerformanceControlStaticCaps(false);
}

void DomainPerformanceControl_001::updateBasedOnConfigTdpInformation(UIntN participantIndex, UIntN domainIndex, 
    ConfigTdpControlSet configTdpControlSet, ConfigTdpControlStatus configTdpControlStatus)
{
    throw not_implemented();
}
