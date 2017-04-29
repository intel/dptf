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

#include "PerformanceControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

PerformanceControlFacade::PerformanceControlFacade(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_policyServices(policyServices)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_domainProperties(domainProperties)
	, m_performanceControlSetProperty(participantIndex, domainIndex, domainProperties, policyServices)
	, m_performanceControlCapabilitiesProperty(participantIndex, domainIndex, domainProperties, policyServices)
	, m_controlsHaveBeenInitialized(false)
	, m_lastIssuedPerformanceControlIndex(Constants::Invalid)
{
}

PerformanceControlFacade::~PerformanceControlFacade()
{
}

Bool PerformanceControlFacade::supportsPerformanceControls()
{
	return m_domainProperties.implementsPerformanceControlInterface();
}

void PerformanceControlFacade::setControl(UIntN performanceControlIndex)
{
	if (supportsPerformanceControls())
	{
		m_policyServices.domainPerformanceControl->setPerformanceControl(
			m_participantIndex, m_domainIndex, performanceControlIndex);
		m_lastIssuedPerformanceControlIndex = performanceControlIndex;
	}
	else
	{
		throw dptf_exception("Domain does not support the performance control interface.");
	}
}

void PerformanceControlFacade::setPerformanceControlDynamicCaps(PerformanceControlDynamicCaps newCapabilities)
{
	if (supportsPerformanceControls())
	{
		m_performanceControlCapabilitiesProperty.invalidate();
		m_policyServices.domainPerformanceControl->setPerformanceControlDynamicCaps(
			m_participantIndex, m_domainIndex, newCapabilities);
		refreshCapabilities();
	}
	else
	{
		throw dptf_exception("Domain does not support the performance control interface.");
	}
}

void PerformanceControlFacade::lockCapabilities()
{
	if (supportsPerformanceControls())
	{
		m_policyServices.domainPerformanceControl->setPerformanceCapsLock(m_participantIndex, m_domainIndex, true);
	}
	else
	{
		throw dptf_exception("Domain does not support the performance control interface.");
	}
}

void PerformanceControlFacade::unlockCapabilities()
{
	if (supportsPerformanceControls())
	{
		m_policyServices.domainPerformanceControl->setPerformanceCapsLock(m_participantIndex, m_domainIndex, false);
	}
	else
	{
		throw dptf_exception("Domain does not support the performance control interface.");
	}
}

const PerformanceControlSet& PerformanceControlFacade::getControls()
{
	return m_performanceControlSetProperty.getPerformanceControlSet();
}

void PerformanceControlFacade::refreshControls()
{
	m_performanceControlSetProperty.refresh();
}

void PerformanceControlFacade::refreshCapabilities()
{
	m_performanceControlCapabilitiesProperty.refresh();
}

const PerformanceControlDynamicCaps& PerformanceControlFacade::getDynamicCapabilities()
{
	return m_performanceControlCapabilitiesProperty.getDynamicCaps();
}

PerformanceControlStatus PerformanceControlFacade::getStatus() const
{
	return PerformanceControlStatus(m_lastIssuedPerformanceControlIndex);
}

PerformanceControlStatus PerformanceControlFacade::getLiveStatus() const
{
	return m_policyServices.domainPerformanceControl->getPerformanceControlStatus(m_participantIndex, m_domainIndex);
}

void PerformanceControlFacade::initializeControlsIfNeeded()
{
	if (supportsPerformanceControls())
	{
		m_policyServices.messageLogging->writeMessageDebug(
			PolicyMessage(FLF, "Performance control initialization started."));
		const PerformanceControlDynamicCaps& caps = getDynamicCapabilities();
		if (m_controlsHaveBeenInitialized == false)
		{
			setControl(caps.getCurrentUpperLimitIndex());
			m_controlsHaveBeenInitialized = true;
		}
		else
		{
			UIntN upperLimit = caps.getCurrentUpperLimitIndex();
			UIntN lowerLimit = caps.getCurrentLowerLimitIndex();
			UIntN lastLimitSet = m_lastIssuedPerformanceControlIndex;
			if (lastLimitSet < upperLimit)
			{
				m_policyServices.messageLogging->writeMessageDebug(
					PolicyMessage(FLF, "Adjusting performance limit to maximum allowed."));
				setControl(upperLimit);
			}
			else if (lastLimitSet > lowerLimit)
			{
				m_policyServices.messageLogging->writeMessageDebug(
					PolicyMessage(FLF, "Adjusting performance limit to minimum allowed."));
				setControl(lowerLimit);
			}
		}
		m_policyServices.messageLogging->writeMessageDebug(
			PolicyMessage(FLF, "Performance control initialization finished."));
	}
}

void PerformanceControlFacade::setControlsToMax()
{
	if (supportsPerformanceControls())
	{
		const PerformanceControlDynamicCaps& caps = getDynamicCapabilities();
		setControl(caps.getCurrentUpperLimitIndex());
	}
}
