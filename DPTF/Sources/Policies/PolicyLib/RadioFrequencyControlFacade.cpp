/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "RadioFrequencyControlFacade.h"
#include "StatusFormat.h"
#include "PolicyLogger.h"

using namespace std;
using namespace StatusFormat;

RadioFrequencyControlFacade::RadioFrequencyControlFacade(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_policyServices(policyServices)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_domainProperties(domainProperties)
	, m_rfProfileData(participantIndex, domainIndex, domainProperties, policyServices)
{
}

RadioFrequencyControlFacade::~RadioFrequencyControlFacade()
{
}

Bool RadioFrequencyControlFacade::supportsRfControls()
{
	return m_domainProperties.implementsRfProfileControlInterface();
}

Bool RadioFrequencyControlFacade::supportsStatus()
{
	return m_domainProperties.implementsRfProfileStatusInterface();
}

RfProfileDataSet RadioFrequencyControlFacade::getRadioProfile()
{
	throwIfStatusNotSupported();
	return m_rfProfileData.getRadioProfile();
}

void RadioFrequencyControlFacade::invalidateProfileData()
{
	throwIfStatusNotSupported();
	m_rfProfileData.invalidate();
}

void RadioFrequencyControlFacade::setOperatingFrequency(Frequency frequency)
{
	throwIfControlNotSupported();
	m_policyServices.domainRfProfileControl->setRfProfileCenterFrequency(m_participantIndex, m_domainIndex, frequency);
	m_lastSetFrequency = frequency;
}

Percentage RadioFrequencyControlFacade::getSSC()
{
	return m_ssc;
}

void RadioFrequencyControlFacade::setSSC(Percentage ssc)
{
	m_ssc = ssc;
}

std::shared_ptr<XmlNode> RadioFrequencyControlFacade::getXml()
{
	Frequency centerFrequency(0);
	Frequency minFrequency(0);
	Frequency maxFrequency(0);

	try
	{
		auto rfProfileCaps = m_policyServices.domainRfProfileControl->getRfProfileCapabilities(m_participantIndex, m_domainIndex);
		centerFrequency = rfProfileCaps.getCenterFrequency();
		minFrequency = rfProfileCaps.getMinFrequency();
		maxFrequency = rfProfileCaps.getMaxFrequency();
	}
	catch (...)
	{
		POLICY_LOG_MESSAGE_DEBUG({
			return "Failed to get center frequency";
			});
	}
	auto control = XmlNode::createWrapperElement("radio_frequency_control");
	control->addChild(XmlNode::createDataElement("supports_status_controls", supportsStatus() ? "true" : "false"));
	control->addChild(XmlNode::createDataElement("supports_set_controls", supportsRfControls() ? "true" : "false"));
	control->addChild(XmlNode::createDataElement("min_frequency", minFrequency.toString()));
	control->addChild(XmlNode::createDataElement("center_frequency", centerFrequency.toString()));
	control->addChild(XmlNode::createDataElement("requested_frequency", m_lastSetFrequency.toString()));
	control->addChild(XmlNode::createDataElement("max_frequency", maxFrequency.toString()));
	control->addChild(XmlNode::createDataElement("ssc", m_ssc.toString()));
	if (supportsStatus())
	{
		try
		{
			control->addChild(getRadioProfile().getXml());
		}
		catch (...)
		{
		}
	}
	return control;
}

void RadioFrequencyControlFacade::throwIfStatusNotSupported()
{
	if (supportsStatus() == false)
	{
		throw dptf_exception("Radio frequency status is not supported.");
	}
}

void RadioFrequencyControlFacade::throwIfControlNotSupported()
{
	if (supportsRfControls() == false)
	{
		throw dptf_exception("Radio frequency control is not supported.");
	}
}

const PolicyServicesInterfaceContainer& RadioFrequencyControlFacade::getPolicyServices() const
{
	return m_policyServices;
}

Percentage RadioFrequencyControlFacade::getSscBaselineSpreadValue(UIntN participantIndex, UIntN domainIndex)
{
	return m_policyServices.domainRfProfileControl->getSscBaselineSpreadValue(participantIndex, domainIndex);
}

Percentage RadioFrequencyControlFacade::getSscBaselineThreshold(UIntN participantIndex, UIntN domainIndex)
{
	return m_policyServices.domainRfProfileControl->getSscBaselineThreshold(participantIndex, domainIndex);
}

Percentage RadioFrequencyControlFacade::getSscBaselineGuardBand(UIntN participantIndex, UIntN domainIndex)
{
	return m_policyServices.domainRfProfileControl->getSscBaselineGuardBand(participantIndex, domainIndex);
}