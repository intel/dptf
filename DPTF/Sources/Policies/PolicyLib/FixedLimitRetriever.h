/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "LimitRetrieverBase.h"

class dptf_export FixedLimitRetriever : public LimitRetrieverBase
{
public:
	FixedLimitRetriever(UInt32 fixedLimit);
	virtual ~FixedLimitRetriever(void);
	virtual UInt32 getControlLimit(void) const;
	virtual std::string ToString(void) const;

private:
	// hide the copy constructor and assignment operator.
	FixedLimitRetriever(const FixedLimitRetriever& retriever);
	FixedLimitRetriever &operator=(const FixedLimitRetriever &retriever);
	
	UInt32 m_fixedLimit;
};
