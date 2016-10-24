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

#include "CoreControlLpoPreference.h"
#include "EsifDataBinaryClpoPackage.h"
#include "XmlNode.h"
#include "StatusFormat.h"

CoreControlLpoPreference::CoreControlLpoPreference(Bool lpoEnabled, UIntN lpoStartPState,
    Percentage lpoStepSize, CoreControlOffliningMode::Type lpoPowerControlOffliningMode,
    CoreControlOffliningMode::Type lpoPerformanceControlOffliningMode) : m_lpoEnabled(lpoEnabled),
    m_startPState(lpoStartPState), m_stepSize(lpoStepSize),
    m_powerControlOffliningMode(lpoPowerControlOffliningMode),
    m_performanceControlOffliningMode(lpoPerformanceControlOffliningMode)
{
}

CoreControlLpoPreference CoreControlLpoPreference::createFromClpo(const DptfBuffer& buffer)
{
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    data += sizeof(esif_data_variant); //Ignore revision field
    struct EsifDataBinaryClpoPackage* currentRow = reinterpret_cast<struct EsifDataBinaryClpoPackage*>(data);

    if (buffer.size() == 0)
    {
        throw dptf_exception("Received empty TDPL buffer.");
    }

    if ((buffer.size() - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryClpoPackage))
    {
        throw dptf_exception("Expected binary data size mismatch. (CLPO)");
    }

    return CoreControlLpoPreference(
        (currentRow->lpoEnable.integer.value != 0),
        static_cast<UIntN>(currentRow->startPstateIndex.integer.value),
        Percentage(static_cast<UIntN>(currentRow->stepSize.integer.value) / 100.0),
        static_cast<CoreControlOffliningMode::Type>(currentRow->powerControlSetting.integer.value),
        static_cast<CoreControlOffliningMode::Type>(currentRow->performanceControlSetting.integer.value));
}

Bool CoreControlLpoPreference::isLpoEnabled(void) const
{
    return m_lpoEnabled;
}

UIntN CoreControlLpoPreference::getStartPState(void) const
{
    return m_startPState;
}

Percentage CoreControlLpoPreference::getStepSize(void) const
{
    return m_stepSize;
}

CoreControlOffliningMode::Type CoreControlLpoPreference::getPowerControlOffliningMode(void) const
{
    return m_powerControlOffliningMode;
}

CoreControlOffliningMode::Type CoreControlLpoPreference::getPerformanceControlOffliningMode(void) const
{
    return m_performanceControlOffliningMode;
}

Bool CoreControlLpoPreference::operator==(const CoreControlLpoPreference& rhs) const
{
    return
        (m_lpoEnabled == rhs.m_lpoEnabled &&
         m_startPState == rhs.m_startPState &&
         m_stepSize == rhs.m_stepSize &&
         m_powerControlOffliningMode == rhs.m_powerControlOffliningMode &&
         m_performanceControlOffliningMode == rhs.m_performanceControlOffliningMode);
}

Bool CoreControlLpoPreference::operator!=(const CoreControlLpoPreference& rhs) const
{
    return !(*this == rhs);
}

std::shared_ptr<XmlNode> CoreControlLpoPreference::getXml(void)
{
    auto root = XmlNode::createWrapperElement("core_control_lpo_preference");
    root->addChild(XmlNode::createDataElement("lpo_enabled", StatusFormat::friendlyValue(m_lpoEnabled)));
    root->addChild(XmlNode::createDataElement("start_p_state", StatusFormat::friendlyValue(m_startPState)));
    root->addChild(XmlNode::createDataElement("step_size", m_stepSize.toString()));
    root->addChild(XmlNode::createDataElement("power_control_offlining_mode", CoreControlOffliningMode::ToString(m_powerControlOffliningMode)));
    root->addChild(XmlNode::createDataElement("performance_control_offlining_mode", CoreControlOffliningMode::ToString(m_performanceControlOffliningMode)));
    return root;
}