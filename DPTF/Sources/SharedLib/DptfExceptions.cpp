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

#include "DptfExceptions.h"

dptf_exception::dptf_exception(const std::string& description) throw()
    : std::logic_error(description.c_str()), m_description(description)
{
}

dptf_exception::~dptf_exception() throw()
{
}

const char* dptf_exception::what() const throw()
{
    return m_description.c_str();
}

memory_allocation_failure::memory_allocation_failure(const std::string& description) :
    dptf_exception(description)
{
}

buffer_too_small::buffer_too_small(const std::string& description) :
    dptf_exception(description)
{
}

primitive_execution_failed::primitive_execution_failed(const std::string& description) :
    dptf_exception(description)
{
}

not_implemented::not_implemented()
    : dptf_exception("The feature is not implemented.")
{
}

participant_not_enabled::participant_not_enabled()
    : dptf_exception("The participant is not enabled.")
{
}

domain_not_enabled::domain_not_enabled()
    : dptf_exception("The domain is not enabled.")
{
}

policy_index_invalid::policy_index_invalid()
    : dptf_exception("The policy index is not valid.")
{
}

policy_not_in_idsp_list::policy_not_in_idsp_list()
    : dptf_exception("The policy is not in the IDSP list.")
{
}

participant_index_invalid::participant_index_invalid()
    : dptf_exception("The participant index is not valid.")
{
}

duplicate_work_item::duplicate_work_item(const std::string& description) :
    dptf_exception(description)
{
}

implement_me::implement_me()
    : dptf_exception("The feature needs to be implemented.")
{
}