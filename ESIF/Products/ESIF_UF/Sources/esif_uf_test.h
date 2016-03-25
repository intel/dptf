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

#ifndef _ESIF_UF_TEST_
#define _ESIF_UF_TEST_

#define ESIF_TEST_PASSED "ETFTestPassed"
#define ESIF_TEST_FAILED "ETFTestFailed"
#define ESIF_TEST_SKIPPED "ETFTestSkipped"

/* Enumerate Test Errors */
typedef enum _t_EsifTestErrorType {
	ESIF_TEST_OK = 0,
	ESIF_TEST_SKIP,
	ESIF_TEST_E_HELP,
	ESIF_TEST_E_NEED_ARG,
	ESIF_TEST_E_BINARY,
	ESIF_TEST_E_OUT_OF_BOUNDS,
	ESIF_TEST_E_FILE_NOT_FOUND,
	ESIF_TEST_E_FILE_SIZE_DIFFER,
	ESIF_TEST_E_FILE_COMPARE,
	ESIF_TEST_E_NO_MEMORY
} eEsifTestErrorType;

/* Error To String */
static ESIF_INLINE EsifString EsifTestErrorStr(eEsifTestErrorType type)
{
	switch (type) {
	ESIF_CASE(ESIF_TEST_OK, "Tests executed properly");
	ESIF_CASE(ESIF_TEST_SKIP, "Skip test");
	ESIF_CASE(ESIF_TEST_E_HELP, "A help function was triggered");
	ESIF_CASE(ESIF_TEST_E_NEED_ARG, "Exited due to a lack of arguments");
	ESIF_CASE(ESIF_TEST_E_BINARY, "Test was for a binary.  check file output");
	ESIF_CASE(ESIF_TEST_E_OUT_OF_BOUNDS, "Test produced out of bounds results");
	ESIF_CASE(ESIF_TEST_E_FILE_NOT_FOUND, "Comparison file not found");
	ESIF_CASE(ESIF_TEST_E_FILE_SIZE_DIFFER, "Comparison and reference size differ");
	ESIF_CASE(ESIF_TEST_E_FILE_COMPARE, "Contents of buffer/file differ");
	ESIF_CASE(ESIF_TEST_E_NO_MEMORY, "Memory allocation failure");
	}
	return ESIF_NOT_AVAILABLE;
}


#ifdef __cplusplus
extern "C" {
#endif

eEsifTestErrorType EsifTestPrimitive(const UInt32 primitive, const UInt16 qualifier, const UInt8 instance, const UInt32 value, const int argc,
									 EsifString *argv);

eEsifTestErrorType EsifTestPrimitiveBinary(const UInt32 primitive,
										   const UInt16 qualifier,
										   const UInt8 instance,
										   const UInt8 *dataPtr,
										   const UInt16 dataLen,
										   const int argc,
										   EsifString *argv);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_TEST_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

