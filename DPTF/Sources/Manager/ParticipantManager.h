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

#pragma once

#include "Dptf.h"
#include "Participant.h"
#include "esif_uf_app_iface.h"

class DptfManager;

class ParticipantManager
{
public:

    ParticipantManager(DptfManager* dptfManager);
    ~ParticipantManager(void);

    void allocateParticipant(UIntN* newParticipantIndex);
    void createParticipant(UIntN participantIndex, const AppParticipantDataPtr participantDataPtr,
        Bool participantEnabled);

    void destroyAllParticipants(void);
    void destroyParticipant(UIntN participantIndex);

    UIntN getParticipantListCount(void) const;
    Participant* getParticipantPtr(UIntN participantIndex);

    // This will clear the cached data stored within all participants *within* the framework.  It will not ask the
    // actual participants to clear their caches.
    void clearAllParticipantCachedData();

    std::string GetStatusAsXml(void);

private:

    // hide the copy constructor and assignment operator.
    ParticipantManager(const ParticipantManager& rhs);
    ParticipantManager& operator=(const ParticipantManager& rhs);

    DptfManager* m_dptfManager;
    std::vector<Participant*> m_participant;
};