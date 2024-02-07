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

#include "PerformanceControlSet.h"
#include "XmlNode.h"
#include "EsifDataBinaryPpssPackage.h"
#include "EsifDataBinaryPssPackage.h"
#include "EsifDataBinaryTssPackage.h"
#include "EsifDataBinaryGfxPstateConfig.h"

PerformanceControlSet::PerformanceControlSet(const std::vector<PerformanceControl>& performanceControl)
	: m_performanceControl(performanceControl)
{
}

PerformanceControlSet::PerformanceControlSet()
{
}

PerformanceControlSet PerformanceControlSet::createFromGenericPpss(const DptfBuffer& buffer)
{
	std::vector<PerformanceControl> controls;
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct EsifDataBinaryPpssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty PPSS buffer.");
	}

	UIntN rows = countPpssRows(buffer.size(), data);
	data = reinterpret_cast<UInt8*>(buffer.get());
	currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);

	for (UIntN i = 0; i < rows; i++)
	{
		PerformanceControl performanceControl(
			static_cast<UInt32>(currentRow->control.integer.value),
			PerformanceControlType::PerformanceState,
			static_cast<UInt32>(currentRow->power.integer.value),
			Percentage(static_cast<UInt32>(currentRow->performancePercentage.integer.value) / 100.0),
			static_cast<UInt32>(currentRow->latency.integer.value),
			static_cast<UInt32>(currentRow->rawPerformance.integer.value),
			std::string(
				reinterpret_cast<const char*>(&(currentRow->rawUnits)) + sizeof(union esif_data_variant),
				currentRow->rawUnits.string.length));

		controls.push_back(performanceControl);

		data += sizeof(struct EsifDataBinaryPpssPackage) + currentRow->rawUnits.string.length;
		currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);
	}

	return PerformanceControlSet(controls);
}

PerformanceControlSet PerformanceControlSet::createFromProcessorPss(const DptfBuffer& buffer)
{
	std::vector<PerformanceControl> controls;
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct EsifDataBinaryPssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPssPackage*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty PSS buffer.");
	}

	UIntN rows = buffer.size() / sizeof(EsifDataBinaryPssPackage);

	if (buffer.size() % sizeof(EsifDataBinaryPssPackage))
	{
		// Data size mismatch, should be evenly divisible
		throw dptf_exception(
			"Failed to parse PSS object.  The length of data received does not match the expected \
							 data length.");
	}

	for (UIntN row = 0; row < rows; row++)
	{
		Percentage ratio(1.0);

		if (!controls.empty())
		{
			if (controls.front().getControlAbsoluteValue() != 0)
			{
				ratio =
					(static_cast<UIntN>(
						(100 * currentRow->coreFrequency.integer.value) / controls.front().getControlAbsoluteValue()))
					/ 100.0;
			}
			else
			{
				ratio = (rows - row) / static_cast<double>(rows);
			}
		}

		PerformanceControl performanceControl(
			static_cast<UInt32>(currentRow->control.integer.value),
			PerformanceControlType::PerformanceState,
			static_cast<UInt32>(currentRow->power.integer.value),
			ratio,
			static_cast<UInt32>(currentRow->latency.integer.value),
			static_cast<UInt32>(currentRow->coreFrequency.integer.value),
			std::string("MHz"));
		controls.push_back(performanceControl);

		data += sizeof(struct EsifDataBinaryPssPackage);
		currentRow = reinterpret_cast<struct EsifDataBinaryPssPackage*>(data);
	}
	return PerformanceControlSet(controls);
}

PerformanceControlSet PerformanceControlSet::createFromProcessorTss(PerformanceControl pN, const DptfBuffer& buffer)
{
	std::vector<PerformanceControl> controls;
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct EsifDataBinaryTssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTssPackage*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty TSS buffer.");
	}

	UIntN rows = buffer.size() / sizeof(EsifDataBinaryTssPackage);

	if (buffer.size() % sizeof(EsifDataBinaryTssPackage))
	{
		// Data size mismatch, should be evenly divisible
		throw dptf_exception(
			"Failed to parse TSS object.  The length of data received does not match the expected \
							 data length.");
	}

	for (UIntN i = 0; i < rows; i++)
	{
		Percentage performancePercentage = static_cast<UIntN>(currentRow->performancePercentage.integer.value) / 100.0;

		PerformanceControl tStateControl(
			static_cast<UInt32>(currentRow->control.integer.value),
			PerformanceControlType::ThrottleState,
			static_cast<UInt32>(currentRow->power.integer.value),
			performancePercentage,
			static_cast<UInt32>(currentRow->latency.integer.value),
			static_cast<UIntN>(pN.getControlAbsoluteValue() * performancePercentage),
			pN.getValueUnits());

		if (tStateControl.getControlAbsoluteValue() != 0)
		{
			controls.push_back(tStateControl);
		}

		data += sizeof(struct EsifDataBinaryTssPackage);
		currentRow = reinterpret_cast<struct EsifDataBinaryTssPackage*>(data);
	}
	return PerformanceControlSet(controls);
}

