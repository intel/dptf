/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

dptf_exception::dptf_exception(const std::string& description) noexcept
	: std::logic_error(description.c_str())
	, m_description(description)
{
}

const char* dptf_exception::what() const noexcept
{
	return m_description.c_str();
}

std::string dptf_exception::getDescription() const
{
	return m_description;
}

temperature_out_of_range::temperature_out_of_range(const std::string& description)
	: dptf_exception(description)
{
}

memory_allocation_failure::memory_allocation_failure(const std::string& description)
	: dptf_exception(description)
{
}

file_open_create_failure::file_open_create_failure(const std::string& description)
	: dptf_exception(description)
{
}

buffer_too_small::buffer_too_small(const std::string& description)
	: dptf_exception(description)
{
}

primitive_execution_failed::primitive_execution_failed(const std::string& description)
	: dptf_exception(description)
{
}

primitive_try_again::primitive_try_again(const std::string& description)
	: dptf_exception(description)
{
}

primitive_destination_unavailable::primitive_destination_unavailable(const std::string& description)
	: dptf_exception(description)
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

domain_control_nullptr::domain_control_nullptr()
	: dptf_exception("The Domain Control is Null.")
{
}

policy_index_invalid::policy_index_invalid()
	: dptf_exception("The policy index is not valid.")
{
}

policy_already_exists::policy_already_exists()
	: dptf_exception("The policy instance already exists.")
{
}

policy_not_in_idsp_list::policy_not_in_idsp_list()
	: dptf_exception("The policy is not in the IDSP list.")
{
}

dynamic_policy_template_guid_invalid::dynamic_policy_template_guid_invalid()
	: dptf_exception("The dynamic policy template guid is not valid.")
{
}

participant_index_invalid::participant_index_invalid()
	: dptf_exception("The participant index is not valid.")
{
}

unsupported_result_temp_type::unsupported_result_temp_type()
	: dptf_exception("The result temperature type after temperature conversion is not supported.")
{
}

duplicate_work_item::duplicate_work_item(const std::string& description)
	: dptf_exception(description)
{
}

implement_me::implement_me()
	: dptf_exception("The feature needs to be implemented.")
{
}

primitive_not_found_in_dsp::primitive_not_found_in_dsp(const std::string& description)
	: dptf_exception(description)
{
}

acpi_object_not_found::acpi_object_not_found(const std::string& description)
	: dptf_exception(description)
{
}

invalid_data_type::invalid_data_type(const std::string& description)
	: dptf_exception(description)
{
}

invalid_data::invalid_data(const std::string& description)
	: dptf_exception(description)
{
}

dptf_out_of_range::dptf_out_of_range(const std::string& description)
	: dptf_exception(description)
{
}

command_failure::command_failure(esif_error_t errorCode, const std::string& description)
	: dptf_exception(description)
	, m_errorCode(errorCode)
{
}

esif_error_t command_failure::getErrorCode() const
{
	return m_errorCode;
}
