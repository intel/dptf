/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

// Use getrandom() for GLIBC 2.25 and higher (Ubuntu 18.04)
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 25))	// getrandom

# include <sys/random.h>
# define os_getrandom(buf, len, flags)	getrandom(buf, len, flags)

#else // Use syscall() to call SYS_getrandom for GLIBC < 2.25 (Ubuntu 16.04)

# include <sys/syscall.h>
# include <linux/random.h>

# if defined(SYS_getrandom) && defined(GRND_NONBLOCK)

#  define os_getrandom(buf, len, flags)	(ssize_t)syscall(SYS_getrandom, buf, len, flags)

# else // Use /dev/urandom for systems without getrandom() or SYS_getrandom support (Ubuntu 14.04)

#  if !defined(GRND_NONBLOCK)
#   define GRND_NONBLOCK 0x0001 // Not supported for reading directly from random devices
#   define GRND_RANDOM   0x0002 // Use Blocking /dev/random instead of /dev/urandom
#  endif
#  include <fcntl.h>

static ESIF_INLINE ssize_t os_getrandom(void *buffer, size_t length, unsigned int flags)
{
	ssize_t bytes = 0;
	int fd = open((flags & GRND_RANDOM ? "/dev/random" : "/dev/urandom"), O_RDONLY);
	if (fd != -1) {
		bytes = read(fd, buffer, length);
		close(fd);
	}
	return bytes;
}

# endif // /dev/urandom

#endif // GLIBC 2.25

static ESIF_INLINE esif_error_t esif_ccb_random(void *buffer, size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (buffer && buf_len) {
		ssize_t ret = os_getrandom(buffer, buf_len, 0);
		if (ret == (ssize_t)buf_len) {
			rc = ESIF_OK;
		}
		else {
			rc = ESIF_E_UNSPECIFIED;
		}
	}
	return rc;
}


#endif /* LINUX USER */
