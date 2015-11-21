/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#include "DomainDisplayControl_001.h"
#include "XmlNode.h"
#include <algorithm>

DomainDisplayControl_001::DomainDisplayControl_001(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface) :
    DomainDisplayControlBase(participantIndex, domainIndex, participantServicesInterface)
{
    initializeDataStructures();
}

DomainDisplayControl_001::~DomainDisplayControl_001(void)
{
    DELETE_MEMORY_TC(m_displayControlDynamicCaps);
    DELETE_MEMORY_TC(m_displayControlSet);
}

DisplayControlDynamicCaps DomainDisplayControl_001::getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    return *m_displayControlDynamicCaps;
}

DisplayControlStatus DomainDisplayControl_001::getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    // FIXME : This primitive will return the brightness after ALS setting has been applied!

    Percentage brightnessPercentage = getParticipantServices()->primitiveExecuteGetAsPercentage(
        esif_primitive_type::GET_DISPLAY_BRIGHTNESS, domainIndex);

    auto displayControlIndex = m_displayControlSet->getControlIndex(brightnessPercentage);

    if (displayControlIndex != m_currentDisplayControlIndex)
    {
        m_currentDisplayControlIndex = displayControlIndex;
    }

    return DisplayControlStatus(m_currentDisplayControlIndex);
}

DisplayControlSet DomainDisplayControl_001::getDisplayControlSet(UIntN participantIndex, UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    return *m_displayControlSet;
}

void DomainDisplayControl_001::setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex)
{
    checkAndCreateControlStructures(domainIndex);

    if (displayControlIndex == m_currentDisplayControlIndex)
    {
        getParticipantServices()->writeMessageDebug(
            ParticipantMessage(FLF, "Requested limit = current limit.  Ignoring."));
        return;
    }

    verifyDisplayControlIndex(displayControlIndex);

    Percentage newBrightness = (*m_displayControlSet)[displayControlIndex].getBrightness();

    getParticipantServices()->primitiveExecuteSetAsPercentage(
        esif_primitive_type::SET_DISPLAY_BRIGHTNESS,
        newBrightness,
        domainIndex);

    // Refresh the current index
    m_currentDisplayControlIndex = displayControlIndex;
}

void DomainDisplayControl_001::initializeDataStructures(void)
{
    m_displayControlDynamicCaps = nullptr;
    m_displayControlSet = nullptr;

    m_currentDisplayControlIndex = Constants::Invalid;
}

void DomainDisplayControl_001::clearCachedData(void)
{
    DELETE_MEMORY_TC(m_displayControlDynamicCaps);
    DELETE_MEMORY_TC(m_displayControlSet);

    initializeDataStructures();
}

XmlNode* DomainDisplayControl_001::getXml(UIntN domainIndex)
{
    checkAndCreateControlStructures(domainIndex);

    XmlNode* root = XmlNode::createWrapperElement("display_control");
    root->addChild(getDisplayControlStatus(Constants::Invalid, domainIndex).getXml());
    root->addChild(m_displayControlDynamicCaps->getXml());
    root->addChild(m_displayControlSet->getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    return root;
}

void DomainDisplayControl_001::createDisplayControlDynamicCaps(UIntN domainIndex)
{
    UIntN upperLimitIndex;
    UIntN lowerLimitIndex;
    UInt32 uint32val;

    // Get dynamic caps
    //  The caps are stored in BIOS as brightness percentage.  They must be converted
    //  to indices before they can be used.

    // FIXME:  ESIF treats this as a UInt32 but we treat this as a percentage.  Need to get this in sync.
    uint32val = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_DISPLAY_DEPTH_LIMIT, domainIndex);
    Percentage lowerLimitBrightness = Percentage::fromWholeNumber(uint32val);
    lowerLimitIndex = m_displayControlSet->getControlIndex(lowerLimitBrightness);

    try
    {
        // FIXME:  ESIF treats this as a UInt32 but we treat this as a percentage.  Need to get this in sync.
        uint32val = getParticipantServices()->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_DISPLAY_CAPABILITY, domainIndex);
        Percentage upperLimitBrightness = Percentage::fromWholeNumber(uint32val);
        upperLimitIndex = m_displayControlSet->getControlIndex(upperLimitBrightness);
    }
    catch (...)
    {
        // DDPC is optional
        getParticipantServices()->writeMessageDebug(
            ParticipantMessage(FLF, "DDPC was not present.  Setting upper limit to 100."));
        upperLimitIndex = 0; // Max brightness
    }

    if (m_displayControlSet == nullptr)
    {
        createDisplayControlSet(domainIndex);
    }

    if (upperLimitIndex >= (m_displayControlSet->getCount()) ||
        lowerLimitIndex >= (m_displayControlSet->getCount()))
    {
        throw dptf_exception("Retrieved control index out of control set bounds.");
    }

    if (upperLimitIndex > lowerLimitIndex)
    {
        lowerLimitIndex = m_displayControlSet->getCount() - 1;
        getParticipantServices()->writeMessageWarning(
            ParticipantMessage(FLF, "Limit index mismatch, ignoring lower limit."));
    }

    m_displayControlDynamicCaps = new DisplayControlDynamicCaps(upperLimitIndex, lowerLimitIndex);
}

