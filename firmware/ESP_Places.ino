#include <ArduinoJson.h>  // Include the ArduinoJSON library
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

double currentLat = 32.9409866;  // Your current latitude
double currentLng = 74.9542694;  // Your current longitude

// Root certificate for valid till 2028S
const char IRG_Root_X1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG
A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv
b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw
MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i
YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT
aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ
jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp
xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp
1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG
snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ
U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8
9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E
BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B
AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz
yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE
38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP
AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad
DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME
HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==
-----END CERTIFICATE-----
)EOF";

// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_WIFI_SSID";
const char* password = "REPLACE_WITH_YOUR_WIFI_PASSWORD";

// Create a list of certificates with the server certificate
X509List cert(IRG_Root_X1);

void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  //Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));


  WiFiClientSecure client;

  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    client.setTrustAnchors(&cert);

    HTTPClient https;
    https.useHTTP10(true);
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(client, "https://maps.googleapis.com/maps/api/place/nearbysearch/json?location=32.9409866,74.9542694&radius=1000&type=hospital&key=REPLACE_WITH_YOUR_GOOGLE_MAPS_API_KEY")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // Serial.println(httpCode);
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          // String payload = https.getString();
          // Serial.println(payload);
          DynamicJsonDocument doc(2048);
          deserializeJson(doc, https.getStream());
          // Serial.println("JSON Document:");
          // serializeJsonPretty(doc, Serial);
          // Serial.println();
          // Serial.println("Doc end :");

          if (doc.containsKey("results")) {
            const JsonArray results = doc["results"];
            for (const JsonObject result : results) {
              const char* name = result["name"];
              // int user_rating = result["user_ratings_total"]; // Not needed
              // Serial.print("Place Name: ");
              // Serial.println(name);
              const double placeLat = result["geometry"]["location"]["lat"];
              const double placeLng = result["geometry"]["location"]["lng"];
              double distance = calculateDistance(currentLat, currentLng, placeLat, placeLng);

              Serial.print("Place Name: ");
              Serial.println(name);
              Serial.print("Distance: ");
              Serial.print(distance, 6);  // Print distance with 6 decimal places
              Serial.println(" km");
            }
          }
          client.stop();
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}

void loop() {
}
// Haversine formula
double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
  double R = 6371.0; // Radius of the Earth in kilometers
  double dLat = radians(lat2 - lat1);
  double dLon = radians(lon2 - lon1);
  double a = sin(dLat / 2) * sin(dLat / 2) + cos(radians(lat1)) * cos(radians(lat2)) * sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double distance = R * c;
  return distance;
}