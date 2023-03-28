################################################################################
## Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
##
## Licensed under the Apache License, Version 2.0 (the "License"); you may not 
## use this file except in compliance with the License.
##
## You may obtain a copy of the License at
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software 
## distributed under the License is distributed on an "AS IS" BASIS, WITHOUT 
## WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## 
## See the License for the specific language governing permissions and 
## limitations under the License.
##
################################################################################

# Default Build Options unless overriden on command line: make PLAT=x86 BUILD=Debug OS=Chrome
PLAT  := x64
BUILD := Release
OS    := Linux

ifneq ($(filter default undefined,$(origin CC)),)
CC := gcc
endif

SOURCES := ../Sources
ESIF_SDK_SOURCES := ../../../../Common
ESIF_LIB_SOURCES := ../../ESIF_LIB/Sources

LDLIBS := -ldl -lpthread

CPPFLAGS += -I$(SOURCES) -I$(ESIF_SDK_SOURCES) -I$(ESIF_LIB_SOURCES)
CPPFLAGS += -DESIF_ATTR_USER

CFLAGS += -Werror

# Compile Flags added for security purposes
CFLAGS  += -fPIE
CFLAGS  += -fPIC
CFLAGS  += -O2 -D_FORTIFY_SOURCE=2
CFLAGS  += -Wformat -Wformat-security

# Some flags are not directly compatible with Clang, but work out okay if the following flags are added
ifeq ($(CC), clang)
	CFLAGS += -Wno-ignored-optimization-argument -Wno-unused-command-line-argument
endif
CFLAGS  += -fno-strict-overflow
CFLAGS  += -fno-delete-null-pointer-checks
CFLAGS  += -fwrapv
#Compile with -g by default and strip it at the end
CFLAGS   += -g

# Linker Flags added for security purposes
LDFLAGS += -z noexecstack 
LDFLAGS += -z relro -z now 
#LDFLAGS += -pie 

# Secure Compile options if GCC version >= 4.9
GCC_STRONG := $(shell expr `($(CC) -dumpfullversion 2>/dev/null || $(CC) -dumpversion) | cut -d'.' -f1,2,3 | sed -e 's/\.\([0-9]\)$$/0\1/' | sed -e 's/\.//g' ` \>= 409)
ifeq ($(GCC_STRONG), 1)
       CFLAGS += -fstack-protector-strong
else
       CFLAGS += -fstack-protector
endif

# Hide Non-Exported Symbols
CFLAGS += -fvisibility=hidden

# make OS=<Android|Chrome|Linux>
ifeq ($(OS), Android)
	CPPFLAGS += -DESIF_ATTR_OS_ANDROID
else ifeq ($(OS), Chrome)
	CPPFLAGS += -DESIF_ATTR_OS_CHROME
	CPPFLAGS += -std=gnu99
else
	CPPFLAGS += -DESIF_ATTR_OS_LINUX
	CPPFLAGS += -std=gnu99

	# make PLAT=<x64|x86>
	ifeq ($(PLAT), x64)
		CFLAGS += -m64
	endif
	ifeq ($(PLAT), x86)
		CFLAGS += -m32
	endif
endif

# make BUILD=Debug
ifeq ($(BUILD), Debug)
	CPPFLAGS += -DESIF_ATTR_DEBUG
endif

$(info ++++ )
$(info ++++ IPF_WS $(PLAT) $(BUILD) $(OS))
$(info ++++ )

###############################################################################
# IPF_WS 
###############################################################################

OBJ := $(SOURCES)/esif_ws_http.o
OBJ += $(SOURCES)/esif_ws_server.o
OBJ += $(SOURCES)/esif_ws_socket.o
OBJ += $(SOURCES)/esif_ws.o
OBJ += $(ESIF_SDK_SOURCES)/esif_sdk_base64_enc.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_istring.o

###############################################################################
# BUILD 
###############################################################################

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.cpp $(DEPS)
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $(CPPFLAGS) -o $@ $<

all: ipf_ws
ifeq ($(OS), Linux)
    ifeq ($(BUILD), Release)
		cp ipf_ws.so ipf_ws.so.debug
		strip --strip-unneeded ipf_ws.so
		objcopy --add-gnu-debuglink=ipf_ws.so.debug ipf_ws.so
    endif
endif
ipf_ws: $(OBJ)
	$(CC) $(CFLAGS) -shared $(EXTRA_CFLAGS) $(LDFLAGS) -o $@.so $^ $(LDLIBS)

clean:
	rm -f $(OBJ) *.so *.debug
