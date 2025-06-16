#include <TinyGPS++.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

const char* serverUrl = "https://5e2b-179-6-2-239.ngrok-free.app/api/v1/location";
//const char* serverUrl = "http://127.0.0.1:5000/api/v1/location";
//const char* serverUrl = "http://192.168.1.105/api/v1/location";
const char* apiKey = "test-api-key-123";
const char* deviceId = "gps-collar-001";

TinyGPSPlus gps;
HTTPClient http;
WiFiClientSecure client;

// ✅ Certificado raíz en formato PEM (ISRG Root X1)
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

const char gps_data[][80] = {
  "$GPGGA,172914.000,1206.240,S,07657.720,W,1,08,1.0,0.0,M,0.0,M,,*67\r\n",
  "$GPRMC,172914.000,A,1206.240,S,07657.720,W,05.5,54.7,130625,000.0,W*7D\r\n",
  "$GPGGA,172915.000,1206.244,S,07657.732,W,1,08,1.0,0.0,M,0.0,M,,*61\r\n",
  "$GPRMC,172915.000,A,1206.244,S,07657.732,W,05.5,54.7,130625,000.0,W*7B\r\n",
  "$GPGGA,172916.000,1206.249,S,07657.744,W,1,08,1.0,0.0,M,0.0,M,,*6E\r\n",
  "$GPRMC,172916.000,A,1206.249,S,07657.744,W,05.5,54.7,130625,000.0,W*74\r\n",
  "$GPGGA,172917.000,1206.253,S,07657.756,W,1,08,1.0,0.0,M,0.0,M,,*67\r\n",
  "$GPRMC,172917.000,A,1206.253,S,07657.756,W,05.5,54.7,130625,000.0,W*007D\r\n",
  "$GPGGA,172918.000,1206.258,S,07657.768,W,1,08,1.0,0.0,M,0.0,M,,*6E\r\n",
  "$GPRMC,172918.000,A,1206.258,S,07657.768,W,05.5,54.7,130625,000.0,W*74\r\n",
  "$GPGGA,172919.000,1206.267,S,07657.782,W,1,08,1.0,0.0,M,0.0,M,,*67\r\n",
  "$GPRMC,172919.000,A,1206.267,S,07657.782,W,05.5,54.7,130625,000.0,W*7D\r\n",
  "$GPGGA,172920.000,1206.277,S,07657.795,W,1,08,1.0,0.0,M,0.0,M,,*06Ar\n",
  "$GPRMC,172920.000,A,1206.277,S,07657.795,W,05.5,54.7,130625,000.0,W*70\r\n",
  "$GPGGA,172921.000,1206.286,S,07657.809,W,1,08,1.0,0.0,M,0.0,M,,*6F\r\n",
  "$GPRMC,172921.000,A,1206.286,S,07657.809,W,05.5,54.7,130625,000.0,W*75\r\n",
  "$GPGGA,172922.000,1206.294,S,07657.810,W,1,08,1.0,0.0,M,0.0,M,,*67\r\n",
  "$GPRMC,172922.000,A,1206.294,S,07657.810,W,05.5,54.7,130625,000.0,W*7D\r\n",
  "$GPGGA,172923.000,1206.312,S,07657.826,W,1,08,1.0,0.0,M,0.0,M,,*6C\r\n",
  "$GPRMC,172923.000,A,1206.312,S,07657.826,W,05.5,54.7,130625,000.0,W*76\r\n",
  "$GPGGA,172924.000,1206.330,S,07657.842,W,1,08,1.0,0.0,M,0.0,M,,*69\r\n",
  "$GPRMC,172924.000,A,1206.330,S,07657.842,W,05.5,54.7,130625,000.0,W*73\r\n",
  "$GPGGA,172925.000,1206.339,S,07657.855,W,1,08,1.0,0.0,M,0.0,M,,*67\r\n",
  "$GPRMC,172925.000,A,1206.339,S,07657.855,W,05.5,54.7,130625,000.0,W*7D\r\n",
  "$GPGGA,172926.000,1206.348,S,07657.868,W,1,08,1.0,0.0,M,0.0,M,,*6C\r\n",
  "$GPRMC,172926.000,A,1206.348,S,07657.868,W,05.5,54.7,130625,000.0,W*0076\r\n",
  "$GPGGA,172927.000,1206.357,S,07657.881,W,1,08,1.0,0.0,M,0.0,M,,*64\r\n",
  "$GPRMC,172927.000,A,1206.357,S,07657.881,W,05.5,54.7,130625,000.0,W*07E\r\n",
  "$GPGGA,172928.000,1206.366,S,07657.894,W,1,08,1.0,0.0,M,0.0,M,,*6D\r\n",
  "$GPRMC,172928.000,A,1206.366,S,07657.894,W,05.5,54.7,130625,000.0,W*77\r\n",
  "$GPGGA,172929.000,1206.384,S,07657.910,W,1,08,1.0,0.0,M,0.0,M,,*69\r\n",
  "$GPRMC,172929.000,A,1206.384,S,07657.910,W,05.5,54.7,130625,000.0,W*73\r\n",
  "$GPGGA,172930.000,1206.402,S,07657.926,W,1,08,1.0,0.0,M,0.0,M,,*6A\r\n",
  "$GPRMC,172930.000,A,1206.402,S,07657.926,W,05.5,54.7,130625,000.0,W*70\r\n",
  "$GPGGA,172931.000,1206.420,S,07657.942,W,1,08,1.0,0.0,M,0.0,M,,*6B\r\n",
  "$GPRMC,172931.000,A,1206.420,S,07657.942,W,05.5,54.7,130625,000.0,W*71\r\n",
  "$GPGGA,172932.000,1206.438,S,07657.958,W,1,08,1.0,0.0,M,0.0,M,,*6C\r\n",
  "$GPRMC,172932.000,A,1206.438,S,07657.958,W,05.5,54.7,130625,000.0,W*76\r\n",
  "$GPGGA,172933.000,1206.456,S,07657.974,W,1,08,1.0,0.0,M,0.0,M,,*61\r\n",
  "$GPRMC,172933.000,A,1206.456,S,07657.974,W,05.5,54.7,130625,000.0,W*7B\r\n",
  "$GPGGA,172934.000,1206.474,S,07657.990,W,1,08,1.0,0.0,M,0.0,M,,*61\r\n",
  "$GPRMC,172934.000,A,1206.474,S,07657.990,W,05.5,54.7,130625,000.0,W*7B\r\n",
  "$GPGGA,172935.000,1206.492,S,07658.006,W,1,08,1.0,0.0,M,0.0,M,,*6F\r\n",
  "$GPRMC,172935.000,A,1206.492,S,07658.006,W,05.5,54.7,130625,000.0,W*75\r\n",
  "$GPGGA,172936.000,1206.510,S,07658.022,W,1,08,1.0,0.0,M,0.0,M,,*6E\r\n",
  "$GPRMC,172936.000,A,1206.510,S,07658.022,W,05.5,54.7,130625,000.0,W*74\r\n",
  "$GPGGA,172937.000,1206.528,S,07658.038,W,1,08,1.0,0.0,M,0.0,M,,*62\r\n",
  "$GPRMC,172937.000,A,1206.528,S,07658.038,W,05.5,54.7,130625,000.0,W*78\r\n",
  "$GPGGA,172938.000,1206.517,S,07658.026,W,1,08,1.0,0.0,M,0.0,M,,*62\r\n",
  "$GPRMC,172938.000,A,1206.517,S,07658.026,W,05.5,54.7,130625,000.0,W*78\r\n",
  "$GPGGA,172939.000,1206.506,S,07658.014,W,1,08,1.0,0.0,M,0.0,M,,*63\r\n",
  "$GPRMC,172939.000,A,1206.506,S,07658.014,W,05.5,54.7,130625,000.0,W*79\r\n",
  "$GPGGA,172940.000,1206.496,S,07658.002,W,1,08,1.0,0.0,M,0.0,M,,*6D\r\n",
  "$GPRMC,172940.000,A,1206.496,S,07658.002,W,05.5,54.7,130625,000.0,W*77\r\n",
  "$GPGGA,172941.000,1206.485,S,07657.990,W,1,08,1.0,0.0,M,0.0,M,,*66\r\n",
  "$GPRMC,172941.000,A,1206.485,S,07657.990,W,05.5,54.7,130625,000.0,W*7C\r\n",
  "$GPGGA,172942.000,1206.474,S,07657.978,W,1,08,1.0,0.0,M,0.0,M,,*6E\r\n",
  "$GPRMC,172942.000,A,1206.474,S,07657.978,W,05.5,54.7,130625,000.0,W*74\r\n",
  "$GPGGA,172943.000,1206.463,S,07657.966,W,1,08,1.0,0.0,M,0.0,M,,*6A\r\n",
  "$GPRMC,172943.000,A,1206.463,S,07657.966,W,05.5,54.7,130625,000.0,W*70\r\n",
  "$GPGGA,172944.000,1206.452,S,07657.954,W,1,08,1.0,0.0,M,0.0,M,,*6D\r\n",
  "$GPRMC,172944.000,A,1206.452,S,07657.954,W,05.5,54.7,130625,000.0,W*77\r\n",
  "$GPGGA,172945.000,1206.442,S,07657.942,W,1,08,1.0,0.0,M,0.0,M,,*6F\r\n",
  "$GPRMC,172945.000,A,1206.442,S,07657.942,W,05.5,54.7,130625,000.0,W*75\r\n",
  "$GPGGA,172946.000,1206.431,S,07657.930,W,1,08,1.0,0.0,M,0.0,M,,*64\r\n",
  "$GPRMC,172946.000,A,1206.431,S,07657.930,W,05.5,54.7,130625,000.0,W*7E\r\n",
  "$GPGGA,172947.000,1206.420,S,07657.918,W,1,08,1.0,0.0,M,0.0,M,,*6F\r\n",
  "$GPRMC,172947.000,A,1206.420,S,07657.918,W,05.5,54.7,130625,000.0,W*75\r\n",
  "$GPGGA,172948.000,1206.409,S,07657.906,W,1,08,1.0,0.0,M,0.0,M,,*6B\r\n",
  "$GPRMC,172948.000,A,1206.409,S,07657.906,W,05.5,54.7,130625,000.0,W*71\r\n",
  "$GPGGA,172949.000,1206.398,S,07657.894,W,1,08,1.0,0.0,M,0.0,M,,*65\r\n",
  "$GPRMC,172949.000,A,1206.398,S,07657.894,W,05.5,54.7,130625,000.0,W*7F\r\n",
  "$GPGGA,172950.000,1206.388,S,07657.882,W,1,08,1.0,0.0,M,0.0,M,,*69\r\n",
  "$GPRMC,172950.000,A,1206.388,S,07657.882,W,05.5,54.7,130625,000.0,W*73\r\n",
  "$GPGGA,172951.000,1206.377,S,07657.870,W,1,08,1.0,0.0,M,0.0,M,,*60\r\n",
  "$GPRMC,172951.000,A,1206.377,S,07657.870,W,05.5,54.7,130625,000.0,W*7A\r\n",
  "$GPGGA,172952.000,1206.366,S,07657.858,W,1,08,1.0,0.0,M,0.0,M,,*60\r\n",
  "$GPRMC,172952.000,A,1206.366,S,07657.858,W,05.5,54.7,130625,000.0,W*7A\r\n"
};
int gps_index = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("🚀 Simulador de GPS con envío HTTP");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("🔌 Conectando al WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado");
  client.setCACert(root_ca);
}

