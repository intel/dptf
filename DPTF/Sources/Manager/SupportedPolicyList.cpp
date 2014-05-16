/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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
#include "DptfMemory.h"
#include "EsifServices.h"
#include "esif_primitive_type.h"
#include "esif_ccb_memory.h"
#include "esif_data_variant.h"

static const UIntN GuidSize = 16;

#pragma pack(push, 1)
typedef struct _AcpiEsifGuid
{
    union esif_data_variant esifDataVariant;
    UInt8 uuid[GuidSize];
} AcpiEsifGuid;
#pragma pack(pop)

SupportedPolicyList::SupportedPolicyList(DptfManager* dptfManager) :
    m_dptfManager(dptfManager)
{
    createSupportedPolicyList();
}

UIntN SupportedPolicyList::getCount(void) const
{
    return static_cast<UIntN>(m_guid.size());
}

const Guid& SupportedPolicyList::operator[](UIntN index) const
{
    return m_guid.at(index);
}

Bool SupportedPolicyList::isPolicyValid(const Guid& guid) const
{
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
}

void SupportedPolicyList::createSupportedPolicyList(void)
{
    m_guid.clear();

    UInt32 dataLength = 0;
    DptfMemory binaryData;
    binaryData.allocate(Constants::DefaultBufferSize, true);

    m_dptfManager->getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_SUPPORTED_POLICIES,
        ESIF_DATA_BINARY,
        binaryData,
        binaryData.getSize(),
        &dataLength);

    if ((dataLength % sizeof(AcpiEsifGuid)) != 0)
    {
        std::stringstream message;
        message << "Received invalid data length [" << dataLength << "] from primitive call: GET_SUPPORTED_POLICIES";
        throw dptf_exception(message.str());
    }

    UInt32 guidCount = dataLength / sizeof(AcpiEsifGuid);
    AcpiEsifGuid* acpiEsifGuid = reinterpret_cast<AcpiEsifGuid*>(binaryData.getPtr());

    for (UInt32 i = 0; i < guidCount; i++)
    {
        UInt8 guidByteArray[GuidSize] = {0};
        esif_ccb_memcpy(guidByteArray, &acpiEsifGuid[i].uuid, GuidSize);
        Guid guid(guidByteArray);
        m_dptfManager->getEsifServices()->writeMessageInfo("Supported GUID: " + guid.toString());
        m_guid.push_back(guid);
    }

    binaryData.deallocate();
}