/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "ipf_sdk_version.h"
#include "esif_sdk_iface_esif.h"

// Check whether Server SDK supports the given Client SDK Version (NULL=Use Current SDK)
static ESIF_INLINE esif_error_t IpfSdk_VersionCheck(const char *serverSdkVersion, const char *clientSdkVersion)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (serverSdkVersion) {
		clientSdkVersion = (clientSdkVersion ? clientSdkVersion : IPF_SDK_VERSION);
		ipfsdk_version_t server = IpfSdk_VersionFromString(serverSdkVersion);
		ipfsdk_version_t client = IpfSdk_VersionFromString(clientSdkVersion);

		// client == server: OK
		if (client == server) {
			rc = ESIF_OK;
		}
		// client < server: OK if server supports it and handles data and API translation
		else if (client < server) {
			struct {
				ipfsdk_version_t minver;	// Minimum Version Supported
				ipfsdk_version_t maxver;	// Maximum Version Supported
			} supported_releases[] = {
				{ IPFSDK_VERSION(1, 0, 0), IPFSDK_VERSION(1, 0, 0xffff) },	// Support 1.0.x but nothing older
				{ 0 }
			};

			// Allow Supported Versions Only
			rc = ESIF_E_NOT_SUPPORTED;
			for (size_t j = 0; supported_releases[j].minver; j++) {
				if (client >= supported_releases[j].minver && client <= supported_releases[j].maxver) {
					rc = ESIF_OK;
					break;
				}
			}
		}
		// client > server: OK only if Major.Minor are exact match. Client may check revision if it chooses to.
		else if (client > server && IPFSDK_GETRELEASE(client) == IPFSDK_GETRELEASE(server)) {
			rc = ESIF_OK;
		}
		// client > server with Major.Minor mismatch: Not supported
		else {
			rc = ESIF_E_NOT_SUPPORTED;
		}
	}
	return rc;
}

/*
** Check whether Server SDK supports the current Client SDK Version using ESIF Interface command: "sdk-version <client-version>"
** This is intended to be called during an ESIF App's AppCreate() to verify that the Server supports the Client SDK Version
** before proceeding and allow the Client to examine the Server SDK Version or Revision for feature compatibility.
*/
static ESIF_INLINE esif_error_t IpfSdk_VersionCheckEsifIface(
	const esif_handle_t esifHandle,	// ESIF Handle (from ESIF_UF or IPFIPC)
	const EsifInterface esifIface,	// ESIF Interface
	char *buf_ptr,					// Optional Buffer for Server SDK Version
	size_t buf_len					// Optional Buffer Length
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (esifHandle != ESIF_INVALID_HANDLE && esifIface.fSendCommandFuncPtr) {
		static char sdkcmd[] = "sdk-version";
		static char sdkver[] = IPF_SDK_VERSION;
		EsifData argv[] = {
			{ ESIF_DATA_STRING, sdkcmd, sizeof(sdkcmd), sizeof(sdkcmd) },
			{ ESIF_DATA_STRING, sdkver, sizeof(sdkver), sizeof(sdkver) }
		};
		int argc = ESIF_ARRAY_LEN(argv);
		char replybuf[ESIF_NAME_LEN] = {0};
		EsifData response = { ESIF_DATA_STRING };
		if (buf_ptr == NULL || buf_len == 0) {
			buf_ptr = replybuf;
			buf_len = sizeof(replybuf);
		}
		response.buf_ptr = buf_ptr;
		response.buf_len = (u32)buf_len;

		rc = esifIface.fSendCommandFuncPtr(
			esifHandle,
			argc,
			argv,
			&response
		);

		if (rc == ESIF_OK && IpfSdk_VersionFromString(buf_ptr) == 0) {
			*buf_ptr = 0;
			rc = ESIF_E_NOT_SUPPORTED;
		}
	}
	return rc;
}
