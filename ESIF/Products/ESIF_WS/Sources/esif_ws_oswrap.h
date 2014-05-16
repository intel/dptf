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

#ifndef _ESIF_CCB_OSWRAP_H_
#define _ESIF_CCB_OSWRAP_H_


#ifdef ESIF_ATTR_OS_WINDOWS
	#include <io.h>
	#define esif_ccb_dup(a)          _dup(a)
	#define esif_ccb_dup2(a, b)       _dup2(_fileno(a), b)
	#define esif_ccb_dup3(a, b)       _dup2(a, b)
	#define esif_ccb_flushall()      _flushall()

#else
	#include <unistd.h>
	#define esif_ccb_dup(a)          dup(a)
	#define esif_ccb_dup2(a, b)       dup2(fileno(a), b)
	#define esif_ccb_dup3(a, b)       dup2(a, b)
	#define esif_ccb_flushall()      fflush(NULL);
#endif

#endif /*_ESIF_CCB_OSWRAP_H_ */