PerformanceControlSet PerformanceControlSet::createFromProcessorGfxPstates(const DptfBuffer& buffer)
{
	std::vector<PerformanceControl> controls;
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct EsifDataBinaryGfxPstateConfig* currentRow = reinterpret_cast<struct EsifDataBinaryGfxPstateConfig*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty Graphics PSS buffer.");
	}

	UIntN rows = buffer.size() / sizeof(EsifDataBinaryGfxPstateConfig);

	if (buffer.size() % sizeof(EsifDataBinaryGfxPstateConfig))
	{
		// Data size mismatch, should be evenly divisible
		throw dptf_exception(
			"Failed to parse Graphics PSS object.  The length of data received does not match the expected \
							 data length.");
	}

	// Reset currentRow to point to the beginning of the data block
	data = reinterpret_cast<UInt8*>(buffer.get());
	currentRow = reinterpret_cast<struct EsifDataBinaryGfxPstateConfig*>(data);

	for (UIntN i = 0; i < rows; i++)
	{
		Percentage* p;

		if (controls.empty())
		{
			p = new Percentage(1.0);
		}
		else
		{
			p = new Percentage(
				(static_cast<UIntN>(
					(100 * currentRow->maxRenderFrequency.integer.value) / controls.front().getControlAbsoluteValue()))
				/ 100.0);
		}

		PerformanceControl gfxPerfControl(
			i, // GFX has no control ID so the index is used.
			PerformanceControlType::PerformanceState,
			Constants::Invalid,
			*p,
			GFX_PSTATE_TRANSITION_LATENCY,
			static_cast<UInt32>(currentRow->maxRenderFrequency.integer.value),
			std::string("MHz"));

		delete p;

		controls.push_back(gfxPerfControl);

		data += sizeof(struct EsifDataBinaryGfxPstateConfig);
		currentRow = reinterpret_cast<struct EsifDataBinaryGfxPstateConfig*>(data);
	}

	return PerformanceControlSet(controls);
}

UIntN PerformanceControlSet::getCount(void) const
{
	return static_cast<UIntN>(m_performanceControl.size());
}

void PerformanceControlSet::append(const PerformanceControlSet& controlSet, UIntN fromIndex)
{
	if (fromIndex >= controlSet.getCount())
	{
		return;
	}

	m_performanceControl.insert(
		m_performanceControl.end(),
		controlSet.m_performanceControl.begin() + fromIndex,
		controlSet.m_performanceControl.end());
}

PerformanceControl PerformanceControlSet::operator[](UIntN index) const
{
	return m_performanceControl.at(index);
}

Bool PerformanceControlSet::operator==(const PerformanceControlSet& rhs) const
{
	return (m_performanceControl == rhs.m_performanceControl);
}

Bool PerformanceControlSet::operator!=(const PerformanceControlSet& rhs) const
{
	return !(*this == rhs);
}

std::shared_ptr<XmlNode> PerformanceControlSet::getXml()
{
	auto root = XmlNode::createWrapperElement("performance_control_set");

	for (UIntN i = 0; i < m_performanceControl.size(); i++)
	{
		root->addChild(m_performanceControl[i].getXml());
	}

	return root;
}

UIntN PerformanceControlSet::countPpssRows(UIntN size, UInt8* data)
{
	IntN bytesRemaining = size;
	UIntN rows = 0;

	struct EsifDataBinaryPpssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);

	while (bytesRemaining > 0)
	{
		bytesRemaining -= sizeof(struct EsifDataBinaryPpssPackage);

		if ((IntN)currentRow->rawUnits.string.length < 0)
		{
			throw dptf_exception("Expected string length invalid. (PPSS)");
		}
		bytesRemaining -= currentRow->rawUnits.string.length;

		if (bytesRemaining >= 0)
		{
			// The math done here will vary based on the number of strings in the BIOS object
			rows++;
			data += sizeof(struct EsifDataBinaryPpssPackage) + currentRow->rawUnits.string.length;
			currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);
		}
		else // Data size mismatch, we went negative
		{
			throw dptf_exception("Expected binary data size mismatch. (PPSS)");
		}
	}

	return rows;
}
