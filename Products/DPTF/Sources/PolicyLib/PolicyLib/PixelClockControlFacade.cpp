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

#include "PixelClockControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

PixelClockControlFacade::PixelClockControlFacade(
    UIntN participantIndex, 
    UIntN domainIndex, 
    const DomainProperties& domainProperties, 
    const PolicyServicesInterfaceContainer& policyServices)
    : m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_domainProperties(domainProperties),
    m_policyServices(policyServices),
    m_controlsHaveBeenInitialized(false)
{
}

PixelClockControlFacade::~PixelClockControlFacade()
{
}

Bool PixelClockControlFacade::supportsPixelClockControls()
{
    return m_domainProperties.implementsPixelClockControlInterface();
}

Bool PixelClockControlFacade::supportsStatus()
{
    return m_domainProperties.implementsPixelClockStatusInterface();
}

void PixelClockControlFacade::sendPixelClockFrequencies(const PixelClockDataSet& pixelClockSet)
{
    throwIfControlNotSupported();
    m_policyServices.domainPixelClockControl->setPixelClockControl(
        m_participantIndex, m_domainIndex, pixelClockSet);
}

PixelClockCapabilities PixelClockControlFacade::getPixelClockCapabilities()
{
    throwIfStatusNotSupported();
    return m_policyServices.domainPixelClockStatus->getPixelClockCapabilities(m_participantIndex, m_domainIndex);
}

PixelClockDataSet PixelClockControlFacade::getPixelClockDataSet()
{
    throwIfStatusNotSupported();
    return m_policyServices.domainPixelClockStatus->getPixelClockDataSet(m_participantIndex, m_domainIndex);
}

XmlNode* PixelClockControlFacade::getXml()
{
    XmlNode* status = XmlNode::createWrapperElement("control_pixel_clock");
    if (supportsStatus())
    {
        status->addChild(getPixelClockCapabilities().getXml());
        status->addChild(getPixelClockDataSet().getXml());
    }
    return status;
}

void PixelClockControlFacade::throwIfStatusNotSupported()
{
    if (supportsStatus() == false)
    {
        throw dptf_exception("Pixel clock status is not supported.");
    }
}

void PixelClockControlFacade::throwIfControlNotSupported()
{
    if (supportsPixelClockControls() == false)
    {
        throw dptf_exception("Pixel clock control is not supported.");
    }
}