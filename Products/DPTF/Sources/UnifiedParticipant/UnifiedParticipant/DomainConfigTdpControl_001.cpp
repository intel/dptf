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

#include "DomainConfigTdpControl_001.h"
#include "XmlNode.h"

static const UInt8 MaxNumberOfConfigTdpControls = 3;

DomainConfigTdpControl_001::DomainConfigTdpControl_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface),
    m_configTdpControlSet(nullptr),
    m_configTdpControlDynamicCaps(nullptr),
    m_configTdpControlStatus(nullptr),
    m_configTdpLevelsAvailable(0),
    m_currentConfigTdpControlId(Constants::Invalid),
    m_configTdpLock(false)
{
}

DomainConfigTdpControl_001::~DomainConfigTdpControl_001(void)
{
    clearCachedData();
}

ConfigTdpControlDynamicCaps DomainConfigTdpControl_001::getConfigTdpControlDynamicCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);
    return *m_configTdpControlDynamicCaps;
}

ConfigTdpControlStatus DomainConfigTdpControl_001::getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);
    return *m_configTdpControlStatus;
}

ConfigTdpControlSet DomainConfigTdpControl_001::getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);
    return *m_configTdpControlSet;
}

void DomainConfigTdpControl_001::setConfigTdpControl(UIntN participantIndex, UIntN domainIndex,
    UIntN configTdpControlIndex)
{
    checkAndCreateControlStructures(domainIndex);

    // If any of the lock bits are set, we cannot program cTDP
    if (m_configTdpLock)
    {
        m_participantServicesInterface->writeMessageWarning(
            ParticipantMessage(FLF, "cTDP set level ignored, lock bit is set!"));
        return;
    }

    // Bounds checking
    verifyConfigTdpControlIndex(configTdpControlIndex);

    m_participantServicesInterface->primitiveExecuteSetAsUInt32(
        esif_primitive_type::SET_PROC_CTDP_CONTROL,
        (UInt32)(*m_configTdpControlSet)[configTdpControlIndex].getControlId(), // This is what 7.x does
        domainIndex);

    m_participantServicesInterface->primitiveExecuteSetAsUInt32(
        esif_primitive_type::SET_PROC_TURBO_ACTIVATION_RATIO,
        (UInt32)((*m_configTdpControlSet)[configTdpControlIndex].getTdpRatio() - 1), // This is what 7.x does
        domainIndex);

    // Then BIOS
    m_participantServicesInterface->primitiveExecuteSetAsUInt32(
        esif_primitive_type::SET_CTDP_POINT,
        configTdpControlIndex,
        domainIndex);

    // Refresh the status
    DELETE_MEMORY_TC(m_configTdpControlStatus);
    m_configTdpControlStatus = new ConfigTdpControlStatus(configTdpControlIndex);
}

void DomainConfigTdpControl_001::clearCachedData(void)
{
    DELETE_MEMORY_TC(m_configTdpControlSet);
    DELETE_MEMORY_TC(m_configTdpControlDynamicCaps);
    DELETE_MEMORY_TC(m_configTdpControlStatus);
}

XmlNode* DomainConfigTdpControl_001::getXml(UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    XmlNode* root = XmlNode::createWrapperElement("config_tdp_control");

    root->addChild(m_configTdpControlDynamicCaps->getXml());
    root->addChild(m_configTdpControlSet->getXml());
    root->addChild(m_configTdpControlStatus->getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    return root;
}

void DomainConfigTdpControl_001::createConfigTdpControlDynamicCaps(UIntN domainIndex)
{
    if (m_configTdpControlSet == nullptr)
    {
        createConfigTdpControlSet(domainIndex);
    }

    // Get dynamic caps
    UInt32 lowerLimitIndex = 0;
    UInt32 upperLimitIndex = 0;

    if (!m_configTdpLock)
    {
        lowerLimitIndex = m_configTdpControlSet->getCount() - 1;  // This is what the 7.0 code does

        upperLimitIndex =
            m_participantServicesInterface->primitiveExecuteGetAsUInt32(
                esif_primitive_type::GET_PROC_CTDP_CAPABILITY,
                domainIndex);

        if (m_configTdpControlSet && upperLimitIndex >= (m_configTdpControlSet->getCount() - 1))
        {
            throw dptf_exception("Retrieved control index out of control set bounds.");
        }
    }

    m_configTdpControlDynamicCaps = new ConfigTdpControlDynamicCaps(lowerLimitIndex, upperLimitIndex);
}

void DomainConfigTdpControl_001::createConfigTdpControlSet(UIntN domainIndex)
{
    // Build TDPL table
    UInt32 dataLength = 0;
    DptfMemory binaryData(Constants::DefaultBufferSize);
    m_participantServicesInterface->primitiveExecuteGet(
        esif_primitive_type::GET_PROC_CTDP_POINT_LIST,
        ESIF_DATA_BINARY,
        binaryData,
        binaryData.getSize(),
        &dataLength,
        domainIndex);
    std::vector<ConfigTdpControl> controls = BinaryParse::processorTdplObject(dataLength, binaryData);
    binaryData.deallocate();

    checkHWConfigTdpSupport(controls, domainIndex);

    // If any lock bit is set, we only provide 1 cTDP level to the policies.
    if (m_configTdpLock)
    {
        while (controls.size() > 1)
        {
            controls.pop_back();
        }
    }

    m_configTdpControlSet = new ConfigTdpControlSet(controls);
}

void DomainConfigTdpControl_001::checkHWConfigTdpSupport(std::vector<ConfigTdpControl> controls, UIntN domainIndex)
{
    m_configTdpLevelsAvailable = getLevelCount(domainIndex);
    m_configTdpLock = isLockBitSet(domainIndex);

    UIntN currentTdpControl = //ulTdpControl NOT index...
        m_participantServicesInterface->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_PROC_CTDP_CURRENT_SETTING,
            domainIndex);

    // Determine the index...
    Bool controlIdFound = false;
    for (UIntN i = 0; i < controls.size(); i++)
    {
        if (currentTdpControl == controls.at(i).getControlId())
        {
            DELETE_MEMORY_TC(m_configTdpControlStatus);
            m_configTdpControlStatus = new ConfigTdpControlStatus(i);
            controlIdFound = true;
        }
    }

    if (controlIdFound == false)
    {
        throw dptf_exception("cTDP control not found in set.");
    }
}

