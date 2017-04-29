/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "DisplayControlFacade.h"
using namespace std;

DisplayControlFacade::DisplayControlFacade(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_policyServices(policyServices)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_domainProperties(domainProperties)
	, m_displayControlSetProperty(participantIndex, domainIndex, domainProperties, policyServices)
	, m_displayControlCapabilitiesProperty(participantIndex, domainIndex, domainProperties, policyServices)
{
}

DisplayControlFacade::~DisplayControlFacade()
{
}

Bool DisplayControlFacade::supportsDisplayControls()
{
	return m_domainProperties.implementsDisplayControlInterface();
}

DisplayControlStatus DisplayControlFacade::getStatus()
{
	if (supportsDisplayControls())
	{
		return m_policyServices.domainDisplayControl->getDisplayControlStatus(m_participantIndex, m_domainIndex);
	}
	else
	{
		throw dptf_exception("Domain does not support the display control interface.");
	}
}

UIntN DisplayControlFacade::getUserPreferredDisplayIndex()
{
	if (supportsDisplayControls())
	{
		return m_policyServices.domainDisplayControl->getUserPreferredDisplayIndex(m_participantIndex, m_domainIndex);
	}
	else
	{
		throw dptf_exception("Domain does not support the display control interface.");
	}
}

Bool DisplayControlFacade::isUserPreferredIndexModified()
{
	if (supportsDisplayControls())
	{
		return m_policyServices.domainDisplayControl->isUserPreferredIndexModified(m_participantIndex, m_domainIndex);
	}
	else
	{
		throw dptf_exception("Domain does not support the display control interface.");
	}
}

void DisplayControlFacade::setControl(UIntN displayControlIndex)
{
	if (supportsDisplayControls())
	{
		m_policyServices.domainDisplayControl->setDisplayControl(
			m_participantIndex, m_domainIndex, displayControlIndex);
	}
	else
	{
		throw dptf_exception("Domain does not support the display control interface.");
	}
}

void DisplayControlFacade::setDisplayControlDynamicCaps(DisplayControlDynamicCaps newCapabilities)
{
	if (supportsDisplayControls())
	{
		m_displayControlCapabilitiesProperty.invalidate();
		m_policyServices.domainDisplayControl->setDisplayControlDynamicCaps(
			m_participantIndex, m_domainIndex, newCapabilities);
		refreshCapabilities();
	}
	else
	{
		throw dptf_exception("Domain does not support the display control interface.");
	}
}

void DisplayControlFacade::lockCapabilities()
{
	if (supportsDisplayControls())
	{
		m_policyServices.domainDisplayControl->setDisplayCapsLock(m_participantIndex, m_domainIndex, true);
	}
	else
	{
		throw dptf_exception("Domain does not support the display control interface.");
	}
}

void DisplayControlFacade::unlockCapabilities()
{
	if (supportsDisplayControls())
	{
		m_policyServices.domainDisplayControl->setDisplayCapsLock(m_participantIndex, m_domainIndex, false);
	}
	else
	{
		throw dptf_exception("Domain does not support the display control interface.");
	}
}

const DisplayControlSet& DisplayControlFacade::getControls()
{
	return m_displayControlSetProperty.getControls();
}

void DisplayControlFacade::invalidateControlSet()
{
	m_displayControlSetProperty.invalidate();
}

void DisplayControlFacade::refreshCapabilities()
{
	m_displayControlCapabilitiesProperty.refresh();
}

const DisplayControlDynamicCaps& DisplayControlFacade::getCapabilities()
{
	return m_displayControlCapabilitiesProperty.getCapabilities();
}

void DisplayControlFacade::setValueWithinCapabilities()
{
	auto controlValue = getStatus().getBrightnessLimitIndex();
	auto capabilities = m_displayControlCapabilitiesProperty.getCapabilities();
	controlValue = std::max(capabilities.getCurrentUpperLimit(), controlValue);
	controlValue = std::min(capabilities.getCurrentLowerLimit(), controlValue);
	setControl(controlValue);
}
