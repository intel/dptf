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

#include "ActiveControlSet.h"
#include "esif_sdk_fan.h"
#include "XmlNode.h"

ActiveControlSet::ActiveControlSet(const std::vector<ActiveControl>& activeControl) :
    m_activeControl(activeControl)
{
}

ActiveControlSet ActiveControlSet::createFromFps(const DptfBuffer& buffer)
{
    std::vector<ActiveControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    data += sizeof(esif_data_variant); //Ignore revision field
    struct EsifDataBinaryFpsPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFpsPackage*>(data);

    if (buffer.size() == 0)
    {
        throw dptf_exception("Received empty FPS buffer.");
    }

    UIntN rows = (buffer.size() - sizeof(esif_data_variant)) / sizeof(EsifDataBinaryFpsPackage);

    if ((buffer.size() - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryFpsPackage))
    {
        throw dptf_exception("Expected binary data size mismatch. (FPS)");
    }

    for (UIntN i = 0; i < rows; i++)
    {
        ActiveControl temp(
            static_cast<UInt32>(currentRow->control.integer.value),
            static_cast<UInt32>(currentRow->tripPoint.integer.value),  // May want to represent this differently; -1 is MAX_INT for whatever type
            static_cast<UInt32>(currentRow->speed.integer.value),
            static_cast<UInt32>(currentRow->noiseLevel.integer.value),
            static_cast<UInt32>(currentRow->power.integer.value));

        controls.push_back(temp);

        data += sizeof(struct EsifDataBinaryFpsPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryFpsPackage*>(data);
    }

    return ActiveControlSet(controls);
}

UIntN ActiveControlSet::getCount(void) const
{
    return static_cast<UIntN>(m_activeControl.size());
}

ActiveControl ActiveControlSet::operator[](UIntN index) const
{
    return m_activeControl.at(index);
}

Bool ActiveControlSet::operator==(const ActiveControlSet& rhs) const
{
    return (m_activeControl == rhs.m_activeControl);
}

Bool ActiveControlSet::operator!=(const ActiveControlSet& rhs) const
{
    return !(*this == rhs);
}

std::shared_ptr<XmlNode> ActiveControlSet::getXml(void)
{
    auto root = XmlNode::createWrapperElement("active_control_set");

    for (UIntN i = 0; i < m_activeControl.size(); i++)
    {
        root->addChild(m_activeControl[i].getXml());
    }

    return root;
}