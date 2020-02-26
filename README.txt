Intel (R) Dynamic Tuning
for Chromium OS - 8.x Release

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
Upon system boot, you should be able to observe the esif_ufd process that has
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
file /etc/init/dptf.conf, at the very last line, instead of just plainly
invoking esif_ufd, add the following command line argument:

exec esif_ufd -a <full path to the DPTF config data vault file>

The tools to generate DPTF config data vault files are supplied to Chromebook
OEMs separately and are not available to end users.

-------------------------------------------------------------------------------
MANUALLY INSTALL DPTF ON UBUNTU LINUX
-------------------------------------------------------------------------------
Even though DPTF is not yet officially supported on Linux, users who wish to
try it on Linux can follow steps below. Please note that these steps apply to
Ubuntu Linux only, and may vary if your Linux distro is different.

Requirement: DPTF requires corresponding ACPI support in BIOS. Not all
Intel based platforms support DPTF in BIOS. Please contact your BIOS vendor
to see if DPTF is enabled in your system.

The 8.x version of DPTF also requires 4.0-rc7 kernel or later in order to
run properly. To compile the DPTF source code, you need GCC version 4.8 or
later.

Step 1 - Install CMake tool if you have not installed it on your Linux system:

	sudo apt-get install cmake

Step 2 - Install libreadline if you have not done so already on your Linux
system:

	sudo apt-get install libreadline6 libreadline6-dev

Step 3 - Go to the Linux subdirectory of DPTF (<DPTF root>/DPTF/Linux/build)
and run the command:

	cmake ..

This command will invoke cmake and generate all the GNU make files for each
sub-modules of DPTF user space libraries. By default this command will
generate the make files for 64-bit release version. If you want to build a
different flavor, please examine the <DPTF root>/DPTF/Linux/CMakeLists.txt
to find out what environment variables that you may need to pass to cmake.

Step 4 - Run make to build all DPTF shared libraries.

	make -j`nproc`

The generated shared libraries will be located under
<DPTF root>/DPTF/Linux/build/x64/release directory. Users can disregard
the static .a libraries as these static libraries are only used to build the
shared library. Here is the break down of the generated shared libraries that
are needed to run DPTF on Linux:

	* Dptf.so
	* DptfPolicyActive.so
	* DptfPolicyCritical.so
	* DptfPolicyPassive.so

Step 5 - Copy the above shared libraries and other DPTF configuration files
to proper locations on your system (assuming that you are running on a 64-bit
system, otherwise replace "64" below with "32" if you are building and
running on 32-bit Linux).

	sudo mkdir -p /usr/share/dptf/ufx64
	sudo cp Dptf*.so /usr/share/dptf/ufx64
	sudo mkdir -p /etc/dptf
	sudo cp <DPTF root>/ESIF/Packages/DSP/dsp.dv /etc/dptf

Step 6 - Run make under <DPTF root>/ESIF/Products/ESIF_UF/Linux to build
the esif_ufd executable. This is the main DPTF service executable that loads
the DPTF policies that you have built in Step 3. After the build is complete,
you will find the esif_ufd executable generated under the same directory.

Please note that the default make target is a 64-bit release version. If you
want to build a different flavor, please examine the Makefile under this
directory to find out what environment variable you want to pass to Gnu make.
Please do not alter the default settings for OS, OPT_GMIN and OPT_DBUS
environment variables - they are for Chromium OS builds only, and for Linux
builds, please use the default values.

After the esif_ufd build is done, copy the executable to the proper location
on your system (using /usr/bin as an example, but any system path should work):
	sudo cp esif_ufd /usr/bin

Step 7 - Install other ESIF shared libraries
Additional ESIF libraries will be required to work with the newer format of
DPTF data vault files. Run make under the following directories:
    <DPTF root>/ESIF/Products/ESIF_CMP/Linux
    <DPTF root>/ESIF/Products/ESIF_WS/Linux

Copy the generated library files to /usr/share/dptf/ufx64

    cp <DPTF root>/ESIF/Products/ESIF_CMP/Linux/esif_cmp.so \
        /usr/share/dptf/ufx64

    cp <DPTF root>/ESIF/Products/ESIF_WS/Linux/esif_ws.so \
        /usr/share/dptf/ufx64

Step 8 - Start DPTF. Simply run:
	sudo /usr/bin/esif_ufd

This executable will run in daemon mode, and DPTF policies will automatically
be loaded by this executable. You can check the status of the DPTF service
by running this command:

	pgrep -l esif_ufd

This command will show the active DPTF process ID.

-------------------------------------------------------------------------------
INSTALL DPTF SERVICE INIT SCRIPT
-------------------------------------------------------------------------------
For Ubuntu 14.10 and earlier:
If you want the DPTF service to start automatically upon system boot, assuming
that you are using upstart init system (which is the default init system on
Ubuntu 14.10 and earlier, and on Chrome OS), you can simply copy the dptf.conf
script to /etc/init:

	sudo cp <DPTF root>/ESIF/Packages/Installers/chrome/dptf.conf /etc/init/

On next system reboot, DPTF service (esif_ufd) will execute automatically. To
manually stop the running DPTF service, type:

	sudo initctl stop dptf

To start a stopped service, type:

	sudo initctl start dptf

To restart the DPTF service, type:

	sudo initctl restart dptf

For Ubuntu 15.04 and later:
Starting with Ubuntu 15.04 the default init system has switched to systemd.
If this is the system that you use, then to auto start DPTF service, copy
the dptf.service script to /lib/systemd/system:

	sudo cp <DPTF root>/ESIF/Packages/Installers/linux/dptf.service \
	/lib/systemd/system

You will then need to enable the DPTF service to auto load upon startup:

	sudo systemctl enable dptf.service

DPTF(esif_ufd) service will automatically start the next time the system
boots. You can also manually start and stop DPTF service anytime by doing:

	systemctl start dptf.service   # To start the service
	systemctl stop dptf.service    # To stop the service
	systemctl restart dptf.service # To restart the service

-------------------------------------------------------------------------------
KNOWN ISSUES / LIMITATIONS
-------------------------------------------------------------------------------
* For board overlays that still deploy 3.10 kernel, compilation warnings will
be noticed during the build. No warnings should be observed for overlays that
are based on 3.18 kernel or above, or for the Ubuntu OS build for that matter.
