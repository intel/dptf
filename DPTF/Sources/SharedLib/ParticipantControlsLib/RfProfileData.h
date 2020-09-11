/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "RfProfileSupplementalData.h"
#include "XmlNode.h"

class XmlNode;

class RfProfileData final
{
public:
	RfProfileData(
		Frequency centerFrequency,
		Frequency leftFrequencySpread,
		Frequency rightFrequencySpread,
		Frequency guardband,
		RfProfileSupplementalData supplementalData);

	Frequency getCenterFrequency(void) const;
	Frequency getLeftFrequencySpread(void) const;
	Frequency getRightFrequencySpread(void) const;
	Frequency getLeftFrequency(void) const;
	Frequency getRightFrequency(void) const;
	Frequency getBandFrequencySpread(void) const;
	Frequency getGuardband(void) const;
	Frequency getLeftFrequencySpreadWithGuardband(void) const;
	Frequency getRightFrequencySpreadWithGuardband(void) const;
	Frequency getLeftFrequencyWithGuardband(void) const;
	Frequency getRightFrequencyWithGuardband(void) const;
	RfProfileSupplementalData getSupplementalData(void) const;

	Bool operator==(const RfProfileData& rhs) const;
	Bool operator!=(const RfProfileData& rhs) const;
	// Bool operator<(const RfProfileData& rhs) const;
	std::shared_ptr<XmlNode> getXml(void) const;

private:
	Frequency m_centerFrequency;
	Frequency m_leftFrequencySpread;
	Frequency m_rightFrequencySpread;
	Frequency m_guardband;
	RfProfileSupplementalData m_supplementalData;
};
