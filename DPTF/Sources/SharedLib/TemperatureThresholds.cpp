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

#include "TemperatureThresholds.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

TemperatureThresholds::TemperatureThresholds(Temperature aux0, Temperature aux1, UIntN hysteresis) :
    m_aux0(aux0), m_aux1(aux1), m_hysteresis(hysteresis)
{
    // FIXME: this needs to be added back later
    //if ((m_aux0.isValid() == true) &&
    //    (m_aux0 != Temperature(0)) &&
    //    (m_aux1.isValid() == true) &&
    //    (m_aux1 != Temperature(0)) &&
    //    (m_aux0 >= m_aux1))
    //{
    //    throw dptf_exception("Aux0 must be less than Aux1.");
    //}
}

Temperature TemperatureThresholds::getAux0(void) const
{
    return m_aux0;
}

Temperature TemperatureThresholds::getAux1(void) const
{
    return m_aux1;
}

UIntN TemperatureThresholds::getHysteresis(void) const
{
    return m_hysteresis;
}

XmlNode* TemperatureThresholds::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("temperature_thresholds");

    root->addChild(XmlNode::createDataElement("aux0", getAux0().toString()));
    root->addChild(XmlNode::createDataElement("aux1", getAux1().toString()));
    root->addChild(XmlNode::createDataElement("hysteresis", StatusFormat::friendlyValue(m_hysteresis)));

    return root;
}