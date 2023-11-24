#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>
#include <WifiLocation.h>
#include <WiFiClientSecure.h>  // For Spreadsheet

/* SMTP HoST */
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587
/* The log in credentials */
#define AUTHOR_EMAIL "esp.snahal@gmail.com"
#define AUTHOR_PASSWORD "vtfuukhtfbjsykxx"

bool isPermanentConnection = 0;
String textMsg = "<p>Your Family Members Needs Your Attention They Are Injured</p><ul>";
/* Recipient email address */
// #define RECIPIENT_EMAIL "snahal135@gmail.com"

String mapCoordinates = "";

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;
/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

int family[3] = { 0, 0, 0 };
// {0-SNAHAL, 1-Sagar, 2-Somya}

void setup() {
  Serial.flush();
  Serial.begin(115200);
  Serial.flush();
  delay(1000);
}


void loop() {
  if (Serial.available() > 0) {
    // Read the incoming data
    String command = Serial.readStringUntil('\n');  // Read until newline character
    if (command == "start") {
      Serial.println("Starting execution...");
      processFamilyMembers();

      // Accident Detected

      getCurrentLocation();

      sendMailtoMember();

      sendtoSpreadSheet();

    } else {
      Serial.println("Unknown command: " + command);
    }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void networkReferesh() {
  isPermanentConnection = 1;
  for (uint8_t i = 0; i < 3; i++) {
    if (family[i] == 1) {
      if (i == 0) snahal();
      else if (i == 1) papa();
      else moma();
      break;
    }
  }
  delay(1000);
}




//################################################## GET FAMILY MEMBERS PRESENT IN A CAR ################################
void processFamilyMembers() {
  snahal(), papa(), moma();

  if (family[0]) {
    Serial.println("HELLO SNAHAL");
    textMsg += "<li><b>Name:</b>SNAHAL KUMAR</li>";
    textMsg += "<li><b>Age:</b>21</li>";
    textMsg += "<li><b>Gender:</b>Male</li>";
    textMsg += "<li><b>Contact Number:</b>7984202797</li>";
    textMsg += "<li><b>Medical History ID: </b>2133</li>";
    textMsg += "</ul>";
  }
  if (family[1]) {
    Serial.println("HELLO Sagar");
    textMsg += "<ul>";
    textMsg += "<li><b>Name:</b>Sagar Jha</li>";
    textMsg += "<li><b>Age:</b>21</li>";
    textMsg += "<li><b>Gender:</b>Male</li>";
    textMsg += "<li><b>Contact Number:</b>9871984322</li>";
    textMsg += "<li><b>Medical History ID: </b>2134</li>";
    textMsg += "</ul>";
  }
  if (family[2]) {
    Serial.println("HELLO Somya");
    textMsg += "<ul>";
    textMsg += "<li><b>Name:</b>Somya Sinha</li>";
    textMsg += "<li><b>Age:</b>21</li>";
    textMsg += "<li><b>Gender:</b>Female</li>";
    textMsg += "<li><b>Contact Number:</b>9173932572</li>";
    textMsg += "<li><b>Medical History ID: </b>2135</li>";
    textMsg += "</ul>";
  }

  String emergencyURL = "https://google.com/";
  textMsg += "<p>Check Medical History: <a href=\"" + emergencyURL + "\">" + emergencyURL + "</a></p>";
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//################################################################  GET LOCATION /////////////////////////////////////////
void getCurrentLocation() {
  const char* googleApiKey = "AIzaSyBgAC1y6dEPcJkNZJSjy_0Ec2KHEAC9NCs";  // Google API
  WifiLocation location(googleApiKey);
  // Set Clock and Track Location Time to execute 5-8 Seconds
  uint8_t stopCheckCount = 3;
  do {
    if (stopCheckCount == 0) {
      Serial.println("Problem Getting Location Kindly Check the issue\n");
      return;
    }
    Serial.println("Reconnecting the Internet");
    networkReferesh();
    setClock();

    location_t loc = location.getGeoFromWiFi();

    Serial.println("Location request data");
    Serial.println(location.getSurroundingWiFiJson() + "\n");
    Serial.println("Location: " + String(loc.lat, 7) + "," + String(loc.lon, 7));
    //Serial.println("Longitude: " + String(loc.lon, 7));
    Serial.println("Accuracy: " + String(loc.accuracy));
    Serial.println("Result: " + location.wlStatusStr(location.getStatus()));
    // String googleMapsURL = "https://www.google.com/maps/search/?api=1&query=" + String(loc.lat, 7) + "," + String(loc.lon, 7);

    String latitude = String(loc.lat, 7);
    String longitude = String(loc.lon, 7);
    mapCoordinates = latitude + "," + longitude;
    String googleMapsURL = "https://www.google.com/maps/search/?api=1&query=" + mapCoordinates;

    textMsg += "<p>View Location on Google Maps: <a href=\"" + googleMapsURL + "\">Google Maps</a></p>";
    WiFi.disconnect();
    Serial.println("Internet Disconnected");
    delay(1000);

    stopCheckCount--;
  } while (mapCoordinates == "0.0000000,0.0000000");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







// ############################# Send an email to the person who is not in car ##########################
void sendMailtoMember() {
  bool flagError = 0;
  uint8_t stopCheckCount = 3;
  do {
    if (stopCheckCount == 0) {
      Serial.println("Problem Sending E-Mail Kindly Check the issue\n");
      return;
    }
    Serial.println("Reconnecting the Internet");
    networkReferesh();
    MailClient.networkReconnect(true);

    /** Enable the debug via Serial port
          * 0 for no debugging
          * 1 for basic level debugging
          *
          * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
          */
    smtp.debug(0);

    /* Set the callback function to get the sending results */
    smtp.callback(smtpCallback);

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

    /* The full message sending logs can now save to file */
    /* Since v3.0.4, the sent logs stored in smtp.sendingResult will store only the latest message logs */
    // config.sentLogs.filename = "/path/to/log/file";
    // config.sentLogs.storage_type = esp_mail_file_storage_type_flash;

    /* Declare the message class */
    SMTP_Message message;

    /* ###############################################################################################################           Set the email message contents */
    message.sender.name = F("NEED ATTENTION");
    message.sender.email = AUTHOR_EMAIL;

    message.subject = F("YOUR FAMILY NEEDS URGENT AMBULANCE");

    String RECIPIENT_EMAIL = "20bec081@smvdu.ac.in";
    if (!family[0]) message.addRecipient(F("Snahal Kumar"), "snahal135@gmail.com");
    else if (!family[1]) message.addRecipient(F("Sagar Chandra Jha"), "pk529852@gmail.com");
    else if (!family[2]) message.addRecipient(F("Somya Sinha"), "20bec082@smvdu.ac.in");
    else message.addRecipient(F("Somya Sinha"), "somyamgr28@gmail.com");

    // message.addRecipient(F("Snahal Kumar"), RECIPIENT_EMAIL);

    // String textMsg = "<p>This is the <span style=\"color:#ff0000;\">html text</span> message.</p><p>The message was sent via ESP device.</p>";

    // // Print the message to the Serial Monitor (for testing)
    // Serial.println(textMsg);
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
    if (!smtp.isLoggedIn()) {
      Serial.println("Not yet logged in.");
    } else {
      if (smtp.isAuthenticated())
        Serial.println("Successfully logged in.");
      else
        Serial.println("Connected with no Auth.");
    }

    /* Start sending Email and close the session */
    if (!MailClient.sendMail(&smtp, &message)) {
      MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      flagError = 1;
    }
    // to clear sending result log
    // smtp.sendingResult.clear();
    WiFi.disconnect();
    Serial.println("Internet Disconnected");
    delay(1000);

    stopCheckCount--;
  } while (flagError);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







// ############################# EDIT HERE FOR PASSWORD AND SSID OF FAMILY MEMBER ##########################
void snahal() {
  if (connectWiFi("SMVDU.", "123456789", "SNAHAL")) family[0] = 1;
}

void papa() {
  if (connectWiFi("Sagar", "123456789", "Sagar")) family[1] = 1;
}

void moma() {
  if (connectWiFi("Redmi", "123456789", "Somya")) family[2] = 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







// ############################################# DO NOT TOUCH WIFI CHECKING ##################################################
bool connectWiFi(String ssid, String password, String name) {
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to %s ...\n", name);
  uint16_t count = 15;

  while (WiFi.status() != WL_CONNECTED && count--) {
    delay(1000);
    Serial.print(".");
  }
  bool isConnected = WiFi.status() == WL_CONNECTED;

  if (isConnected) {
    Serial.printf("Connected to %s\n", name);
    delay(1000);
    if (isPermanentConnection == 0) {
      WiFi.disconnect();
      delay(1000);
      Serial.println("Disconnected");
    }
    return 1;
  } else Serial.printf("%s is not present \n", name);
  return 0;
}
// ########################################################################################################









/* ################################ Callback function to get the Email sending status ########################### */
void smtpCallback(SMTP_Status status) {
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()) {
    // MailClient.printf used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    MailClient.printf("Message sent success: %d\n", status.completedCount());
    MailClient.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

      MailClient.printf("Message No: %d\n", i + 1);
      MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
      MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      MailClient.printf("Recipient: %s\n", result.recipients.c_str());
      MailClient.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







//################################# Set time via NTP, as required for x.509 validation #################################
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  uint8_t stopCheckCount = 0;
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    if(stopCheckCount == 9) return;
    stopCheckCount++;
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("\n");
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






//################################################ SEND MAP LINK TO SPREADSHEET#######################################
void sendtoSpreadSheet() {
  bool flagError = 0;
  uint8_t stopCheckCount = 3;
  do {
    if (stopCheckCount == 0) {
      Serial.println("Problem Updating SpreadSheet Kindly Check the issue\n");
      return;
    }
    Serial.println("Reconnecting the Internet");
    networkReferesh();
    const char* host = "script.google.com";
    const unsigned int httpsPort = 443;
    WiFiClientSecure client;                                                                     //--> Create a WiFiClientSecure object.
    String GAS_ID = "AKfycbz3uwSmHFNBrUyE0Xo9rJPGIKfERlqiqSKAOEmHdTsQD1ohtL1a0WKNjjjxB9hCBwbh";  //--> spreadsheet script ID
    client.setInsecure();

    Serial.println("======= Ready To Send Location in SpreadSheet ======");
    Serial.print("connecting to ");
    Serial.println(host);

    //----------------------------------------Connect to Google host
    if (!client.connect(host, httpsPort)) {
      Serial.println("connection failed");
      flagError = 1;
      stopCheckCount--;
      continue;
    }
    //----------------------------------------
    delay(1000);
    //----------------------------------------Processing data and sending data
    String url = "/macros/s/" + GAS_ID + "/exec?value=" + mapCoordinates;
    Serial.print("requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");

    Serial.println("request sent");
    //----------------------------------------

    //----------------------------------------Checking whether the data was sent successfully or not
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    String line = client.readStringUntil('\n');
    Serial.print("reply was : ");
    Serial.println(line);
    Serial.println("closing connection");
    Serial.println("==========");
    Serial.println();
    //----------------------------------------
    WiFi.disconnect();
    Serial.println("Internet Disconnected");
    delay(1000);

    stopCheckCount--;
  } while (flagError);
}
