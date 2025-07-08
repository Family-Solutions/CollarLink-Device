#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Pines para Serial2 (GPS)
#define GPS_BAUDRATE 9600
#define RXD2_PIN     17  // GPS TX → RX2
#define TXD2_PIN     16  // GPS RX ← TX2

TinyGPSPlus gps;

const char* WIFI_SSID     = "CARBAJAL";
const char* WIFI_PASSWORD = "08808204";

const char* serverUrl = "http://192.168.68.124:5000/api/v1/location";
const char* apiKey    = "test-api-key-123";
const char* deviceId  = "gps-collar-001";

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, RXD2_PIN, TXD2_PIN);
  Serial.println("🚀 Iniciando GPS + POST");

  // Conexión WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("🔌 Conectando a %s", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado");
}

void loop() {
  // Leer datos del GPS
  while (Serial2.available() > 0) {
    //char c = Serial2.read();
    //Serial.write(c);
    if (gps.encode(Serial2.read())) {
      // Si hay posición y hora válidas
      if (gps.location.isValid()) {
        // Formato ISO8601
        char timestamp[25];
        snprintf(timestamp, sizeof(timestamp),
                 "%04d-%02d-%02dT%02d:%02d:%02dZ",
                 gps.date.year(), gps.date.month(), gps.date.day(),
                 gps.time.hour(), gps.time.minute(), gps.time.second());

        // Construir JSON
        StaticJsonDocument<256> payload;
        payload["device_id"]  = deviceId;
        payload["latitude"]   = gps.location.lat();
        payload["longitude"]  = gps.location.lng();
        payload["created_at"] = timestamp;

        String body;
        serializeJson(payload, body);

        // Enviar HTTP POST
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-API-Key", apiKey);

        Serial.println("[POST] " + body);
        int code = http.POST(body);

        if (code > 0) {
          Serial.printf("✅ HTTP %d — %s\n", code, http.getString().c_str());
        } else {
          Serial.printf("❌ Error POST: %s\n",
                        http.errorToString(code).c_str());
        }
        http.end();

        // Esperar antes del siguiente envío
        delay(10000);
      } else {
        Serial.printf("❌ Invalid GPS — sats: %d, HDOP: %.1f\n",
                  gps.satellites.value(),
                  gps.hdop.hdop());
      }
    }
  }

  // Detección de falta de datos
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("⚠️ No llegan datos GPS");
    delay(2000);
  }
}
