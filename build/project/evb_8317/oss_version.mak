export KMOD_VERSION       ?= 8
export LIBPNG_VERSION     ?= 1.6.13
export NCURSES_VERSION    ?= 5.9
export QT_VERSION         = 5.5.0
export TSLIB_VERSION      ?= 1.1
export UDEV_VERSION       ?= 182
export UTILLINUX_VERSION  ?= 2.24.2
export ZLIB_VERSION       ?= 1.2.8
export E2FSPROGS_VERSION  ?= 1.42.9
export NTFS3G_VERSION	  ?= 2015.3.14
export SQLITE_VERSION	  ?= 3.8.6

export DBUS_LIB_VERSION   ?=
export ALSA_VERSION       ?= 
export ALSALIB_VERSION    ?= 1.0.28
export ALSAUTILS_VERSION  ?= 1.0.29
export BUSYBOX_VERSION    ?= 1.22.1
export FOTOWALL_VERSION   ?= 0.9
export GLIB_VERSION       ?= 2.40.0
export LIBFFI_VERSION     ?= 3.1
export EXPAT_VERSION      ?= 2.0.1
export LIBXML2_VERSION    ?= 2.9.1
ifeq ($(PKG_CONFIG_GST), 0.1)
  export GSTREAMER_VERSION  ?= 0.10.36
  export GST_PLUGINS_BASE_VERSION  ?= 0.10.36
  export GST_PLUGINS_GOOD_VERSION  ?= 0.10.31
  export GST_OPENMAX_VERSION  ?= 0.10.1
  export GST_FFMPEG_VERSION   ?= 0.10.13
else
  export GSTREAMER_VERSION  ?= 1.4.1
  export GST_PLUGINS_BASE_VERSION  ?= 1.4.1
  export GST_PLUGINS_GOOD_VERSION  ?= 1.4.1
  export GST_OMX_VERSION    ?= 1.2.0
  export GST_LIBAV_VERSION  ?= 1.4.1
endif
export JSON-C_VERSION       ?= 0.11
export LIBTOOL_VERSION      ?= 2.4.2
export LIBSNDFILE_VERSION   ?= 1.0.25
export DBUS_VERSION         = 1.8.2
export AVAHI_VERSION        ?= 0.6.31
export PULSEADUIO_VERSION   ?= 5.0
export VALGRIND_VERSION     ?= 3.10.0
export LIBICONV_VERSION     ?= 1.14
export BZIP2_VERSION        ?= 1.0.6
export YASM_VERSION         ?= 1.2.0
export ORC_VERSION          ?= 0.4.18
export LIBNL_VERSION        ?= 3.2.25
export GNUTLS_VERSION       ?= 3.3.5
export NETTLE_VERSION       ?= 2.7.1
export GMP_VERSION          ?= 6.0.0
export LIBGCRYPT_VERSION    ?= 1.6.1
export LIBGPG_ERROR_VERSION ?= 1.12
export READLINE_VERSION		?= 6.3
export IPTABLES_VERSION		?= 1.4.21
export CONNMAN_VERSION      ?= 1.25
export OFONO_VERSION        ?= 1.15
export LIBUSB_VERSION		?= 1.0.20
export USB_MODESWITCH_VERSION	?= 2.2.5
export OPENSSL_VERSION		?= 1.0.1j
export HOSTAPD_VERSION		?= 2.2
export WPA_SUPPLICANT_VERSION	?= 2.2
export QCONNMANUI_VERSION          ?=1.0
export QCONNMAN_VERSION          ?=1.24
export WAYLAND_VERSION          ?= 1.8.0
export CAIRO_VERSION		?= 1.12.16
export FC_VERSION		    ?= 2.11.1
export FT_VERSION		    ?= 2.5.3
export HB_VERSION		    ?= 0.9.35
export JPEG_VERSION		    ?= 8d
export LIBDRM_VERSION		?= 2.4.54
export LIBEVDEV_VERSION		?= 1.2.2
export LIBINPUT_VERSION		?= 0.10.0
export XKBCOMMON_VERSION	?= 0.5.0
export MTDEV_VERSION		?= 1.1.5
export PANGO_VERSION		?= 1.36.6
export PIXMAN_VERSION		?= 0.32.6
export WESTON_VERSION		?= 1.8.0
export DNSMASQ_VERSION		?= 2.75
export STRACE_VERSION		?=4.10

