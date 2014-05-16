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

#include "CoreControlDynamicCaps.h"
#include "XmlNode.h"
#include "StatusFormat.h"

CoreControlDynamicCaps::CoreControlDynamicCaps(UIntN minActiveCores, UIntN maxActiveCores) :
    m_minActiveCores(minActiveCores), m_maxActiveCores(maxActiveCores)
{
    if (minActiveCores > maxActiveCores)
    {
        throw dptf_exception("minActiveCores > maxActiveCores");
    }
}

UIntN CoreControlDynamicCaps::getMinActiveCores(void) const
{
    return m_minActiveCores;
}

UIntN CoreControlDynamicCaps::getMaxActiveCores(void) const
{
    return m_maxActiveCores;
}

Bool CoreControlDynamicCaps::operator==(const CoreControlDynamicCaps& rhs) const
{
    return
        (m_minActiveCores == rhs.m_minActiveCores &&
         m_maxActiveCores == rhs.m_maxActiveCores);
}

Bool CoreControlDynamicCaps::operator!=(const CoreControlDynamicCaps& rhs) const
{
    return !(*this == rhs);
}

XmlNode* CoreControlDynamicCaps::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("core_control_dynamic_caps");

    root->addChild(XmlNode::createDataElement("max_active_cores", StatusFormat::friendlyValue(m_maxActiveCores)));
    root->addChild(XmlNode::createDataElement("min_active_cores", StatusFormat::friendlyValue(m_minActiveCores)));

    return root;
}