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

#include "PlatformPowerControlState.h"

PlatformPowerControlState::PlatformPowerControlState(DomainPlatformPowerControlBase* control)
    : m_control(control),
    m_enablesValid(false),
    m_pl1LimitValid(false), m_pl2LimitValid(false), m_pl3LimitValid(false),
    m_pl1TimeWindowValid(false), m_pl3TimeWindowValid(false),
    m_pl3DutyCycleValid(false)
{

}

PlatformPowerControlState::~PlatformPowerControlState()
{

}

void PlatformPowerControlState::capture()
{
    captureEnables();
    captureLimit(PlatformPowerLimitType::PSysPL1, m_pl1Limit, m_pl1LimitValid);
    captureLimit(PlatformPowerLimitType::PSysPL2, m_pl2Limit, m_pl2LimitValid);
    captureLimit(PlatformPowerLimitType::PSysPL3, m_pl3Limit, m_pl3LimitValid);
    captureTimeWindow(PlatformPowerLimitType::PSysPL1, m_pl1TimeWindow, m_pl1TimeWindowValid);
    captureTimeWindow(PlatformPowerLimitType::PSysPL3, m_pl3TimeWindow, m_pl3TimeWindowValid);
    captureDutyCycle(PlatformPowerLimitType::PSysPL3, m_pl3DutyCycle, m_pl3DutyCycleValid);
}

void PlatformPowerControlState::restore()
{
    restoreLimit(PlatformPowerLimitType::PSysPL1, m_pl1Limit, m_pl1LimitValid);
    restoreLimit(PlatformPowerLimitType::PSysPL2, m_pl2Limit, m_pl2LimitValid);
    restoreLimit(PlatformPowerLimitType::PSysPL3, m_pl3Limit, m_pl3LimitValid);
    restoreTimeWindow(PlatformPowerLimitType::PSysPL1, m_pl1TimeWindow, m_pl1TimeWindowValid);
    restoreTimeWindow(PlatformPowerLimitType::PSysPL3, m_pl3TimeWindow, m_pl3TimeWindowValid);
    restoreDutyCycle(PlatformPowerLimitType::PSysPL3, m_pl3DutyCycle, m_pl3DutyCycleValid);
}

void PlatformPowerControlState::restoreEnables()
{
    if (m_enablesValid)
    {
        m_control->setEnabled(PlatformPowerLimitType::PSysPL1, m_pl1Enabled);
        m_control->setEnabled(PlatformPowerLimitType::PSysPL2, m_pl2Enabled);
        m_control->setEnabled(PlatformPowerLimitType::PSysPL3, m_pl3Enabled);
    }
}

void PlatformPowerControlState::captureEnables()
{
    m_pl1Enabled = m_control->checkEnabled(PlatformPowerLimitType::PSysPL1);
    m_pl2Enabled = m_control->checkEnabled(PlatformPowerLimitType::PSysPL2);
    m_pl3Enabled = m_control->checkEnabled(PlatformPowerLimitType::PSysPL3);
    m_enablesValid = true;
}

void PlatformPowerControlState::captureLimit(
    PlatformPowerLimitType::Type limitType, Power& limit, Bool& limitValid)
{
    try
    {
        limit = m_control->getPlatformPowerLimit(
            m_control->getParticipantIndex(), m_control->getDomainIndex(),
            limitType);
        limitValid = true;
    }
    catch (...)
    {
        limitValid = false;
    }
}

void PlatformPowerControlState::captureTimeWindow(
    PlatformPowerLimitType::Type limitType, TimeSpan& timeWindow, Bool& timeWindowValid)
{
    try
    {
        timeWindow = m_control->getPlatformPowerLimitTimeWindow(
            m_control->getParticipantIndex(), m_control->getDomainIndex(),
            limitType);
        timeWindowValid = true;
    }
    catch (...)
    {
        timeWindowValid = false;
    }
}

void PlatformPowerControlState::captureDutyCycle(
    PlatformPowerLimitType::Type limitType, Percentage& dutyCycle, Bool& dutyCycleValid)
{
    try
    {
        dutyCycle = m_control->getPlatformPowerLimitDutyCycle(
            m_control->getParticipantIndex(), m_control->getDomainIndex(),
            limitType);
        dutyCycleValid = true;
    }
    catch (...)
    {
        dutyCycleValid = false;
    }
}

void PlatformPowerControlState::restoreLimit(
    PlatformPowerLimitType::Type limitType, const Power& limit, const Bool& limitValid)
{
    if (limitValid)
    {
        try
        {
            m_control->setPlatformPowerLimit(
                m_control->getParticipantIndex(), m_control->getDomainIndex(),
                limitType, limit);
        }
        catch (...)
        {
        }
    }
}

void PlatformPowerControlState::restoreTimeWindow(
    PlatformPowerLimitType::Type limitType, const TimeSpan& timeWindow, const Bool& timeWindowValid)
{
    if (timeWindowValid)
    {
        try
        {
            m_control->setPlatformPowerLimitTimeWindow(
                m_control->getParticipantIndex(), m_control->getDomainIndex(),
                limitType, timeWindow);
        }
        catch (...)
        {
        }
    }
}

void PlatformPowerControlState::restoreDutyCycle(
    PlatformPowerLimitType::Type limitType, const Percentage& dutyCycle, const Bool& dutyCycleValid)
{
    if (dutyCycleValid)
    {
        try
        {
            m_control->setPlatformPowerLimitDutyCycle(
                m_control->getParticipantIndex(), m_control->getDomainIndex(),
                limitType, dutyCycle);
        }
        catch (...)
        {
        }
    }
}