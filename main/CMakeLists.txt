idf_component_register(SRCS "ui_view_handler.c" "ui_control.c" "ui_screen.c" "power_control.c" "timerwire.c" "blink.c" "u8g2_esp32_hal.c" "ui_screen2.c" "temp_sensors.c" "ui_control.c" "rotary.c"
                    INCLUDE_DIRS "."
                    REQUIRES onewire u8g2 tarablessd1306 ds18x20 esp32-rotary-encoder)


