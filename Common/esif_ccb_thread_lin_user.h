/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#if defined(ESIF_ATTR_OS_LINUX) && defined(ESIF_ATTR_USER)

#include <pthread.h>
typedef pthread_t esif_thread_t;	/* Non-Unique Thread Handle */
typedef pthread_t esif_thread_id_t; /* Unique Thread ID */

/* Get current unique Thread ID (NOT Thread Handle) */
static ESIF_INLINE esif_thread_id_t esif_ccb_thread_id_current()
{
	return pthread_self(); /* Thread Handle == Thread ID for Linux */
}

/* Convert a non-unique Thread Handle to a unique Thread ID */
static ESIF_INLINE esif_thread_id_t esif_ccb_thread_id(esif_thread_t *threadPtr)
{
	return *threadPtr; /* Thread Handle == Thread ID for Linux */
}

static ESIF_INLINE enum esif_rc esif_ccb_thread_create(
	esif_thread_t *thread_ptr,
	work_func_t function_ptr,
	void *argument_ptr
	)
{
	enum esif_rc rc = ESIF_OK;

	pthread_attr_t attr;		/* Thread Attributes     */
	pthread_attr_init(&attr);	/* Initialize Attributes */

	if (pthread_create(
			thread_ptr,	/* Thread                    */
			&attr,		/* Attributes                */
			function_ptr,	/* Function To Start         */
			argument_ptr	/* Function Arguments If Any */
			) != 0) {
		rc = ESIF_E_UNSPECIFIED;
	}

	THREAD_DEBUG(
		"thread = %p entry function = %p entry args = %p rc=%s(%d)\n",
		thread_ptr,
		function_ptr,
		argument_ptr,
		esif_rc_str(rc),
		rc);

	pthread_attr_destroy(&attr);
	return rc;
}

/* wait for a thread to complete */
static ESIF_INLINE enum esif_rc esif_ccb_thread_join (
	esif_thread_t *thread_ptr
	)
{
	enum esif_rc rc = ESIF_OK;

	if (thread_ptr && esif_ccb_thread_id(thread_ptr)) {
		pthread_join(*thread_ptr, NULL);
	} else {
		rc = ESIF_E_PARAMETER_IS_NULL;
	}

	THREAD_DEBUG(
		"joined thread = %p rc=%s(%d)\n",
		thread_ptr,
		esif_rc_str(rc),
		rc);

	return rc;
}

#endif /* LINUX USER */
