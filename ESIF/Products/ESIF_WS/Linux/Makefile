################################################################################
## Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

LDLIBS := -ldl

CPPFLAGS += -I$(SOURCES) -I$(ESIF_SDK_SOURCES)
CPPFLAGS += -DESIF_ATTR_USER

CFLAGS += -Werror

# Compile Flags added for security purposes
CFLAGS  += -fPIE
CFLAGS  += -fPIC
CFLAGS  += -O2 -D_FORTIFY_SOURCE=2
CFLAGS  += -Wformat -Wformat-security

# Linker Flags added for security purposes
LDFLAGS += -z noexecstack 
LDFLAGS += -z relro -z now 
#LDFLAGS += -pie 

# Secure Compile options if GCC version >= 4.9
GCC_STRONG := $(shell expr `$(CC) -dumpversion | cut -d'.' -f1,2 | sed -e 's/\.\([0-9]\)$$/0\1/' | sed -e 's/\.//' ` \>= 409)
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
	CPPFLAGS += -std=gnu89
else
	CPPFLAGS += -DESIF_ATTR_OS_LINUX
	CPPFLAGS += -std=gnu89

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
	CFLAGS   += -g
endif

$(info ++++ )
$(info ++++ ESIF_WS $(PLAT) $(BUILD) $(OS))
$(info ++++ )

###############################################################################
# ESIF_WS 
###############################################################################

OBJ := $(SOURCES)/esif_ws_http.o
OBJ += $(SOURCES)/esif_ws_server.o
OBJ += $(SOURCES)/esif_ws_socket.o
OBJ += $(SOURCES)/esif_ws.o

###############################################################################
# BUILD 
###############################################################################

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.cpp $(DEPS)
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $(CPPFLAGS) -o $@ $<

all: esif_ws

esif_ws: $(OBJ)
	$(CC) $(CFLAGS) -shared $(EXTRA_CFLAGS) $(LDFLAGS) -o $@.so $^ $(LDLIBS)

clean:
	rm -f $(OBJ) *.so
