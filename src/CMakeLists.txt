idf_component_register(
    SRCS 
        "JC-ESP32P4-M3-WiFi-Ethernet.cpp"
        "wifi_fast_scan.c"
        "lcd_init.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES arduino-esp32 esp_wifi esp_eth
)