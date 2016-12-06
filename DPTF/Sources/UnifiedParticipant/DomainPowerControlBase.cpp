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

#include "DomainPowerControlBase.h"

DomainPowerControlBase::DomainPowerControlBase(UIntN participantIndex, UIntN domainIndex,
    std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
    : ControlBase(participantIndex, domainIndex, participantServicesInterface),
    m_pl1Enabled(false),
    m_pl2Enabled(false),
    m_pl3Enabled(false),
    m_pl4Enabled(false)
{

}

DomainPowerControlBase::~DomainPowerControlBase()
{

}

void DomainPowerControlBase::updateEnabled(PowerControlType::Type controlType)
{
    // For PL4, use the GET_RAPL_POWER_LIMIT primitive, for PL1/2/3, use
    // GET_RAPL_POWER_LIMIT_ENABLE primitive
    if (PowerControlType::PL4 == controlType)
    {
        try
        {
            getParticipantServices()->primitiveExecuteGetAsPower(
                esif_primitive_type::GET_RAPL_POWER_LIMIT,
                getDomainIndex(),
                (UInt8)controlType);
            // Enable PL4 control if the above primitive execution does not generate an exception
            m_pl4Enabled = true;
        }
        catch (...)
        {
            m_pl4Enabled = false;
        }
    }
    else
    {
        try
        {
            UInt32 plEnabled = getParticipantServices()->primitiveExecuteGetAsUInt32(
                GET_RAPL_POWER_LIMIT_ENABLE, getDomainIndex(), (UInt8)controlType);
            Bool enabled = plEnabled > 0 ? true : false;
            switch (controlType)
            {
            case PowerControlType::PL1:
                m_pl1Enabled = enabled;
                break;
            case PowerControlType::PL2:
                m_pl2Enabled = enabled;
                break;
            case PowerControlType::PL3:
                m_pl3Enabled = enabled;
                break;
            default:
                break;
            }
        }
        catch (...)
        {

        }
    }
}

void DomainPowerControlBase::setEnabled(PowerControlType::Type controlType, Bool enable)
{
    try
    {
        getParticipantServices()->primitiveExecuteSetAsUInt32(
            SET_RAPL_POWER_LIMIT_ENABLE, enable ? (UInt32)1 : (UInt32)0, getDomainIndex(), (UInt8)controlType);
    }
    catch (...)
    {

    }
}

Bool DomainPowerControlBase::isEnabled(PowerControlType::Type controlType) const
{
    switch (controlType)
    {
    case PowerControlType::PL1:
        return m_pl1Enabled;
    case PowerControlType::PL2:
        return m_pl2Enabled;
    case PowerControlType::PL3:
        return m_pl3Enabled;
    case PowerControlType::PL4:
        return m_pl4Enabled;
    default:
        return false;
    }
}