export OSS_PACKAGES :=  \
    kmod-$(KMOD_VERSION)           \
    libpng-$(LIBPNG_VERSION)       \
    ncurses-$(NCURSES_VERSION)     \
    qt-$(QT_VERSION)               \
    tslib-$(TSLIB_VERSION)         \
    udev-$(UDEV_VERSION)           \
    util-linux-$(UTILLINUX_VERSION) \
    zlib-$(ZLIB_VERSION)           \
    e2fsprogs-$(E2FSPROGS_VERSION) \
    ntfs-3g_ntfsprogs-$(NTFS3G_VERSION) \
    sqlite-$(SQLITE_VERSION)	\
    busybox-$(BUSYBOX_VERSION)     \
    glib-$(GLIB_VERSION)           \
    libffi-$(LIBFFI_VERSION)       \
    expat-$(EXPAT_VERSION)         \
    libxml2-$(LIBXML2_VERSION)     \
    gstreamer-$(GSTREAMER_VERSION) \
    gst-plugins-base-$(GST_PLUGINS_BASE_VERSION) \
    gst-plugins-good-$(GST_PLUGINS_GOOD_VERSION) \
    alsa-lib-$(ALSALIB_VERSION) \
    alsa-utils-$(ALSAUTILS_VERSION) \
    pulseaudio-$(PULSEADUIO_VERSION) \
    json-c-$(JSON-C_VERSION)         \
    libtool-$(LIBTOOL_VERSION)         \
    libsndfile-$(LIBSNDFILE_VERSION) \
    dbus-$(DBUS_VERSION)               \
    avahi-$(AVAHI_VERSION)            \
    libcap-$(LIBCAP_VERSION)  \
    valgrind-$(VALGRIND_VERSION)  \
    libiconv-$(LIBICONV_VERSION)  \
    bzip2-$(BZIP2_VERSION)  \
    yasm-$(YASM_VERSION) \
    orc-$(ORC_VERSION) \
    libnl-$(LIBNL_VERSION) \
    gnutls-$(GNUTLS_VERSION) \
    nettle-$(NETTLE_VERSION) \
    gmp-$(GMP_VERSION) \
    libgcrypt-$(LIBGCRYPT_VERSION) \
    libgpg-error-$(LIBGPG_ERROR_VERSION) \
    readline-$(READLINE_VERSION) \
    iptables-$(IPTABLES_VERSION) \
    connman-$(CONNMAN_VERSION) \
    ofono-$(OFONO_VERSION) \
    libusb-$(LIBUSB_VERSION) \
    usb-modeswitch-$(USB_MODESWITCH_VERSION) \
    openssl-$(OPENSSL_VERSION) \
    hostapd-$(HOSTAPD_VERSION) \
    wpa_supplicant-$(WPA_SUPPLICANT_VERSION) \
    wayland-$(WAYLAND_VERSION) \
    cairo-$(CAIRO_VERSION) \
    fontconfig-$(FC_VERSION) \
    freetype-$(FT_VERSION) \
    harfbuzz-$(HB_VERSION) \
    jpeg-$(JPEG_VERSION) \
    libdrm-$(LIBDRM_VERSION) \
    libevdev-$(LIBEVDEV_VERSION) \
    libinput-$(LIBINPUT_VERSION) \
    libxkbcommon-$(XKBCOMMON_VERSION) \
    mtdev-$(MTDEV_VERSION) \
    pango-$(PANGO_VERSION) \
    pixman-$(PIXMAN_VERSION) \
    weston-$(WESTON_VERSION) \
    dnsmasq-$(DNSMASQ_VERSION) \
    strace-$(STRACE_VERSION) \
    qconnmanui-$(QCONNMANUI_VERSION) \
    qconnman-$(QCONNMAN_VERSION)

ifeq ($(PKG_CONFIG_GST), 0.1)
  export OSS_PACKAGES +=  \
    gst-openmax-$(GST_OPENMAX_VERSION) \
    gst-ffmpeg-$(GST_FFMPEG_VERSION)
else
  export OSS_PACKAGES +=  \
    gst-omx-$(GST_OMX_VERSION) \
    gst-libav-$(GST_LIBAV_VERSION)
endif
