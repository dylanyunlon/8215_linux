ifneq ($(ATC_WIFI_CHIP),)
include $(wildcard $(pkgdir)/*/*.mk)
endif