void DomainConfigTdpControl_001::verifyConfigTdpControlIndex(UIntN configTdpControlIndex)
{
    if (configTdpControlIndex >= m_configTdpControlSet->getCount())
    {
        std::stringstream infoMessage;

        infoMessage << "Control index out of control set bounds." << std::endl
                    << "Desired Index : " << configTdpControlIndex << std::endl
                    << "PerformanceControlSet size :" << m_configTdpControlSet->getCount() << std::endl;

        throw dptf_exception(infoMessage.str());
    }

    if (configTdpControlIndex < m_configTdpControlDynamicCaps->getCurrentUpperLimitIndex() ||
        configTdpControlIndex > m_configTdpControlDynamicCaps->getCurrentLowerLimitIndex())
    {
        std::stringstream infoMessage;

        infoMessage << "Got a performance control index that was outside the allowable range." << std::endl
                    << "Desired Index : " << configTdpControlIndex << std::endl
                    << "Upper Limit Index : " << m_configTdpControlDynamicCaps->getCurrentUpperLimitIndex() << std::endl
                    << "Lower Limit Index : " << m_configTdpControlDynamicCaps->getCurrentLowerLimitIndex() << std::endl;

        throw dptf_exception(infoMessage.str());
    }
}

UIntN DomainConfigTdpControl_001::getLevelCount(UIntN domainIndex)
{
    UInt32 cTDPSupport =
        m_participantServicesInterface->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_PROC_CTDP_SUPPORT_CHECK,
            domainIndex);

    if (cTDPSupport == 0 || cTDPSupport == 3)
    {
        // cTDP is not supported in HW!
        // Because of this Custom cTDP is not supported as well
        //   PLATFORM_INFO Register(Read Only)
        //   bit 34:33 - Number of configurable TDP levels - default : 00b
        //     00 - Config TDP not supported
        //     01 - One additional TDP level supported
        //     10 - Two additional TDP levels supported
        //     11 - Reserved
        throw dptf_exception("ConfigTdp not supported.");
    }
    else
    {
        return cTDPSupport + 1; // Add 1 to account for the nominal level
    }
}

Bool DomainConfigTdpControl_001::isLockBitSet(UIntN domainIndex)
{
    // Check to see if the TAR or if cTDP is locked
    Bool tarLock =
        (m_participantServicesInterface->primitiveExecuteGetAsUInt32(
             esif_primitive_type::GET_PROC_CTDP_TAR_LOCK_STATUS,
             domainIndex) == 1) ? true : false;

    Bool configTdpLock =
        (m_participantServicesInterface->primitiveExecuteGetAsUInt32(
             esif_primitive_type::GET_PROC_CTDP_LOCK_STATUS,
             domainIndex) == 1) ? true : false;

    if (tarLock || configTdpLock)
    {
        m_participantServicesInterface->writeMessageWarning(
            ParticipantMessage(FLF, "cTDP is supported, but the lock bit is set!"));
        return true;
    }
    else
    {
        return false;
    }
}

void DomainConfigTdpControl_001::checkAndCreateControlStructures(UIntN domainIndex)
{
    if (m_configTdpControlSet == nullptr)
    {
        createConfigTdpControlSet(domainIndex);
    }

    if (m_configTdpControlDynamicCaps == nullptr)
    {
        createConfigTdpControlDynamicCaps(domainIndex);
    }
}