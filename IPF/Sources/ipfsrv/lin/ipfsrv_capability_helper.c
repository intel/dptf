/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "esif_ccb_rc.h"
// Feature Flag to enable dynamic capability feature
#ifdef ESIF_FEAT_OPT_DYNAMIC_CAPABILITY
#include <sys/capability.h>

eEsifError enable_chown_capability()
{
	eEsifError rc = ESIF_OK;
	cap_t caps = NULL;
	cap_value_t capToEnable = CAP_CHOWN;

	if (!CAP_IS_SUPPORTED(capToEnable)) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	// Get the current Capability set
	caps = cap_get_proc();
	if (caps == NULL) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	// Set that specific capability from effective capability set
	if (cap_set_flag(caps, CAP_EFFECTIVE, 1, &capToEnable, CAP_SET) == -1) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	// Set the new cleared effective capability set
	if (cap_set_proc(caps) == -1) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

exit:
	if ( caps != NULL) {
		cap_free(caps);
	}
	return rc;
}

eEsifError disable_chown_capability()
{
	eEsifError rc = ESIF_OK;
	cap_t caps = NULL;
	cap_value_t capToEnable = CAP_CHOWN;

	if (!CAP_IS_SUPPORTED(capToEnable)) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	// Get the current Capability set
	caps = cap_get_proc();
	if (caps == NULL) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	// Clear that specific capability from effective capability set
	if (cap_set_flag(caps, CAP_EFFECTIVE, 1, &capToEnable, CAP_CLEAR) == -1) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	// Set the new cleared effective capability set
	if (cap_set_proc(caps) == -1) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

exit:
	if ( caps != NULL) {
		cap_free(caps);
	}
	return rc;
}

#else // Feature Flag disabled

eEsifError enable_chown_capability()
{
	return ESIF_OK;
}
eEsifError disable_chown_capability()
{
	return ESIF_OK;
}

#endif // End of Feature Flag