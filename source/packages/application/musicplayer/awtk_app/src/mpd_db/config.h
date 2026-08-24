/* config.h — generated for embedded musicplayer port */
#ifndef MPD_CONFIG_H
#define MPD_CONFIG_H

#define ENABLE_DATABASE 1

/* Disable heavy optional features */
/* #undef ENABLE_ZLIB */
/* #undef ENABLE_ARCHIVE */
/* #undef ENABLE_UPNP */
/* #undef ENABLE_ICU */
/* #undef ENABLE_INOTIFY */
/* #undef ENABLE_NEIGHBOR_PLUGINS */
/* #undef ENABLE_SYSTEMD_DAEMON */
/* #undef HAVE_GETPWNAM_R */
/* #undef HAVE_INITGROUPS */
/* #undef ENABLE_DBUS */
/* #undef HAVE_ICU */
/* #undef HAVE_ICU_CONVERTER */

/* We're on Linux */
#define HAVE_UN 1

/* Version */
#define VERSION "0.23-embedded"
#define PROTOCOL_VERSION "0.23.0"

#endif
#define PACKAGE_NAME "musicplayer"
