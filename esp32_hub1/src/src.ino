#define MQTT_MAX_PACKET_SIZE 1024

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>

// ===== Globals =====
WiFiClient espClient;
PubSubClient client(espClient);
Preferences prefs;

String wifi_ssid;
String wifi_password;
String mqtt_server;
uint16_t mqtt_port;
String firmware_url;

long counter1 = 0, counter2 = 0, counter3 = 0, counter4 = 0, counter5 = 0;

// ===== MQTT Topics & Discovery =====
const char* TOPIC_DISCOVERY_NUMBER1 = "homeassistant/number/mx_hub1_number1/config";
const char* TOPIC_DISCOVERY_NUMBER2 = "homeassistant/number/mx_hub1_number2/config";
const char* TOPIC_DISCOVERY_NUMBER3 = "homeassistant/number/mx_hub1_number3/config";
const char* TOPIC_DISCOVERY_NUMBER4 = "homeassistant/number/mx_hub1_number4/config";
const char* TOPIC_DISCOVERY_NUMBER5 = "homeassistant/number/mx_hub1_number5/config";
const char* TOPIC_DISCOVERY_BUTTON1 = "homeassistant/button/mx_hub1_update/config";

const char* TOPIC_STATE1 = "mx_hub1/state1";
const char* TOPIC_STATE2 = "mx_hub1/state2";
const char* TOPIC_STATE3 = "mx_hub1/state3";
const char* TOPIC_STATE4 = "mx_hub1/state4";
const char* TOPIC_STATE5 = "mx_hub1/state5";

const char* TOPIC_SET1 = "mx_hub1/set1";
const char* TOPIC_SET2 = "mx_hub1/set2";
const char* TOPIC_SET3 = "mx_hub1/set3";
const char* TOPIC_SET4 = "mx_hub1/set4";
const char* TOPIC_SET5 = "mx_hub1/set5";

const char* TOPIC_UPDATE1    = "mx_hub1/update";
const char* TOPIC_STATUS    = "mx_hub1/status";
const char* TOPIC_VERSION   = "mx_hub1/version";

// ===== Discovery Payloads =====
const char* PAYLOAD_DISCOVERY1 = "{\"name\":\"MX Hub1 Counter 1\",\"state_topic\":\"mx_hub1/state1\",\"command_topic\":\"mx_hub1/set1\",\"min\":0,\"max\":1000000,\"step\":1,\"unique_id\":\"mx_hub1_number1\",\"device\":{\"identifiers\":[\"mx_hub1\"],\"name\":\"MX Hub1\"}}";
const char* PAYLOAD_DISCOVERY2 = "{\"name\":\"MX Hub1 Counter 2\",\"state_topic\":\"mx_hub1/state2\",\"command_topic\":\"mx_hub1/set2\",\"min\":0,\"max\":1000000,\"step\":1,\"unique_id\":\"mx_hub1_number2\",\"device\":{\"identifiers\":[\"mx_hub1\"],\"name\":\"MX Hub1\"}}";
const char* PAYLOAD_DISCOVERY3 = "{\"name\":\"MX Hub1 Counter 3\",\"state_topic\":\"mx_hub1/state3\",\"command_topic\":\"mx_hub1/set3\",\"min\":0,\"max\":1000000,\"step\":1,\"unique_id\":\"mx_hub1_number3\",\"device\":{\"identifiers\":[\"mx_hub1\"],\"name\":\"MX Hub1\"}}";
const char* PAYLOAD_DISCOVERY4 = "{\"name\":\"MX Hub1 Counter 4\",\"state_topic\":\"mx_hub1/state4\",\"command_topic\":\"mx_hub1/set4\",\"min\":0,\"max\":1000000,\"step\":1,\"unique_id\":\"mx_hub1_number4\",\"device\":{\"identifiers\":[\"mx_hub1\"],\"name\":\"MX Hub1\"}}";
const char* PAYLOAD_DISCOVERY5 = "{\"name\":\"MX Hub1 Counter 5\",\"state_topic\":\"mx_hub1/state5\",\"command_topic\":\"mx_hub1/set5\",\"min\":0,\"max\":1000000,\"step\":1,\"unique_id\":\"mx_hub1_number5\",\"device\":{\"identifiers\":[\"mx_hub1\"],\"name\":\"MX Hub1\"}}";
const char* PAYLOAD_BUTTON1 = "{\"name\":\"MX Hub1 Update\",\"unique_id\":\"mx_hub1_update_btn\",\"command_topic\":\"mx_hub1/update\",\"payload_press\":\"update\",\"device\":{\"identifiers\":[\"mx_hub1\"],\"name\":\"MX Hub1\"}}";

void publishState1() { client.publish(TOPIC_STATE1, String(counter1).c_str(), true); }
void publishState2() { client.publish(TOPIC_STATE2, String(counter2).c_str(), true); }
void publishState3() { client.publish(TOPIC_STATE3, String(counter3).c_str(), true); }
void publishState4() { client.publish(TOPIC_STATE4, String(counter4).c_str(), true); }
void publishState5() { client.publish(TOPIC_STATE5, String(counter5).c_str(), true); }
void publishVersion() { client.publish(TOPIC_VERSION, "V3", true); Serial.println("V3 (Counter 1)"); }
void publishStatus(const char* status) {
  client.publish(TOPIC_STATUS, status, true);
  Serial.print("OTA Status (Counter 1): ");
  Serial.println(status);
}

