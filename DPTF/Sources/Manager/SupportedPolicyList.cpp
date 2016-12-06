/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "SupportedPolicyList.h"
#include "EsifServicesInterface.h"
#include "esif_ccb_memory.h"
#include "esif_sdk_primitive_type.h"
#include "esif_sdk_data.h"

static const UIntN GuidSize = 16;

#pragma pack(push, 1)
typedef struct _AcpiEsifGuid
{
    union esif_data_variant esifDataVariant;
    UInt8 uuid[GuidSize];
} AcpiEsifGuid;
#pragma pack(pop)

SupportedPolicyList::SupportedPolicyList(DptfManagerInterface* dptfManager) :
    m_dptfManager(dptfManager)
{
    update();
}

UIntN SupportedPolicyList::getCount(void) const
{
    return static_cast<UIntN>(m_guid.size());
}

Guid SupportedPolicyList::operator[](UIntN index) const
{
    return m_guid.at(index);
}

Bool SupportedPolicyList::isPolicyValid(const Guid& guid) const
{
#ifdef DISABLE_VALID_POLICY_CHECK

    return true;

#else

    Bool supported = false;

    for (auto it = m_guid.begin(); it != m_guid.end(); it++)
    {
        if (*it == guid)
        {
            supported = true;
            break;
        }
    }

    return supported;

#endif
}

void SupportedPolicyList::update(void)
{
    m_guid.clear();

    DptfBuffer buffer = m_dptfManager->getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_SUPPORTED_POLICIES, ESIF_DATA_BINARY);

    if ((buffer.size() % sizeof(AcpiEsifGuid)) != 0)
    {
        std::stringstream message;
        message << "Received invalid data length [" << buffer.size() << "] from primitive call: GET_SUPPORTED_POLICIES";
        throw dptf_exception(message.str());
    }

    UInt32 guidCount = buffer.size() / sizeof(AcpiEsifGuid);
    AcpiEsifGuid* acpiEsifGuid = reinterpret_cast<AcpiEsifGuid*>(buffer.get());

    for (UInt32 i = 0; i < guidCount; i++)
    {
        UInt8 guidByteArray[GuidSize] = {0};
        esif_ccb_memcpy(guidByteArray, &acpiEsifGuid[i].uuid, GuidSize);
        Guid guid(guidByteArray);
        m_dptfManager->getEsifServices()->writeMessageInfo("Supported GUID: " + guid.toString());
        m_guid.push_back(guid);
    }
}