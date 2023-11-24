#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>
#include <WifiLocation.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587
#define AUTHOR_EMAIL "esp.snahal@gmail.com"
#define AUTHOR_PASSWORD "vtfuukhtfbjsykxx"

SMTPSession smtp;
WiFiClientSecure client;
HTTPClient https;
// void smtpCallback(SMTP_Status status);
String Hospital_EMAILS[3] = { "null", "null", "null" };

bool isPermanentConnection = 0;
String textMsg = "<p>An Accident Needs Your Attention They Are Injured</p><ul>";

String mapCoordinates = "";

int family[3] = { 0, 0, 0 };
// {0-SNAHAL, 1-Sagar, 2-Somya}

// ############################# EDIT HERE FOR PASSWORD AND SSID OF FAMILY MEMBER ##########################
void member1() {
  if (connectWiFi("noway", "123456789", "SNAHAL")) family[0] = 1;
}

void member2() {
  if (connectWiFi("Sagar", "123456789", "Sagar")) family[1] = 1;
}

void member3() {
  if (connectWiFi("Redmi", "123456789", "Somya")) family[2] = 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// ############################################# DO NOT TOUCH WIFI CHECKING ##################################################
bool connectWiFi(String ssid, String password, String name) {
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to %s", name);
  uint16_t count = 10;

  while (WiFi.status() != WL_CONNECTED && count--) {
    delay(1000);
    Serial.print(".");
  }
  bool isConnected = WiFi.status() == WL_CONNECTED;

  if (isConnected) {
    Serial.printf(" Connected.\n");
    delay(1000);
    if (isPermanentConnection == 0) WiFi.disconnect();
    return 1;
  } else Serial.printf("%s is not present \n", name);
  return 0;
}
// ########################################################################################################


void networkReferesh() {
  WiFi.disconnect();
  delay(1000);
  isPermanentConnection = 1;
  for (uint8_t i = 0; i < 3; i++) {
    if (family[i] == 1) {
      if (i == 0) member1();
      else if (i == 1) member2();
      else member3();
      break;
    }
  }
  delay(1000);
}

//################################# Set time via NTP, as required for x.509 validation #################################
void setClock() {
  Serial.println();
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  for (int i = 0; i < 10; i++) {
    time_t now = time(nullptr);
    if (now > 8 * 3600 * 2) {
      struct tm timeinfo;
      gmtime_r(&now, &timeinfo);
      Serial.println("\nCurrent time: " + String(asctime(&timeinfo)));
      return;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nFailed to sync with NTP server.");
  networkReferesh();
  setClock();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.flush();
  Serial.begin(115200);
  Serial.flush();
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(D3, OUTPUT);
}


void loop() {
  digitalWrite(BUILTIN_LED, LOW);
  digitalWrite(D3, LOW);
  delay(3000);
  // if (Serial.available() > 0) {
  // String command = Serial.readStringUntil('\n');
  // if (command == "start") {
  Serial.println("Starting execution...");
  processFamilyMembers();

  if (family[0] == 0 && family[1] == 0 && family[2] == 0) {
    Serial.println("No Person Detected");
    delay(7000);
    processFamilyMembers();
  }

  delay(5000);
  // Accident Detected
  digitalWrite(BUILTIN_LED, HIGH);
  digitalWrite(D3, HIGH);

  getCurrentLocation();

  // digitalWrite(D3, LOW);
  // delay(500);
  // digitalWrite(D3, HIGH);

  sendtoSpreadSheet();

  digitalWrite(D3, LOW);
  delay(500);
  digitalWrite(D3, HIGH);

  sendMailtoMember();
  // }
  // }
  digitalWrite(BUILTIN_LED, LOW);
  digitalWrite(D3, LOW);
  return;
}




void processFamilyMembers() {
  member1(), member2(), member3();
  // String emergencyURL = "https://google.com/";
  Serial.println();
  if (family[0]) {
    Serial.println("HELLO SNAHAL");
    textMsg += "<li><b>Name:</b>SNAHAL KUMAR</li>";
    textMsg += "<li><b>Age:</b>21</li>";
    textMsg += "<li><b>Gender:</b>Male</li>";
    textMsg += "<li><b>Contact Number:</b>7984202797</li>";
    textMsg += "<li><b>Medical History ID: </b> <a href=http://13.233.246.42/sk798/>Check Medical History</a></li>";
    textMsg += "</ul>";
  }
  if (family[1]) {
    Serial.println("HELLO Sagar");
    textMsg += "<ul>";
    textMsg += "<li><b>Name: </b>Sagar Jha</li>";
    textMsg += "<li><b>Age: </b>21</li>";
    textMsg += "<li><b>Gender: </b>Male</li>";
    textMsg += "<li><b>Contact Number: </b>9871984322</li>";
    textMsg += "<li><b>Medical History ID: </b> <a href=http://13.233.246.42/scj798/>Check Medical History</a></li>";
    textMsg += "</ul>";
  }
  if (family[2]) {
    Serial.println("HELLO Somya");
    textMsg += "<ul>";
    textMsg += "<li><b>Name:</b>Somya Sinha</li>";
    textMsg += "<li><b>Age: </b>21</li>";
    textMsg += "<li><b>Gender: </b>Female</li>";
    textMsg += "<li><b>Contact Number: </b>9173932572</li>";
    textMsg += "<li><b>Medical History ID: </b> <a href=http://13.233.246.42/ss798/>Check Medical History</a></li>";
    textMsg += "</ul>";
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//################################################################  GET LOCATION /////////////////////////////////////////
void getCurrentLocation() {
  const char* googleApiKey = "AIzaSyBgAC1y6dEPcJkNZJSjy_0Ec2KHEAC9NCs";  // Google API key
  WifiLocation location(googleApiKey);

  for (uint8_t stopCheckCount = 3; stopCheckCount > 0; stopCheckCount--) {
    Serial.println();
    Serial.println("Reconnecting the Internet");

    networkReferesh();
    setClock();

    location_t loc = location.getGeoFromWiFi();

    Serial.println("Location request data");
    Serial.println(location.getSurroundingWiFiJson() + "\n");
    Serial.println("Location: " + String(loc.lat, 7) + "," + String(loc.lon, 7));
    Serial.println();
    mapCoordinates = String(loc.lat, 7) + "," + String(loc.lon, 7);


    if (mapCoordinates != "0.0000000,0.0000000") {

      String directions = "https://docs.google.com/spreadsheets/d/1Ha6ap3USqMDXS67iOrA7RBHCeQkxMkzW3ywBVGEC0F0/edit?usp=sharing";

      textMsg += "<p>View Accident Location & Directions to Nearby Hospitals (Hold on name to see note with contact details): <a href=\"" + directions + "\">Google Sheets</a></p><br>";
      textMsg += "<p> <span style=\"font-size: 8px; color: #888888;\"> MADE WITH LOVE BY SNAHAL KUMAR </span> </p><br>";
      return;
    }
  }

  Serial.println("Problem Getting Location Kindly Check the issue\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void sendtoSpreadSheet() {
  String GOOGLE_SCRIPT_ID = "AKfycbywRorkO0VtGBZ-b94wPWXT6Hm4wo1hQL8rX6azT1006V3Ctt34EbXywxVxDViw-y8Z";  // to get Contact

  // Serial.println("Reconnecting the Internet");
  networkReferesh();
  Serial.println();

  Serial.println("Reading the hospital details!");
  client.setInsecure();
  https.setTimeout(30000);  // Adjust the timeout value as needed
  uint8_t maxRetryAttempts = 3;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?value=" + mapCoordinates;
  for (uint8_t attempt = 0; attempt < maxRetryAttempts; attempt++) {
    https.begin(client, url);
    https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode = https.GET();
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK) {
      // Request was successful
      String payload = https.getString();
      parsePayload(payload);
      // delay(1000);
      Serial.println("All Details is Uploaded to Google Sheets!\n");
      client.stop();
      https.end();
      break;  // Exit the retry loop on success
    } else {
      Serial.println("HTTP request failed");
      String errorPayload = https.getString();
      Serial.println("Error Message:");
      Serial.println(errorPayload);
      if (attempt < maxRetryAttempts - 1) {
        Serial.println("Retrying...");
        delay(3000);
      } else {
        Serial.println("Max retry attempts reached.");
      }
    }
    https.end();
    networkReferesh();
    client.setInsecure();
    https.setTimeout(30000);  // Adjust the timeout value as needed
  }
}


void parsePayload(String payload) {
  // Split the payload by commas
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

  // Print the email values
  // Serial.println("Email 1: " + Hospital_EMAILS[0]);
  // Serial.println("Email 2: " + Hospital_EMAILS[1]);
  // Serial.println("Email 3: " + Hospital_EMAILS[2]);
}



void sendMailtoMember() {
  bool flagError = 0;
  uint8_t stopCheckCount = 3;
  do {
    if (stopCheckCount == 0) {
      Serial.println("Problem Sending E-Mail Kindly Check the issue\n");
      return;
    }
    // Serial.println("Reconnecting the Internet");
    networkReferesh();
    Serial.println();
    MailClient.networkReconnect(true);

    smtp.debug(0);  // 0/1

    /* Set the callback function to get the sending results */
    // smtp.callback(smtpCallback);

    /* Declare the Session_Config for user defined session credentials */
    Session_Config config;

    /* Set the session config */
    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;
    config.login.user_domain = F("127.0.0.1");

    /* Set the NTP config time */
    config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    config.time.gmt_offset = 3;
    config.time.day_light_offset = 0;

    SMTP_Message message;

    /* ###############################################################################################################           Set the email message contents */
    message.sender.name = F("NEED ATTENTION");
    message.sender.email = AUTHOR_EMAIL;

    message.subject = F("YOUR FAMILY NEEDS URGENT AMBULANCE");

    // String RECIPIENT_EMAIL = "20bec081@smvdu.ac.in";
    message.addRecipient(F("Snahal Kumar"), "snahal135@gmail.com");
    message.addRecipient(F("Sagar Chandra Jha"), "pk529852@gmail.com");
    message.addRecipient(F("Somya Sinha"), "20bec082@smvdu.ac.in");
    //  message.addRecipient(F("Somya Sinha"), "somyamgr28@gmail.com");

    for (uint8_t i = 0; i < 3; i++) {
      if (Hospital_EMAILS[i] == "null") break;
      message.addRecipient("Hospital", Hospital_EMAILS[i]);
    }

    message.html.content = textMsg;
    message.html.charSet = F("us-ascii");

    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

    message.addHeader(F("Message-ID: snahal135@gmail.com"));
    // ############################################################################################################################################################

    /* Connect to the server */
    if (!smtp.connect(&config)) {
      MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      flagError = 1;
      stopCheckCount--;
      continue;
    }
    // if (!smtp.isLoggedIn()) {
    //   Serial.println("Not yet logged in.");
    // } else {
    //   if (smtp.isAuthenticated())
    //     Serial.println("Successfully logged in.");
    //   else
    //     Serial.println("Connected with no Auth.");
    // }

    /* Start sending Email and close the session */
    if (!MailClient.sendMail(&smtp, &message)) {
      MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      flagError = 1;
    }
    // to clear sending result log
    // Serial.println(status.info());
    smtp.sendingResult.clear();
    WiFi.disconnect();
    Serial.println("Email Sent\n");

    Serial.println("Internet Disconnected Thank You For Using Us, We Wish Good Luck For Your Family!");
    Serial.println("Everything Will be Fine Soon :)");
    delay(1000);
    stopCheckCount--;
  } while (flagError);
}



/* ################################ Callback function to get the Email sending status ########################### */
// void smtpCallback(SMTP_Status status) {
//   Serial.println(status.info());

//   if (status.success()) {
//     Serial.println("----------------");
//     MailClient.printf("Message sent success: %d\n", status.completedCount());
//     MailClient.printf("Message sent failed: %d\n", status.failedCount());
//     Serial.println("----------------\n");

//     for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
//       SMTP_Result result = smtp.sendingResult.getItem(i);
//       MailClient.printf("Message No: %d\n", i + 1);
//       MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
//       MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
//       MailClient.printf("Recipient: %s\n", result.recipients.c_str());
//       MailClient.printf("Subject: %s\n", result.subject.c_str());
//     }
//     Serial.println("----------------\n");
//     smtp.sendingResult.clear();
//   }
// }