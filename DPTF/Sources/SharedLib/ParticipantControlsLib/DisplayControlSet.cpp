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

#include "DisplayControlSet.h"
#include "EsifDataBinaryBclPackage.h"
#include "XmlNode.h"
#include <algorithm>

DisplayControlSet::DisplayControlSet(const std::vector<DisplayControl>& displayControl)
	: m_displayControl(displayControl)
{
}

DisplayControlSet DisplayControlSet::createFromBcl(const DptfBuffer& buffer)
{
	std::vector<DisplayControl> controls;
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct EsifDataBinaryBclPackage* currentRow = reinterpret_cast<struct EsifDataBinaryBclPackage*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty BCL buffer.");
	}

	UIntN rows = buffer.size() / sizeof(EsifDataBinaryBclPackage);

	if (buffer.size() % sizeof(EsifDataBinaryBclPackage))
	{
		throw dptf_exception("Expected binary data size mismatch. (BCL)");
	}

	for (UIntN i = 0; i < rows; i++)
	{
		DisplayControl temp(Percentage(static_cast<UInt32>(currentRow->brightnessLevel.integer.value) / 100.0));

		controls.push_back(temp);

		data += sizeof(struct EsifDataBinaryBclPackage);
		currentRow = reinterpret_cast<struct EsifDataBinaryBclPackage*>(data);
	}

	std::sort(controls.begin(), controls.end());
	controls.erase(std::unique(controls.begin(), controls.end()), controls.end()); // remove duplicates
	std::reverse(controls.begin(), controls.end());

	return DisplayControlSet(controls);
}

UIntN DisplayControlSet::getCount(void) const
{
	return static_cast<UIntN>(m_displayControl.size());
}

DisplayControl DisplayControlSet::operator[](UIntN index) const
{
	return m_displayControl.at(index);
}

Bool DisplayControlSet::operator==(const DisplayControlSet& rhs) const
{
	return (m_displayControl == rhs.m_displayControl);
}

Bool DisplayControlSet::operator!=(const DisplayControlSet& rhs) const
{
	return !(*this == rhs);
}

UIntN DisplayControlSet::getControlIndex(Percentage brightness)
{
	if (m_displayControl.empty())
	{
		throw dptf_exception("Cannot get control index.  Display control set is empty.");
	}
	else
	{
		// check for brightness being at or between any of the rows in the set
		for (UIntN i = 0; i < m_displayControl.size(); i++)
		{
			if (m_displayControl.at(i).getBrightness() == brightness)
			{
				return i;
			}
			else if (m_displayControl.at(i).getBrightness() < brightness)
			{
				return (i == 0) ? 0 : (i - 1);
			}
		}

		// if brightness is not inside the table, it must be below, and thus the best we can do is
		// return the last element
		return (UIntN)(m_displayControl.size() - 1);
	}
}

std::shared_ptr<XmlNode> DisplayControlSet::getXml(void)
{
	auto root = XmlNode::createWrapperElement("display_control_set");

	for (UIntN i = 0; i < m_displayControl.size(); i++)
	{
		root->addChild(m_displayControl[i].getXml());
	}

	return root;
}