bool doOTA() {
  WiFiClientSecure clientHTTP;
  clientHTTP.setInsecure();
  HTTPClient http;

  Serial.println("Requesting OTA file from GitHub RAW...");
  http.begin(clientHTTP, firmware_url.c_str());
  int httpCode = http.GET();
  Serial.print("OTA HTTP GET response: ");
  Serial.println(httpCode);
  if (httpCode != 200) {
    http.end();
    return false;
  }
  int contentLength = http.getSize();
  Serial.print("Content Length: ");
  Serial.println(contentLength);
  if (!Update.begin(contentLength)) {
    Serial.println("Update.begin() failed!");
    http.end();
    return false;
  }
  WiFiClient* stream = http.getStreamPtr();
  uint8_t buff[1024];
  int written = 0;
  while (http.connected() && written < contentLength) {
    size_t avail = stream->available();
    if (avail) {
      size_t len = stream->readBytes(buff, min((int)sizeof(buff), (int)avail));
      size_t w = Update.write(buff, len);
      written += w;
      Serial.printf("Wrote %d bytes, total %d", (int)w, written);
    }
    delay(1);
  }
  bool ok = Update.end();
  Serial.print("Update.end() returned: ");
  Serial.println(ok ? "true" : "false");
  http.end();
  return ok && Update.isFinished();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String t = topic;
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (t == TOPIC_SET1) { counter1 = msg.toInt(); publishState1(); }
  else if (t == TOPIC_SET2) { counter2 = msg.toInt(); publishState2(); }
  else if (t == TOPIC_SET3) { counter3 = msg.toInt(); publishState3(); }
  else if (t == TOPIC_SET4) { counter4 = msg.toInt(); publishState4(); }
  else if (t == TOPIC_SET5) { counter5 = msg.toInt(); publishState5(); }
  else if (t == TOPIC_UPDATE1) {
    msg.trim();
    if (msg == "update") {
      publishStatus("Starting OTA update...");
      bool ok = doOTA();
      if (ok) {
        publishStatus("OTA update successful. Rebooting...");
        delay(2000);
        ESP.restart();
      } else {
        publishStatus("OTA update failed.");
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("MXHub1Client")) {
      Serial.println(" Connected!");
      client.subscribe(TOPIC_SET1);
      client.subscribe(TOPIC_SET2);
      client.subscribe(TOPIC_SET3);
      client.subscribe(TOPIC_SET4);
      client.subscribe(TOPIC_SET5);
      client.subscribe(TOPIC_UPDATE1);

      client.publish(TOPIC_DISCOVERY_NUMBER1, PAYLOAD_DISCOVERY1, true);
      client.publish(TOPIC_DISCOVERY_NUMBER2, PAYLOAD_DISCOVERY2, true);
      client.publish(TOPIC_DISCOVERY_NUMBER3, PAYLOAD_DISCOVERY3, true);
      client.publish(TOPIC_DISCOVERY_NUMBER4, PAYLOAD_DISCOVERY4, true);
      client.publish(TOPIC_DISCOVERY_NUMBER5, PAYLOAD_DISCOVERY5, true);
      client.publish(TOPIC_DISCOVERY_BUTTON1, PAYLOAD_BUTTON1, true);

      publishState1(); publishState2(); publishState3(); publishState4(); publishState5();
      publishVersion();
      publishStatus("MQTT connected.");
    } else {
      Serial.printf(" Failed (%d). Retrying...", client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("--- ESP32 MX Hub1 BOOT ---");

  prefs.begin("mxhub1", false);
  bool initialized = prefs.getBool("initialized", false);

  if (!initialized) {
    prefs.putString("wifi_ssid", "LMT_0A29");
    prefs.putString("wifi_password", "nm3MHB9YbT2");
    prefs.putString("mqtt_server", "65.109.173.41");
    prefs.putUShort("mqtt_port", 1883);
    prefs.putString("firmware_url", "https://raw.githubusercontent.com/edzenis/maintainx/main/esp32_hub1/firmware.bin");
    prefs.putBool("initialized", true);
    Serial.println("NVS initialized with default settings.");
  } else {
    Serial.println("NVS already initialized. Loading values...");
    wifi_ssid     = prefs.getString("wifi_ssid", "LMT_0A29");
    wifi_password = prefs.getString("wifi_password", "nm3MHB9YbT2");
    mqtt_server   = prefs.getString("mqtt_server", "65.109.173.41");
    mqtt_port     = prefs.getUShort("mqtt_port", 1883);
    firmware_url  = prefs.getString("firmware_url", "https://raw.githubusercontent.com/edzenis/maintainx/main/esp32_hub1/firmware.bin");
    Serial.println("Loaded from NVS:");
    Serial.print("  SSID: "); Serial.println(wifi_ssid);
    Serial.print("  MQTT: "); Serial.println(mqtt_server);
    Serial.print("  Port: "); Serial.println(mqtt_port);
    Serial.print("  OTA : "); Serial.println(firmware_url);
  }

  const char* ssid = wifi_ssid.c_str();
  const char* password = wifi_password.c_str();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" WiFi connected!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  client.setServer(mqtt_server.c_str(), mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last > 5000) {
    last = now;
    counter1++; counter2++; counter3++; counter4++; counter5++;
    Serial.printf("Counters: %ld %ld %ld %ld %ld", counter1, counter2, counter3, counter4, counter5);
    publishState1(); publishState2(); publishState3(); publishState4(); publishState5();
  }
}
