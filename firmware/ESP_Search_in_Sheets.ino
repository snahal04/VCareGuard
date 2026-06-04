//----------------------------------------Include the NodeMCU ESP8266 Library
//----------------------------------------see here: https://www.youtube.com/watch?v=8jMr94B8iN0 to add NodeMCU ESP12E ESP8266 library and board (ESP8266 Core SDK)
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
//----------------------------------------

//----------------------------------------SSID and Password of your WiFi router.
const char* ssid = "REPLACE_WITH_YOUR_WIFI_SSID";         //--> Your wifi name or SSID.
const char* password = "REPLACE_WITH_YOUR_WIFI_PASSWORD";  //--> Your wifi password.
//----------------------------------------

String GOOGLE_SCRIPT_ID = "REPLACE_WITH_YOUR_GAS_SCRIPT_ID";  //--> spreadsheet script ID
String hospitalNames[3] = { "Shri%20Mata%20Vaishno%20Devi%20Narayana%20Superspeciality%20Hospital" };
const char* delimiter = ",";  // Choose a suitable delimiter
String combinedHospitalNames = hospitalNames[0] + delimiter + hospitalNames[1] + delimiter + hospitalNames[2];
//============================================================================== void setup
void setup() {

  Serial.begin(115200);
  delay(500);
  connect();
  sendData();
}
void connect() {
  WiFi.begin(ssid, password);  //--> Connect to your WiFi router
  Serial.println("");
  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  //----------------------------------------
  Serial.println("");
  Serial.print("Successfully connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------
}
void loop() {
}

void sendData() {
  Serial.println(combinedHospitalNames);
  WiFiClientSecure client;
  HTTPClient https;
  client.setInsecure();
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?values=" + combinedHospitalNames;
  Serial.println(url);
  https.begin(client, url);
  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  https.GET();
  String payload = https.getString();
  Serial.println(payload);
  Serial.println("######______#######");
  // Release resources for HTTPClient
  https.end();
  // Release resources for WiFiClientSecure
  client.stop();
  // https.begin(client, "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?value=" + map);
  // //-----------------------------------------------------------------------------------
  // //Removes the error "302 Moved Temporarily Error"
  // https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  // https.GET();
  // // Serial.print("HTTP Status Code: ");
  // // Serial.println(httpCode);
  // // //-----------------------------------------------------------------------------------
  // // if (httpCode <= 0) {
  // //   Serial.println("Error on HTTP request");
  // //   https.end();
  // //   return;
  // // }
  // //-----------------------------------------------------------------------------------
  // //reading data comming from Google Sheet
  // String payload = https.getString();
  // Serial.println("Payload: " + payload);
  parsePayload(payload);
  // https.end();
}


String urlEncode(String str) {
  String encodedString = "";
  char c;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isAlphaNumeric(c) || c == '-' || c == '.' || c == '_' || c == '~') {
      encodedString += c;
    } else {
      // Encode special characters as %XX where XX is the hexadecimal representation of the character
      char hex[3];
      sprintf(hex, "%02X", c);
      encodedString += "%" + String(hex);
    }
  }
  return encodedString;
}
//==============================================================================

String email1;
String email2;
String email3;

void parsePayload(String payload) {
  // Split the payload by line breaks
  String lines[12];
  for (int i = 0; i < 12; i++) {
    int newlinePos = payload.indexOf('\n');
    if (newlinePos != -1) {
      lines[i] = payload.substring(0, newlinePos);
      payload = payload.substring(newlinePos + 1);
    } else {
      lines[i] = payload; // Last line
    }
  }

  // Extract email values from the lines
  email1 = lines[1].substring(7); // Remove "Email: "
  email2 = lines[6].substring(7); // Remove "Email: "
  email3 = lines[11].substring(7); // Remove "Email: "

  // Print the email values
  Serial.println("Email 1: " + email1);
  Serial.println("Email 2: " + email2);
  Serial.println("Email 3: " + email3);
}
