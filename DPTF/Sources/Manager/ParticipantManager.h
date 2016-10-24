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

#pragma once

#include "ParticipantManagerInterface.h"

class ParticipantManager : public ParticipantManagerInterface
{
public:

    ParticipantManager(DptfManagerInterface* dptfManager);
    ~ParticipantManager(void);

    virtual void allocateParticipant(UIntN* newParticipantIndex) override;
    virtual void createParticipant(UIntN participantIndex, const AppParticipantDataPtr participantDataPtr,
        Bool participantEnabled) override;

    virtual void destroyAllParticipants(void) override;
    virtual void destroyParticipant(UIntN participantIndex) override;

    virtual UIntN getParticipantListCount(void) const override;
    virtual Participant* getParticipantPtr(UIntN participantIndex) const override;

    // This will clear the cached data stored within all participants *within* the framework.  It will not ask the
    // actual participants to clear their caches.
    virtual void clearAllParticipantCachedData() override;

    virtual std::string GetStatusAsXml(void) override;

private:

    // hide the copy constructor and assignment operator.
    ParticipantManager(const ParticipantManager& rhs);
    ParticipantManager& operator=(const ParticipantManager& rhs);

    DptfManagerInterface* m_dptfManager;
    std::vector<Participant*> m_participant;
};