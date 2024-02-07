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

// Manages string spaces for use when creating HTML or XML tag hierarchies.
// Refer to the StringSpaces test for examples.
class Indent
{
public:
	Indent();
	Indent(UIntN length, UIntN stepSize);

	friend std::ostream& operator<<(std::ostream& out, const Indent& indent);

	Indent& operator++(); // prefix
	Indent operator++(int); // postfix
	Indent& operator--(); // prefix
	Indent operator--(int); // postfix

private:

	// make copy constructor and assignment operator private only
	Indent(const Indent& indent);
	Indent& operator=(const Indent& rhs);

	std::string emit() const;
	
	UIntN m_length;
	UIntN m_stepSize;
};
