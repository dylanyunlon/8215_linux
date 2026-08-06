
-include $(BR2_EXTERNAL_AC8X_PATH)/board/autochips/$(PLATFORM)/ProjectConfig.mk

include $(sort $(wildcard $(BR2_EXTERNAL_AC8X_PATH)/package/*/*.mk))