void DomainDisplayControl_001::createDisplayControlSet(UIntN domainIndex)
{
    // _BCL Table
    DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
        esif_primitive_type::GET_DISPLAY_BRIGHTNESS_LEVELS, ESIF_DATA_BINARY, domainIndex);
    std::vector<DisplayControl> controls = BinaryParse::displayBclObject(buffer);
    if (controls.size() == 0)
    {
        throw dptf_exception("P-state set is empty. Impossible if we support performance controls.");
    }
    m_displayControlSet = new DisplayControlSet(processDisplayControlSetData(controls));
}

void DomainDisplayControl_001::verifyDisplayControlIndex(UIntN displayControlIndex)
{
    if (displayControlIndex >= m_displayControlSet->getCount())
    {
        std::stringstream infoMessage;

        infoMessage << "Control index out of control set bounds." << std::endl
            << "Desired Index : " << displayControlIndex << std::endl
            << "PerformanceControlSet size :" << m_displayControlSet->getCount() << std::endl;

        throw dptf_exception(infoMessage.str());
    }

    Percentage brightnessPercentage = (*m_displayControlSet)[displayControlIndex].getBrightness();

    Percentage upperLimitPercentage =
        (*m_displayControlSet)[m_displayControlDynamicCaps->getCurrentUpperLimit()].getBrightness();

    Percentage lowerLimitPercentage =
        (*m_displayControlSet)[m_displayControlDynamicCaps->getCurrentLowerLimit()].getBrightness();

    if (brightnessPercentage > upperLimitPercentage ||
        brightnessPercentage < lowerLimitPercentage)
    {
        std::stringstream infoMessage;

        infoMessage << "Got a display control that was outside the allowable range." << std::endl
            << "Desired : " << displayControlIndex << std::endl
            << "Upper Limit : " << m_displayControlDynamicCaps->getCurrentUpperLimit() << std::endl
            << "Lower Limit : " << m_displayControlDynamicCaps->getCurrentLowerLimit() << std::endl;

        throw dptf_exception(infoMessage.str());
    }
}

void DomainDisplayControl_001::checkAndCreateControlStructures(UIntN domainIndex)
{
    if (m_displayControlSet == nullptr)
    {
        createDisplayControlSet(domainIndex);
    }

    if (m_displayControlDynamicCaps == nullptr)
    {
        createDisplayControlDynamicCaps(domainIndex);
    }
}

std::vector<DisplayControl> DomainDisplayControl_001::processDisplayControlSetData(std::vector<DisplayControl> controls)
{
    std::vector<DisplayControl> brightnessSet(controls.begin() + 2, controls.end());  //The first 2 elements in the data are not the brightness set
    std::sort(brightnessSet.begin(), brightnessSet.end());
    std::reverse(brightnessSet.begin(), brightnessSet.end());

    return brightnessSet;
}

std::string DomainDisplayControl_001::getName(void)
{
    return "Display Control (Version 1)";
}