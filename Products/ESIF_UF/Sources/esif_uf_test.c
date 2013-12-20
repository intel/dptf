/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

// #define ESIF_TRACE_DEBUG_DISABLED

#include "esif_uf.h"
#include "esif_uf_test.h"
#include "esif_uf_version.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Version */
const EsifString g_esif_etf_version = ESIF_UF_VERSION;

#define TEST_DEBUG printf

/* Upper and Lower Bound Test */
static eEsifTestErrorType TestPrimitiveBounds (
	const UInt32 ub,
	const UInt32 lb,
	const UInt32 value
	)
{
	ESIF_TRACE_DEBUG("%s:\n", ESIF_FUNC);

	if (value >= lb && value <= ub) {
		return ESIF_TEST_OK;
	} else {
		ESIF_TRACE_DEBUG("ub %u lb %u value %u\n", ub, lb, value);
		return ESIF_TEST_E_OUT_OF_BOUNDS;
	}
}


/* Test Primitive Result */
eEsifTestErrorType EsifTestPrimitive (
	const UInt32 primitive,
	const UInt16 qualifier,
	const UInt8 instance,
	const UInt32 value,
	const int argc,
	EsifString *argv
	)
{
	UInt32 lb = 0;
	UInt32 ub = 0;
	int j;

	UNREFERENCED_PARAMETER(primitive);
	UNREFERENCED_PARAMETER(qualifier);
	UNREFERENCED_PARAMETER(instance);

	ESIF_TRACE_DEBUG("%s: argc count %d\n", ESIF_FUNC, argc);

	// parse command line options: -l## -u##
	for (j = 0; j < argc; j++) {
		int option   = 0;
		char *optarg = 0;

		// ESIF_TRACE_DEBUG("%s: argc = %d argv = %p argv = %s\n", ESIF_FUNC, j, argv[j], argv[j]);

		if (argv[j][0] == '-') {
			option = argv[j][1];
			optarg = &argv[j][2];
			if (*optarg == 0 && j + 1 < argc) {	// separte arguments: -o ##
				optarg = argv[++j];
			}
		}
		switch (option) {
		case 'l':
			lb = esif_atoi(optarg);
			ESIF_TRACE_DEBUG("testp_lower_bound %u\n", lb);
			break;

		case 'u':
			ub = esif_atoi(optarg);
			ESIF_TRACE_DEBUG("testp_upper_bound %u\n", ub);
			break;

		default:
			return ESIF_TEST_SKIP;

			break;
		}
	}
	return TestPrimitiveBounds(ub, lb, value);
}


/* Test Binary Primitive Result Compares Result To Reference */
eEsifTestErrorType EsifTestPrimitiveBinary (
	const UInt32 primitive,
	const UInt16 qualifier,
	const UInt8 instance,
	const UInt8 *dataPtr,
	const UInt16 dataLen,
	const int argc,
	EsifString *argv
	)
{
	eEsifTestErrorType rc = ESIF_TEST_OK;
	int j;

	UNREFERENCED_PARAMETER(primitive);
	UNREFERENCED_PARAMETER(qualifier);
	UNREFERENCED_PARAMETER(instance);

	ESIF_TRACE_DEBUG("%s:\n", ESIF_FUNC);

	// parse command line options: -b##
	for (j = 0; j < argc; j++) {
		int option   = 0;
		char *optarg = 0;
		if (argv[j][0] == '-') {
			option = argv[j][1];
			optarg = &argv[j][2];
			if (*optarg == 0 && j + 1 < argc) {	// separte arguments: -o ##
				optarg = argv[++j];
			}
		}
		switch (option) {
		case 'b':
		{
			char full_path[ESIF_PATH_LEN];
			FILE *fp_ptr = NULL;
			// esif_ccb_sprintf(ESIF_PATH_LEN, full_path, "..%sbin%s%s", ESIF_PATH_SEP, ESIF_PATH_SEP, optarg);
			esif_ccb_sprintf(ESIF_PATH_LEN, full_path, "%s", esif_build_path(full_path, ESIF_PATH_LEN, ESIF_DIR_BIN, optarg));

			ESIF_TRACE_DEBUG("testp_compare: %s to primitive response %u bytes\n",
							 full_path, dataLen);

			esif_ccb_fopen(&fp_ptr, full_path, (char*)"rb");
			if (fp_ptr) {
				UInt32 ref_size = 0;
				struct stat ref_file_stat = {0};

				esif_ccb_stat(full_path, &ref_file_stat);
				ref_size = ref_file_stat.st_size;

				ESIF_TRACE_DEBUG("testp_compare: primitive bytes = %u reference bytes = %u\n",
								 dataLen, ref_size);

				if (dataLen != ref_size) {
					fclose(fp_ptr);
					return ESIF_TEST_E_FILE_SIZE_DIFFER;
				} else {
					size_t bytes_read   = 0;
					UInt8 *ref_file_buf = (UInt8*)esif_ccb_malloc(ref_size);
					if (NULL == ref_file_buf) {
						fclose(fp_ptr);
						return ESIF_TEST_E_NO_MEMORY;
					}

					bytes_read = fread(ref_file_buf, 1, ref_size, fp_ptr);
					if (bytes_read != ref_size) {
						fclose(fp_ptr);
						esif_ccb_free(ref_file_buf);
						return ESIF_TEST_E_FILE_SIZE_DIFFER;
					}

					ESIF_TRACE_DEBUG("testp_compare: reference bytes read = %u\n",
									 (int)bytes_read);

					if (memcmp(dataPtr, ref_file_buf, ref_size) == 0) {
						ESIF_TRACE_DEBUG("testp_compare: SUCCESS\n");
					} else {
						ESIF_TRACE_DEBUG("testp_compare: FAILED\n");
						rc = ESIF_TEST_E_FILE_COMPARE;
					}

					esif_ccb_free(ref_file_buf);
					fclose(fp_ptr);
				}
			} else {
				rc = ESIF_TEST_E_FILE_NOT_FOUND;
			}
			break;
		}

		default:
			return ESIF_TEST_SKIP;

			break;
		}
	}
	return rc;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
