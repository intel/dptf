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

#include "CoreControlLpoPreference.h"
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

XmlNode* CoreControlLpoPreference::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("core_control_lpo_preference");
    root->addChild(XmlNode::createDataElement("lpo_enabled", StatusFormat::friendlyValue(m_lpoEnabled)));
    root->addChild(XmlNode::createDataElement("start_p_state", StatusFormat::friendlyValue(m_startPState)));
    root->addChild(XmlNode::createDataElement("step_size", m_stepSize.toString()));
    root->addChild(XmlNode::createDataElement("power_control_offlining_mode", CoreControlOffliningMode::ToString(m_powerControlOffliningMode)));
    root->addChild(XmlNode::createDataElement("performance_control_offlining_mode", CoreControlOffliningMode::ToString(m_performanceControlOffliningMode)));
    return root;
}