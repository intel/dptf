Intel (R) Dynamic Tuning
for Chromium OS - 9.x Release

README

-------------------------------------------------------------------------------

This document describes how to build and integrate DPTF to Chromimum OS. Please
note that DPTF is already fully integrated to most Intel based board overlays
by default if you build from the upstream code from chromium.org.
On overlays where DPTF is not enabled by default, you can use this guide and
follow the necessary steps to build and enable it.

This document assumes that the user has already set up the Chromium OS SDK on
their development machine. Otherwise, please refer to Google's "Chromium OS
Developer Guide" available at:

	http://www.chromium.org/chromium-os/developer-guide

-------------------------------------------------------------------------------
MANUALLY ADD DPTF TO OVERLAYS IN CHROMIUM OS SDK
-------------------------------------------------------------------------------

WARNING: if the underlying CoreBoot BIOS does not support DPTF ACPI objects,
then these instructions will not help, because DPTF user space daemon relies
on INT340X thermal drivers to perform its functions, and these drivers will
not load if the underlying DPTF ACPI objects are missing. Chances are that
if CoreBoot in your system supports DPTF, then DPTF should have already been
enabled by default in Chromium OS SDK tree.

Step 1 - Locate the board overlay make.conf file in Chromium OS SDK. You will
need to know the board project name of your system. For example, for the "Cyan"
board, this file is at src/overlays/overlay-cyan/make.conf.

Step 2 - Add the following line to make.conf:

USE="${USE} dptf"

If "dptf" is already part of the USE flag, then it means that DPTF has
already been enabled in your system, and you can skip this step and build the
OS image directly.

Step 3 - Follow the official Chromium OS Developer Guide to build and install
the Chromium OS image for your board.

https://www.chromium.org/chromium-os/developer-guide

-------------------------------------------------------------------------------
VERIFY INSTALL/CONFIGURATION
-------------------------------------------------------------------------------
Upon system boot, you should be able to observe the ipf_ufd process that has
been launched by upstart.

-------------------------------------------------------------------------------
SELECTIVELY LOAD DPTF CONFIGURATION FILES FROM FILE SYSTEM
-------------------------------------------------------------------------------
DPTF relies on a predefined set of thermal tables and parameters to operate
properly. Normally this set of data is derived from the thermal tuning process
for each specific platform, and is stored in the platform BIOS (CoreBoot in the
case of Chrome/Chromium OS). Starting with the 8.4.10100 build, system
integrators now also have the option to point to the data file that is stored
in the file system instead. To use this feature, modify the DPTF service init
file /etc/init/ipf.conf, at the very last line, instead of just plainly
invoking ipf_ufd, add the following command line argument:

exec ipf_ufd -a <full path to the DPTF config data vault file>

The tools to generate DPTF config data vault files are supplied to Chromebook
OEMs separately and are not available to end users.

-------------------------------------------------------------------------------
MANUALLY INSTALL DPTF ON UBUNTU LINUX
-------------------------------------------------------------------------------
Although DPTF is not officially supported on Linux, users who wish to
experiment with it can follow the steps below. These steps are tailored for
Ubuntu Linux and may require adjustments for other distributions.

Requirements:
1. DPTF requires corresponding ACPI support in the BIOS. Not all Intel-based
   platforms support DPTF in BIOS. Contact your BIOS vendor to confirm if
   DPTF is enabled on your system.
2. Install pm-utils for certain DPTF functionalities.
3. For DPTF version 9.x, a kernel version 4.0-rc7 or later is required.
4. GCC version 4.8 or later is needed to compile the source code.

Dependencies:
Install the following packages to set up DPTF on Ubuntu:

- cmake
- libedit-dev
- pm-utils

Recommended Utilities:
Install lm-sensors to monitor thermal and power-related data.

 Step-by-Step Installation

Step 1 - Install CMake
Ensure CMake is installed on your system:

sudo apt install cmake libedit-dev pm-utils (lm-sensors) - Note: apt-get for versions prior to Ubuntu 16.04 LTS


Step 2 - Configure the Build System
Set up an environment variable for the DPTF root directory:

export DPTF=/path/to/dptf

Navigate to the Linux build directory and generate makefiles:

cd $DPTF/DPTF/Linux/build
cmake ..

By default, this generates makefiles for a 64-bit release build. To build other variants, refer to $DPTF/DPTF/Linux/CMakeLists.txt to determine the appropriate environment variables for cmake.

Step 3 - Build DPTF Shared Libraries
Compile the DPTF shared libraries:

make -j$(nproc)

The shared libraries will be created in $DPTF/DPTF/Linux/build/x64/release.

Generated shared libraries:
- Dptf.so
- DptfPolicyActive.so
- DptfPolicyCritical.so
- DptfPolicyPassive.so

Step 4 - Deploy Shared Libraries and Configuration Files
Copy the shared libraries to the appropriate system directory:

sudo mkdir -p /usr/share/dptf/ufx64
sudo cp $DPTF/DPTF/Linux/build/x64/release/Dptf*.so /usr/share/dptf/ufx64

Copy configuration files:

sudo mkdir -p /etc/dptf
sudo cp $DPTF/ESIF/Packages/DSP/dsp.dv /etc/dptf


Step 5 - Build DPTF Executables
Build the main service executables:

make -C $DPTF/ESIF/Products/ESIF_UF/Linux
make -C $DPTF/IPF/Linux

Copy the executables to a system directory (e.g., /usr/bin):

sudo cp $DPTF/ESIF/Products/ESIF_UF/Linux/ipf_ufd /usr/bin
sudo cp $DPTF/IPF/Linux/ipfhostd /usr/bin


Step 6 - Build and Deploy Additional Libraries
Compile the necessary ESIF libraries:

make -C $DPTF/ESIF/Products/ESIF_CMP/Linux
make -C $DPTF/ESIF/Products/ESIF_WS/Linux

Copy the generated libraries:

sudo cp $DPTF/ESIF/Products/ESIF_CMP/Linux/ipf_cmp.so /usr/share/dptf/ufx64
sudo cp $DPTF/ESIF/Products/ESIF_WS/Linux/ipf_ws.so /usr/share/dptf/ufx64
sudo cp $DPTF/IPF/Linux/ipfsrv.so /usr/share/dptf/ufx64
sudo cp $DPTF/IPF/Linux/ipfipc.so /usr/share/dptf/ufx64


Step 7 - Start DPTF
Start the DPTF service manually:

sudo /usr/bin/ipf_ufd

Verify the service status:

pgrep -l ipf_ufd


-------------------------------------------------------------------------------
CONFIGURE DPTF SERVICE
-------------------------------------------------------------------------------

Enable Auto-Start with Systemd
For Ubuntu 15.04 and later (using systemd):
1. Copy the ipf.service file:
   
   sudo cp $DPTF/ESIF/Packages/Installers/linux/ipf.service /lib/systemd/system
   
2. Enable the service:
   
   sudo systemctl enable ipf.service
   
3. Optionally, manage the service:
   
   sudo systemctl start ipf.service   Start the service
   sudo systemctl stop ipf.service    Stop the service
   sudo systemctl restart ipf.service Restart the service


Enable DTT Service
Similarly, copy and enable the dtt.service file:

sudo cp $DPTF/ESIF/Packages/Installers/linux/dtt.service /lib/systemd/system
sudo systemctl enable dtt.service

You can manage the DTT service as needed:

sudo systemctl start dtt.service
sudo systemctl stop dtt.service
sudo systemctl restart dtt.service

-------------------------------------------------------------------------------
KNOWN ISSUES / LIMITATIONS
-------------------------------------------------------------------------------
