#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>


//----------------------------------------SSID and Password of your WiFi router.
const char* ssid = "SMVDU.";         //--> Your wifi name or SSID.
const char* password = "123456789";  //--> Your wifi password.
//----------------------------------------

String GOOGLE_SCRIPT_ID = "AKfycbxXXz0YmHEd6W_7TpkaFpgdPyq5vLOp2DPDKEa0aYMDB6OFNTedqzFL-Cp2WU1ckGxS";  //--> spreadsheet script ID

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


void sendData() {
  String latitude = "32.9444427";
  String longitude = "74.9522781";
  String map = latitude + "," + longitude;
  WiFiClientSecure client;
  HTTPClient https;
  client.setInsecure();

  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?value=" + map;
  Serial.println(url);

  https.setTimeout(10000); // Adjust the timeout value as needed
  https.begin(client, url);
  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  // https.begin(client, "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?value=" + map);
  // //-----------------------------------------------------------------------------------
  // //Removes the error "302 Moved Temporarily Error"
  // https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = https.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //-----------------------------------------------------------------------------------
  if (httpCode <= 0) {
    Serial.println("Error on HTTP request");
    https.end();
    return;
  }

  String payload = https.getString();
  Serial.println(payload);
  Serial.println("######______#######");
  // Release resources for HTTPClient
  https.end();
  // Release resources for WiFiClientSecure
  client.stop();
  // //-----------------------------------------------------------------------------------
  // //reading data comming from Google Sheet
  // String payload = https.getString();
  // Serial.println("Payload: " + payload);
  // parsePayload(payload);
  // https.end();
}

void loop(){}