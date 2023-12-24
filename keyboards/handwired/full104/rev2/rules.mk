# This file intentionally left blank
VIA_ENABLE = yes
VIAL_ENABLE = yes
RGBLIGHT_ENABLE = yes
RGBLIGHT_DRIVER = ws2812
WS2812_DRIVER = vendor
OLED_ENABLE = yes
OLED_DRIVER = ssd1306
OPT_DEFS += -DHAL_USE_I2C=TRUE
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = pimoroni_trackball
SRC += pimoroni_trackball.c