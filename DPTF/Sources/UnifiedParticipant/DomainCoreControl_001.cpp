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

#include "DomainCoreControl_001.h"
#include "XmlNode.h"

DomainCoreControl_001::DomainCoreControl_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface),
    m_coreControlStaticCaps(nullptr),
    m_coreControlDynamicCaps(nullptr),
    m_coreControlLpoPreference(nullptr),
    m_coreControlStatus(nullptr)
{
    m_coreControlStatus = new CoreControlStatus(Constants::Invalid);
}

DomainCoreControl_001::~DomainCoreControl_001(void)
{
    DELETE_MEMORY_TC(m_coreControlStaticCaps);
    DELETE_MEMORY_TC(m_coreControlDynamicCaps);
    DELETE_MEMORY_TC(m_coreControlLpoPreference);
    DELETE_MEMORY_TC(m_coreControlStatus);
}

CoreControlStaticCaps DomainCoreControl_001::getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
    createCoreControlStaticCapsIfNeeded(domainIndex);
    return *m_coreControlStaticCaps;
}

CoreControlDynamicCaps DomainCoreControl_001::getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    createCoreControlDynamicCapsIfNeeded(domainIndex);
    return *m_coreControlDynamicCaps;
}

CoreControlLpoPreference DomainCoreControl_001::getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex)
{
    createCoreControlLpoPreferenceIfNeeded(domainIndex);
    return *m_coreControlLpoPreference;
}

CoreControlStatus DomainCoreControl_001::getCoreControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    if (m_coreControlStatus->getNumActiveLogicalProcessors() == Constants::Invalid)
    {
        throw dptf_exception("No core control has been set.  No status available.");
    }

    return *m_coreControlStatus;
}

void DomainCoreControl_001::setActiveCoreControl(UIntN participantIndex, UIntN domainIndex,
    const CoreControlStatus& coreControlStatus)
{
    verifyCoreControlStatus(domainIndex, coreControlStatus);
    createCoreControlStaticCapsIfNeeded(domainIndex);
    UIntN totalCores = m_coreControlStaticCaps->getTotalLogicalProcessors();
    UIntN totalOfflineCoreRequest = totalCores - coreControlStatus.getNumActiveLogicalProcessors();

    m_participantServicesInterface->primitiveExecuteSetAsUInt32(
        esif_primitive_type::SET_PROC_NUMBER_OFFLINE_CORES,
        totalOfflineCoreRequest,
        domainIndex);

    // Refresh the status
    DELETE_MEMORY_TC(m_coreControlStatus);
    m_coreControlStatus = new CoreControlStatus(coreControlStatus.getNumActiveLogicalProcessors());
}

void DomainCoreControl_001::clearCachedData(void)
{
    // Do not delete m_coreControlStatus.  We can't read this from ESIF.  We store it whenever it is 'set' and return
    // the value when requested.

    DELETE_MEMORY_TC(m_coreControlStaticCaps);
    DELETE_MEMORY_TC(m_coreControlDynamicCaps);
    DELETE_MEMORY_TC(m_coreControlLpoPreference);
}

XmlNode* DomainCoreControl_001::getXml(UIntN domainIndex)
{
    createCoreControlStaticCapsIfNeeded(domainIndex);
    createCoreControlDynamicCapsIfNeeded(domainIndex);
    createCoreControlLpoPreferenceIfNeeded(domainIndex);

    XmlNode* root = XmlNode::createWrapperElement("core_control");

    root->addChild(m_coreControlStatus->getXml());
    root->addChild(m_coreControlStaticCaps->getXml());
    root->addChild(m_coreControlDynamicCaps->getXml());
    root->addChild(m_coreControlLpoPreference->getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    return root;
}

void DomainCoreControl_001::createCoreControlStaticCapsIfNeeded(UIntN domainIndex)
{
    if (m_coreControlStaticCaps == nullptr)
    {
        UInt32 logicalCoreCount = m_participantServicesInterface->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_PROC_LOGICAL_PROCESSOR_COUNT,
            domainIndex);

        m_coreControlStaticCaps = new CoreControlStaticCaps(logicalCoreCount);
    }
}

void DomainCoreControl_001::createCoreControlDynamicCapsIfNeeded(UIntN domainIndex)
{
    if (m_coreControlDynamicCaps == nullptr)
    {
        createCoreControlStaticCapsIfNeeded(domainIndex);

        UInt32 minActiveCoreLimit = 1;
        UInt32 maxActiveCoreLimit = m_coreControlStaticCaps->getTotalLogicalProcessors();
        m_coreControlDynamicCaps = new CoreControlDynamicCaps(minActiveCoreLimit, maxActiveCoreLimit);
    }
}

void DomainCoreControl_001::createCoreControlLpoPreferenceIfNeeded(UIntN domainIndex)
{
    if (m_coreControlLpoPreference == nullptr)
    {
        Bool useDefault = false;
        UInt32 dataLength = 0;
        DptfMemory binaryData(Constants::DefaultBufferSize);

        try
        {
            m_participantServicesInterface->primitiveExecuteGet(
                esif_primitive_type::GET_PROC_CURRENT_LOGICAL_PROCESSOR_OFFLINING,
                ESIF_DATA_BINARY,
                binaryData,
                binaryData.getSize(),
                &dataLength,
                domainIndex);
        }
        catch (...)
        {
            m_participantServicesInterface->writeMessageWarning(
                ParticipantMessage(FLF, "CLPO not found.  Using defaults."));
            useDefault = true;
        }

        if (useDefault == false)
        {
            try
            {
                m_coreControlLpoPreference = BinaryParse::processorClpoObject(dataLength, binaryData);
            }
            catch (...)
            {
                m_participantServicesInterface->writeMessageWarning(
                    ParticipantMessage(FLF, "Could not parse CLPO data.  Using defaults."));
                DELETE_MEMORY_TC(m_coreControlLpoPreference);
                useDefault = true;
            }
        }

        if (useDefault == true)
        {
            m_coreControlLpoPreference = new CoreControlLpoPreference(true, 0, Percentage(.50),
                CoreControlOffliningMode::Smt, CoreControlOffliningMode::Core);
        }

        binaryData.deallocate();
    }
}

void DomainCoreControl_001::verifyCoreControlStatus(UIntN domainIndex, const CoreControlStatus& coreControlStatus)
{
    createCoreControlDynamicCapsIfNeeded(domainIndex);

    if (coreControlStatus.getNumActiveLogicalProcessors() > m_coreControlDynamicCaps->getMaxActiveCores() ||
        coreControlStatus.getNumActiveLogicalProcessors() < m_coreControlDynamicCaps->getMinActiveCores())
    {
        throw dptf_exception("Desired number of cores outside dynamic caps range.");
    }
}