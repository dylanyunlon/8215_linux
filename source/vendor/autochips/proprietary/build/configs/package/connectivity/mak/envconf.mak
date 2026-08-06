#   dir var
# Path prefixes
base_prefix ?= 
prefix ?= /usr
exec_prefix ?= /usr

# Base paths
base_bindir ?= $(base_prefix)/bin
base_sbindir ?= $(base_prefix)/sbin
base_libdir ?= $(base_prefix)/$(baselib)
nonarch_base_libdir ?= $(base_prefix)/lib

# Architecture independent paths
sysconfdir ?= $(base_prefix)/etc
servicedir ?= $(base_prefix)/srv
sharedstatedir ?= $(base_prefix)/com
localstatedir ?= $(base_prefix)/var
datadir ?= $(prefix)/share
infodir ?= $(datadir)/info
mandir ?= $(datadir)/man
docdir ?= $(datadir)/doc
systemd_unitdir ?= /lib/systemd

# Architecture dependent paths
bindir ?= $(exec_prefix)/bin
sbindir ?= $(exec_prefix)/sbin
libdir ?= $(exec_prefix)/lib
libexecdir ?= $(libdir)/$(BPN)
includedir ?= $(exec_prefix)/include
oldincludedir ?= $(exec_prefix)/include
localedir ?= $(libdir)/locale
appdir ?= $(exec_prefix)/app

#localpreifx := $(DA_TOP)/lib
localprefix := $(TARGET_TOP)
locallibdir := $(localprefix)/lib
localbindir := $(localprefix)/bin
localincludedir := $(localprefix)/include
localappdir := $(localprefix)/app



