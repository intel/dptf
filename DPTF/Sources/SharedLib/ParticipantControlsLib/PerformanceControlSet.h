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
#include "PerformanceControl.h"

class XmlNode;

class PerformanceControlSet final
{
public:
	PerformanceControlSet();
	PerformanceControlSet(const std::vector<PerformanceControl>& performanceControl);
	static PerformanceControlSet createFromGenericPpss(const DptfBuffer& buffer);
	static PerformanceControlSet createFromProcessorPss(const DptfBuffer& buffer);
	static PerformanceControlSet createFromProcessorTss(PerformanceControl pN, const DptfBuffer& buffer);
	static PerformanceControlSet createFromProcessorGfxPstates(const DptfBuffer& buffer);

	UIntN getCount(void) const;
	void append(const PerformanceControlSet& controlSet, UIntN fromIndex);
	std::shared_ptr<XmlNode> getXml(void);
	PerformanceControl operator[](UIntN index) const;
	Bool operator==(const PerformanceControlSet& rhs) const;
	Bool operator!=(const PerformanceControlSet& rhs) const;

private:
	std::vector<PerformanceControl> m_performanceControl;
	static UIntN countPpssRows(UInt32 size, UInt8* data);
};
