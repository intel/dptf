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
#include "DptfManager.h"

class dptf_export DynamicIdspTableEntry
{
public:
	DynamicIdspTableEntry(const std::string& uuid, const std::string& uuidTempalate, const std::string& name);
	virtual ~DynamicIdspTableEntry();

	Bool isSameAs(const DynamicIdspTableEntry& dynamicPolicyTableEntry) const;
	Guid getUuid() const;
	Guid getTemplateGuid() const;
	std::string getName() const;
	std::string getUuidString() const;

private:
	Guid m_uuid;
	Guid m_templateGuid;
	std::string m_name;
	std::string m_uuidString;
};

class dptf_export ISupportedDynamicPolicyList
{
public:
	virtual ~ISupportedDynamicPolicyList(){};
	virtual UIntN getCount(void) const = 0;
	virtual DynamicIdspTableEntry get(UIntN index) const = 0;
	virtual void update(void) = 0;
};

class dptf_export SupportedDynamicPolicyList : public ISupportedDynamicPolicyList
{
public:
	SupportedDynamicPolicyList(DptfManagerInterface* dptfManager);
	virtual ~SupportedDynamicPolicyList(){};
	virtual UIntN getCount(void) const override;
	virtual DynamicIdspTableEntry get(UIntN index) const override;
	virtual void update(void) override;

private:
	DptfManagerInterface* m_dptfManager;
	std::vector<DynamicIdspTableEntry> m_dynamicPolicyList;
	EsifServicesInterface* getEsifServices() const;

	static std::vector<DynamicIdspTableEntry> parseBufferForDynamicPolicyUuids(const DptfBuffer& buffer);
	static UIntN countRows(UInt32 size, UInt8* data);
	static void throwIfOutOfRange(IntN bytesRemaining);
	static void throwIfInvalidUuidLength(std::string uuid);
};
