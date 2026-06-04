#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>
#include <WifiLocation.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587
#define AUTHOR_EMAIL "REPLACE_WITH_YOUR_GMAIL_ADDRESS"
#define AUTHOR_PASSWORD "REPLACE_WITH_YOUR_GMAIL_APP_PASSWORD"

SMTPSession smtp;
WiFiClientSecure client;
HTTPClient https;

String Hospital_EMAILS[3] = { "null", "null", "null" };

bool isPermanentConnection = 0;
String textMsg = "<p>An Accident Needs Your Attention They Are Injured</p><ul>";

String mapCoordinates = "";

int family[3] = { 0, 0, 0 };


// ############################# EDIT HERE FOR PASSWORD AND SSID OF FAMILY MEMBER ##########################
void member1() {
  if (connectWiFi("REPLACE_WITH_FAMILY_WIFI_SSID_1", "REPLACE_WITH_YOUR_WIFI_PASSWORD")) family[0] = 1;
}

void member2() {
  if (connectWiFi("sagar", "REPLACE_WITH_YOUR_WIFI_PASSWORD")) family[1] = 1;
}

void member3() {
  if (connectWiFi("redmi", "REPLACE_WITH_YOUR_WIFI_PASSWORD")) family[2] = 1;
}

bool connectWiFi(String ssid, String password) {
  WiFi.begin(ssid, password);
  uint16_t count = 10;
  while (WiFi.status() != WL_CONNECTED && count--) {
    delay(1000);
  }
  bool isConnected = WiFi.status() == WL_CONNECTED;
  if (isConnected) {
    delay(1000);
    if (isPermanentConnection == 0) WiFi.disconnect();
    return 1;
  } 
  return 0;
}

void networkReferesh() {
  WiFi.disconnect();
  delay(2000);
  isPermanentConnection = 1;
  for (uint8_t i = 0; i < 3; i++) {
    if (family[i] == 1) {
      if (i == 0) member1();
      else if (i == 1) member2();
      else member3();
      break;
    }
  }
  // delay(1000);
}

void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  for (int i = 0; i < 10; i++) {
    time_t now = time(nullptr);
    if (now > 8 * 3600 * 2) {
      struct tm timeinfo;
      gmtime_r(&now, &timeinfo);
      return;
    }
    delay(500);
  }
  networkReferesh();
  setClock();
}

void setup() {
  Serial.flush();
  Serial.flush();
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(D3, OUTPUT);
  delay(1000);
}


void loop() {
  digitalWrite(BUILTIN_LED, LOW);
  digitalWrite(D3, LOW);
  processFamilyMembers();
  if (family[0] == 0 && family[1] == 0 && family[2] == 0) {
    delay(2000);
    processFamilyMembers();
  }

  delay(5000);
  digitalWrite(BUILTIN_LED, HIGH);
  digitalWrite(D3, HIGH);

  getCurrentLocation();

  sendtoSpreadSheet();

  digitalWrite(D3, LOW);
  delay(1000);
  digitalWrite(D3, HIGH);

  sendMailtoMember();
  digitalWrite(BUILTIN_LED, LOW);
  digitalWrite(D3, LOW);
}




void processFamilyMembers() {
  member1(), member2(), member3();
  if (family[0]) {
    textMsg += "<li><b>Name:</b>SNAHAL KUMAR</li>";
    textMsg += "<li><b>Age:</b>21</li>";
    textMsg += "<li><b>Gender:</b>Male</li>";
    textMsg += "<li><b>Contact Number:</b>7984202797</li>";
    textMsg += "<li><b>Medical History ID: </b> <a href=http://13.233.246.42/sk798/>Check Medical History</a></li>";
    textMsg += "</ul>";
  }
  if (family[1]) {
    textMsg += "<ul>";
    textMsg += "<li><b>Name: </b>REPLACE_WITH_FAMILY_WIFI_SSID_2 Jha</li>";
    textMsg += "<li><b>Age: </b>21</li>";
    textMsg += "<li><b>Gender: </b>Male</li>";
    textMsg += "<li><b>Contact Number: </b>9871984322</li>";
    textMsg += "<li><b>Medical History ID: </b> <a href=http://13.233.246.42/scj798/>Check Medical History</a></li>";
    textMsg += "</ul>";
  }
  if (family[2]) {
    textMsg += "<ul>";
    textMsg += "<li><b>Name:</b>Somya Sinha</li>";
    textMsg += "<li><b>Age: </b>21</li>";
    textMsg += "<li><b>Gender: </b>Female</li>";
    textMsg += "<li><b>Contact Number: </b>9173932572</li>";
    textMsg += "<li><b>Medical History ID: </b> <a href=http://13.233.246.42/ss798/>Check Medical History</a></li>";
    textMsg += "</ul>";
  }
}

