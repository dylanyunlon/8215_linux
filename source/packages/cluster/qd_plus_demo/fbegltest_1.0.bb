# Copyright (C) 2015 Autochips.
# Released under the LGPL license (see COPYING.LGPL for the terms)
DESCRIPTION = "ATC fbegltest"
LICENSE = "ATC-1"
LIC_FILES_CHKSUM = "file://${COREBASE}/../meta-atc/files/custom-licenses/ATC-1;md5=b36538de914c1d599e1cf946e39e05f0"

COMPATIBLE_MACHINE_ac8317 = "ac8317"
PACKAGES = "${PN} ${PN}-dbg"

inherit pkgconfig
require  conf/env.conf

PROVIDES = "libgl libgles1 libgles2 egl"

PN = "fbegltest"
PV = "1.0"


ATC_SRC = "${TOPDIR}/../src/graphics/egl_test"

inherit externalsrc

EXTERNALSRC = "${ATC_SRC}"

EXTERNALSRC_BUILD = "${EXTERNALSRC}"

do_compile() {
    make TARGET_TOP="${EXTERNALSRC_BUILD}"
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/QD_demo ${D}${bindir}/QD_demo
	install -d ${D}/usr/share/QD_HMI
	cp -r ${S}/QD_HMI/* ${D}/usr/share/QD_HMI/
}

FILES_${PN} += "${libdir}/*.so ${includedir}/*.h /usr/share/QD_HMI"

