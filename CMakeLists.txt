cmake_minimum_required(VERSION 3.16)`

# Include the ESP-IDF project build logic
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Espressif_ESP32-P4-DevKit-Wifi-Ethernet)idf_component_register(
    SRCS 
        "JC-ESP32P4-M3-WiFi-Ethernet.cpp"
        "wifi_fast_scan.c"
        "lcd_init.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES arduino-esp32 esp_wifi esp_eth
)