void getCurrentLocation() {
  const char* googleApiKey = "REPLACE_WITH_YOUR_GOOGLE_MAPS_API_KEY";  // Google API key
  WifiLocation location(googleApiKey);

  for (uint8_t stopCheckCount = 3; stopCheckCount > 0; stopCheckCount--) {
    networkReferesh();
    setClock();
    location_t loc = location.getGeoFromWiFi();
    mapCoordinates = String(loc.lat, 7) + "," + String(loc.lon, 7);
    if (mapCoordinates != "0.0000000,0.0000000") {
      String directions = "https://docs.google.com/spreadsheets/d/1Ha6ap3USqMDXS67iOrA7RBHCeQkxMkzW3ywBVGEC0F0/edit?usp=sharing";
      textMsg += "<p>View Accident Location & Directions to Nearby Hospitals (Hold on name to see note with contact details): <a href=\"" + directions + "\">Google Sheets</a></p><br>";
      textMsg += "<p> <span style=\"font-size: 8px; color: #888888;\"> MADE WITH LOVE BY SNAHAL KUMAR </span> </p><br>";
      return;
    }
  }
}


void sendtoSpreadSheet() {
  String GOOGLE_SCRIPT_ID = "REPLACE_WITH_YOUR_GAS_SCRIPT_ID";  // to get Contact
  networkReferesh();
  client.setInsecure();
  https.setTimeout(30000);
  uint8_t maxRetryAttempts = 3;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?value=" + mapCoordinates;
  for (uint8_t attempt = 0; attempt < maxRetryAttempts; attempt++) {
    https.begin(client, url);
    https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = https.getString();
      parsePayload(payload);
      client.stop();
      https.end();
      break; 
    } else {
      String errorPayload = https.getString();
      if (attempt < maxRetryAttempts - 1) {
        delay(1000);
      } else {
      }
    }
    https.end();
    networkReferesh();
    client.setInsecure();
    https.setTimeout(30000);
  }
}


void parsePayload(String payload) {
  uint8_t startIndex = 0;

  for (uint8_t i = 0; i < 3; i++) {
    uint16_t commaPos = payload.indexOf(',', startIndex);
    if (commaPos != -1) {
      Hospital_EMAILS[i] = payload.substring(startIndex, commaPos);
      startIndex = commaPos + 1;
    } else {
      Hospital_EMAILS[i] = payload.substring(startIndex);  // Last email
    }
  }
}



void sendMailtoMember() {
  bool flagError = 0;
  uint8_t stopCheckCount = 3;
  do {
    if (stopCheckCount == 0) {
      return;
    }
    networkReferesh();
    Serial.println();
    MailClient.networkReconnect(true);

    smtp.debug(0);
    Session_Config config;

    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;
    config.login.user_domain = F("127.0.0.1");

    config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    config.time.gmt_offset = 3;
    config.time.day_light_offset = 0;

    SMTP_Message message;

    message.sender.name = F("NEED ATTENTION");
    message.sender.email = AUTHOR_EMAIL;

    message.subject = F("YOUR FAMILY NEEDS URGENT AMBULANCE");

    message.addRecipient(F("Snahal Kumar"), "snahal135@gmail.com");
    message.addRecipient(F("REPLACE_WITH_FAMILY_WIFI_SSID_2 Chandra Jha"), "pk529852@gmail.com");
    message.addRecipient(F("Somya Sinha"), "20bec082@smvdu.ac.in");

    for (uint8_t i = 0; i < 3; i++) {
      if (Hospital_EMAILS[i] == "null") break;
      message.addRecipient("Hospital", Hospital_EMAILS[i]);
    }

    message.html.content = textMsg;
    message.html.charSet = F("us-ascii");

    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

    message.addHeader(F("Message-ID: snahal135@gmail.com"));

    if (!smtp.connect(&config)) {
      MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      flagError = 1;
      stopCheckCount--;
      continue;
    }

    if (!MailClient.sendMail(&smtp, &message)) {
      MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      flagError = 1;
    }

    smtp.sendingResult.clear();
    WiFi.disconnect();
    // delay(1000);
    stopCheckCount--;
  } while (flagError);
}