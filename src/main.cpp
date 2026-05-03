#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>

// Pin 26 is the C6 Enable/Reset pin for synchronization
#define C6_EN 26 

// Static IP Configuration - Using .200 as requested
IPAddress local_IP(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

static bool eth_connected = false;

void NetworkEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.printf("ETH Connected! IP: %s\n", ETH.localIP().toString().c_str());
            eth_connected = true;
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("ETH Disconnected");
            eth_connected = false;
            break;
        default: break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(3000); 
    
    Serial.println("\n--- KENNEDY LAB: P4 RECOVERY (WIFI + ETH) ---");

    // Reset C6 for SDIO synchronization
    pinMode(C6_EN, OUTPUT);
    digitalWrite(C6_EN, LOW);
    delay(1000); 
    digitalWrite(C6_EN, HIGH);
    
    Serial.println("Waiting 4s for C6 bridge...");
    delay(4000); 

    WiFi.onEvent(NetworkEvent);
    
    // Initialize Ethernet with Static IP
    ETH.begin(); 
    ETH.config(local_IP, gateway, subnet, dns);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("System Ready.");
}

void loop() {
    Serial.print(eth_connected ? "[ETH: ONLINE] " : "[ETH: OFFLINE] ");
    
    int n = WiFi.scanNetworks();
    if (n > 0) {
        Serial.printf("WiFi Success! Found %d networks.\n", n);
    } else {
        Serial.println("Scanning WiFi...");
    }
    
    WiFi.scanDelete();
    delay(10000); 
}