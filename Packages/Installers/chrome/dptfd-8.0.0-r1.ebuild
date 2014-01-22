# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="4"
EGIT_REPO_URI="git://github.com/01org/dptf.git"
EGIT_COMMIT="3d767c13b9567a2106691f86519c205874422fcd"

inherit git-2 toolchain-funcs

DESCRIPTION="User space daemon for Dynamic Platform & Thermal Framework"
HOMEPAGE=""

LICENSE="Apache 2.0, GPLv2, BSD"
SLOT="0"
KEYWORDS="amd64 ~x86"
RDEPEND="sys-power/dptf-kmod"

ESIF_UF_DST_DIR=usr/bin
DSP_SRC_DIR=Packages/DSP
DSP_DST_DIR=usr/share/dptf/dsp

get_builddir() {
	builddir="Products/ESIF_UF"
	use x86 && builddir="${builddir}/Chrome86"
	use amd64 && builddir="${builddir}/Chrome64"
	if use debug ; then
		builddir="${builddir}/Debug"
	else
		builddir="${builddir}/Release"
	fi
}

src_compile() {
	get_builddir
	cd ${builddir}
	emake || die "Make failed!"
}

src_install() {
	get_builddir
	insinto ${ESIF_UF_DST_DIR}
	doins ${builddir}/esif_uf
	insinto ${DSP_DST_DIR}
	doins ${DSP_SRC_DIR}/*.edp
}