void loop() {
  const char* line = gps_data[gps_index++];
  Serial.print("📡 Simulando: ");
  Serial.println(line);

  for (size_t i = 0; i < strlen(line); i++) {
    gps.encode(line[i]);
  }

  if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) {
    char timestamp[25];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02d",
             gps.date.year(), gps.date.month(), gps.date.day(),
             gps.time.hour(), gps.time.minute(), gps.time.second());

    DynamicJsonDocument payload(512);
    payload["device_id"] = deviceId;
    payload["latitude"] = gps.location.lat();
    payload["longitude"] = gps.location.lng();
    payload["created_at"] = String(timestamp);

    String requestBody;
    serializeJson(payload, requestBody);

    http.begin(serverUrl);
    //http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", apiKey);
    http.addHeader("ngrok-skip-browser-warning", "69420");

    Serial.println("[POST] Payload: " + requestBody);
    int responseCode = http.POST(requestBody);

    if (responseCode > 0) {
      Serial.println("✅ HTTP " + String(responseCode));
      Serial.println(http.getString());
    } else {
      Serial.print("❌ POST failed: ");
      Serial.println(http.errorToString(responseCode));
    }

    http.end();
  } else {
    Serial.println("⚠️ Datos GPS inválidos");
  }

  if (gps_index >= sizeof(gps_data) / sizeof(gps_data[0])) {
    gps_index = 0;
  }

  delay(5000);  // Cada 5 segundos simula un nuevo punto
}