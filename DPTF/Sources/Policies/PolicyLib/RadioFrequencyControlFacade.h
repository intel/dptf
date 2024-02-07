/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#pragma once

#include "Dptf.h"
#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"
#include "XmlNode.h"
#include "RfStatusCachedProperty.h"
#include "RfProfileDataSet.h"

// this facade class provides a simpler interface on top of Radio Frequency controls as well as combines all of the
// Radio Frequency control properties and capabilities into a single class.  these properties also have the ability to
// be cached.
class dptf_export RadioFrequencyControlFacade
{
public:
	RadioFrequencyControlFacade(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~RadioFrequencyControlFacade();

	// controls
	Bool supportsRfControls();
	void setOperatingFrequency(Frequency frequency);
	Percentage getSSC();
	void setSSC(Percentage ssc);
	Percentage getSscBaselineSpreadValue(UIntN participantIndex, UIntN domainIndex);
	Percentage getSscBaselineThreshold(UIntN participantIndex, UIntN domainIndex);
	Percentage getSscBaselineGuardBand(UIntN participantIndex, UIntN domainIndex);

	// status
	Bool supportsStatus();
	RfProfileDataSet getRadioProfile();
	void invalidateProfileData();

	// properties
	std::shared_ptr<XmlNode> getXml();

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;

	// domain properties
	UIntN m_participantIndex;
	UIntN m_domainIndex;
	DomainProperties m_domainProperties;

	// control properties
	RfStatusCachedProperty m_rfProfileData;
	Frequency m_lastSetFrequency;
	Percentage m_ssc;
	void throwIfControlNotSupported();
	void throwIfStatusNotSupported();

	const PolicyServicesInterfaceContainer& getPolicyServices() const;
};
