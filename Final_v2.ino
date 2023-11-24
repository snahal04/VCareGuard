/* FlowChart 
   1. Person Detection (At Each Start of car)
   2. Accident Detected
   3. Find Current Location
   4. Send Map link to Google Sheet 1 {Date , Time, Map URL}
   5. Find Nearby Hospitals In the Radius of 5Km #
   6. Extract the Contacts of atmost 5 nearest Hospitals from Google Sheet 2 {Email, PhoneNumber, AlternateNumber} #
   7. Send an Alert Email to a Family member who is not present in car and Hospitals.
   THE END
*/

// THis is version 2
// update -> No need for nearbyhospital search
// it will be done by spread sheet when it send current location
// to spread sheet then the return result will be 3 nearby hospital
// sendtoSpreadSheet() code updated and findNearByHospital(); removed
// update 2 -> removed searchInSheets(); its task will be done by sendtoSpreadSheet()


#include <ESP8266WiFi.h>       // To Connect WiFi
#include <ESP_Mail_Client.h>   // For Email
#include <WifiLocation.h>      // GeoLocation
#include <WiFiClientSecure.h>  // For Spreadsheet
// #include <ArduinoJson.h>       // Include the ArduinoJSON library
#include <ESP8266HTTPClient.h>


/* SMTP HoST */
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587
/* The log in credentials */
#define AUTHOR_EMAIL "esp.snahal@gmail.com"
#define AUTHOR_PASSWORD "vtfuukhtfbjsykxx"



/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;
void smtpCallback(SMTP_Status status); /* Callback function to get the Email sending status */

String Hospital_NAME[3] = { "null", "null", "null" };
String RECIPIENT_Hospital_EMAILS[3] = { "null", "null", "null" };
String Hospital_NUMBER[3] = { "null", "null", "null" };
String Hospital_ALT_NUMBER[3] = { "null", "null", "null" };

bool isPermanentConnection = 0;
String textMsg = "<p>An Accident Needs Your Attention They Are Injured</p><ul>";

String mapCoordinates = "";
// String hospitalNames[3] = { "00", "00", "00" };

int family[3] = { 0, 0, 0 };
// {0-SNAHAL, 1-Sagar, 2-Somya}

// ############################# EDIT HERE FOR PASSWORD AND SSID OF FAMILY MEMBER ##########################
void member1() {
  if (connectWiFi("SMVDU.", "123456789", "SNAHAL")) family[0] = 1;
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
  delay(1000);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(D3, OUTPUT);
}


void loop() {
  digitalWrite(BUILTIN_LED, LOW);
  digitalWrite(D3, LOW);
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command == "start") {
      digitalWrite(BUILTIN_LED, HIGH);
      Serial.println("Starting execution...");

      processFamilyMembers();
      if (family[0] == 0 && family[1] == 0 && family[2] == 0) {
        Serial.println("No Person Detected");
        delay(7000);
        processFamilyMembers();
      }
      delay(5000);
      // Accident Detected

      getCurrentLocation();

      sendtoSpreadSheet();
      delay(1000);
      sendMailtoMember();
    }
  }
}




void processFamilyMembers() {
  member1(), member2(), member3();

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
  const char* googleApiKey = "AIzaSyBgAC1y6dEPcJkNZJSjy_0Ec2KHEAC9NCs";  // Google API key
  WifiLocation location(googleApiKey);

  for (uint8_t stopCheckCount = 3; stopCheckCount > 0; stopCheckCount--) {
    Serial.println("Reconnecting the Internet");
    networkReferesh();
    setClock();

    location_t loc = location.getGeoFromWiFi();

    Serial.println("Location request data");
    Serial.println(location.getSurroundingWiFiJson() + "\n");
    Serial.println("Location: " + String(loc.lat, 7) + "," + String(loc.lon, 7));

    // String latitude = String(loc.lat, 7);
    // String longitude = String(loc.lon, 7);
    mapCoordinates = String(loc.lat, 7) + "," + String(loc.lon, 7);


    if (mapCoordinates != "0.0000000,0.0000000") {

      String directions = "https://docs.google.com/spreadsheets/d/1Ha6ap3USqMDXS67iOrA7RBHCeQkxMkzW3ywBVGEC0F0/edit?usp=sharing";

      textMsg += "<p>View Accident Location & Directions to Nearby Hospitals: <a href=\"" + directions + "\">Google Sheets</a></p>";
      return;
    }
  }

  Serial.println("Problem Getting Location Kindly Check the issue\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void sendtoSpreadSheet() {
  // String latitude = "32.9444427";
  // String longitude = "74.9522781";
  // String map = latitude + "," + longitude;
  // GOOGLE_SCRIPT_ID = "AKfycbxXXz0YmHEd6W_7TpkaFpgdPyq5vLOp2DPDKEa0aYMDB6OFNTedqzFL-Cp2WU1ckGxS"; // to get hospital names
  String GOOGLE_SCRIPT_ID = "AKfycbxtD7IyHJJfNu6iEg8QkgsGqkoTln5YCpvofQgbKAmTI70n7Wteuhfj8nzXnPtMhW3H";  // to get Contact
  Serial.println("Reconnecting the Internet");
  networkReferesh();
  delay(1000);
  WiFiClientSecure client;
  HTTPClient https;
  client.setInsecure();

  // String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?value=" + mapCoordinates;
  // Serial.println(url);

  https.setTimeout(15000);  // Adjust the timeout value as needed
  https.begin(client, "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?value=" + mapCoordinates);
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
    client.stop();
    delay(3000);
    sendtoSpreadSheet();
    // return;
  }

  String payload = https.getString();
  // textMsg += "<p> Hospital Contact Details <br>" + payload + "</p>";
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
  parsePayload(payload);
  // Serial.println(payload);
  // https.end();
  textMsg += "<p> <span style=\"color: #013220;\"> TOP 3 NEARBY HOSPITALS CONTACTS </span> </p><br><br>";
  textMsg += "<p> Hospital Name: " + Hospital_NAME[0] + "</p><br>";
  textMsg += "<p> Hospital Email: " + RECIPIENT_Hospital_EMAILS[0] + "</p><br>";
  textMsg += "<p> Phone Number: " + Hospital_NUMBER[0] + "</p><br>";
  textMsg += "<p> Alternate Phone Number: " + Hospital_ALT_NUMBER[0] + "</p><br><br>";

  textMsg += "<p> Hospital Name: " + Hospital_NAME[1] + "</p><br>";
  textMsg += "<p> Hospital Email: " + RECIPIENT_Hospital_EMAILS[1] + "</p><br>";
  textMsg += "<p> Phone Number: " + Hospital_NUMBER[1] + "</p><br>";
  textMsg += "<p> Alternate Phone Number: " + Hospital_ALT_NUMBER[1] + "</p><br><br>";

  textMsg += "<p> Hospital Name: " + Hospital_NAME[2] + "</p><br>";
  textMsg += "<p> Hospital Email: " + RECIPIENT_Hospital_EMAILS[2] + "</p><br>";
  textMsg += "<p> Phone Number: " + Hospital_NUMBER[2] + "</p><br>";
  textMsg += "<p> Alternate Phone Number: " + Hospital_ALT_NUMBER[2] + "</p><br><br>";

  textMsg += "<p> <span style=\"font-size: 8px; color: #888888;\"> MADE WITH LOVE BY SNAHAL KUMAR </span> </p><br>";

}


void parsePayload(String payload) {
  // Split the payload by line breaks
  String lines[12];
  for (int i = 0; i < 12; i++) {
    int newlinePos = payload.indexOf('\n');
    if (newlinePos != -1) {
      lines[i] = payload.substring(0, newlinePos);
      payload = payload.substring(newlinePos + 1);
    } else {
      lines[i] = payload;  // Last line
    }
  }


  Hospital_NAME[0] = lines[0].substring(15);
  Hospital_NAME[0] = lines[5].substring(15);
  Hospital_NAME[0] = lines[10].substring(15);


  RECIPIENT_Hospital_EMAILS[0] = lines[1].substring(7);
  RECIPIENT_Hospital_EMAILS[1] = lines[6].substring(7); 
  RECIPIENT_Hospital_EMAILS[2] = lines[11].substring(7); 

  Hospital_NUMBER[0] = lines[2].substring(14);
  Hospital_NUMBER[0] = lines[7].substring(14);
  Hospital_NUMBER[0] = lines[12].substring(14);


  Hospital_ALT_NUMBER[0] = lines[3].substring(24);
  Hospital_ALT_NUMBER[0] = lines[8].substring(24);
  Hospital_ALT_NUMBER[0] = lines[13].substring(24);


  // Print the email values
  Serial.println("Email 1: " + RECIPIENT_Hospital_EMAILS[0]);
  Serial.println("Email 2: " + RECIPIENT_Hospital_EMAILS[1]);
  Serial.println("Email 3: " + RECIPIENT_Hospital_EMAILS[2]);
}


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

    // String RECIPIENT_EMAIL = "20bec081@smvdu.ac.in";
    if (!family[0]) message.addRecipient(F("Snahal Kumar"), "snahal135@gmail.com");
    else if (!family[1]) message.addRecipient(F("Sagar Chandra Jha"), "pk529852@gmail.com");
    else if (!family[2]) message.addRecipient(F("Somya Sinha"), "20bec082@smvdu.ac.in");
    else message.addRecipient(F("Somya Sinha"), "somyamgr28@gmail.com");

    for (uint8_t i = 0; i < 3; i++) {
      if (RECIPIENT_Hospital_EMAILS[i] == "null") break;
      message.addRecipient(("Hospital" + String(i + 1)), RECIPIENT_Hospital_EMAILS[i]);
    }
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




///////////////////////////////////  Removed Code after updated version  //////////////////////////////////////////////





// Root certificate for valid till 2028S
// const char IRG_Root_X1[] PROGMEM = R"EOF(
// -----BEGIN CERTIFICATE-----
// MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG
// A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv
// b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw
// MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i
// YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT
// aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ
// jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp
// xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp
// 1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG
// snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ
// U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8
// 9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E
// BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B
// AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz
// yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE
// 38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP
// AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad
// DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME
// HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==
// -----END CERTIFICATE-----
// )EOF";



//################################################ SEND MAP LINK TO SPREADSHEET#######################################
// void sendtoSpreadSheet() {
//   delay(1000);
//   Serial.println("Reconnecting the Internet");
//   networkReferesh();
//   const unsigned int httpsPort = 443;
//   String GAS_ID = "AKfycbz3uwSmHFNBrUyE0Xo9rJPGIKfERlqiqSKAOEmHdTsQD1ohtL1a0WKNjjjxB9hCBwbh";  // Spreadsheet script ID

//   const char* host = "script.google.com";
//   WiFiClientSecure client;  // Create a WiFiClientSecure object
//   client.setInsecure();

//   Serial.println("======= Ready To Send Location in SpreadSheet ======");
//   Serial.print("connecting to ");
//   Serial.println(host);

//   if (!client.connect(host, httpsPort)) {
//     Serial.println("Connection failed");
//   }

//   delay(1000);

//   String url = "/macros/s/" + GAS_ID + "/exec?value=" + mapCoordinates;
//   Serial.print("Requesting URL: ");
//   Serial.println(url);

//   client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");
//   client.stop();
//   Serial.println("Request sent");
// }

// void findNearByHospital() {

//   X509List cert(IRG_Root_X1);
//   const char* googleApiKey = "AIzaSyBgAC1y6dEPcJkNZJSjy_0Ec2KHEAC9NCs";
//   Serial.println("Reconnecting the Internet");
//   networkReferesh();
//   setClock();

//   WiFiClientSecure client;

//   client.setTrustAnchors(&cert);

//   HTTPClient https;
//   https.useHTTP10(true);
//   Serial.print("[HTTPS] begin...\n");
//   if (https.begin(client, "https://maps.googleapis.com/maps/api/place/nearbysearch/json?location=" + mapCoordinates + "&radius=3000&type=hospital&key=" + googleApiKey)) {  // HTTPS

//     Serial.print("[HTTPS] GET...\n");
//     int httpCode = https.GET();
//     if (httpCode > 0) {
//       Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

//       // file found at server
//       if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
//         // String payload = https.getString();
//         // Serial.println(payload);

//         DynamicJsonDocument doc(3072);
//         deserializeJson(doc, https.getStream());

//         client.stop();
//         https.end();
//         WiFi.disconnect();

//         uint8_t resultCount = 0;
//         const JsonArray results = doc["results"];
//         for (const JsonObject result : results) {
//           if (resultCount == 3) break;
//           String name = result["name"];
//           // int user_rating = result["user_ratings_total"]; // Not needed
//           // Serial.print("Place Name: ");
//           // Serial.println(name);
//           // const double placeLat = result["geometry"]["location"]["lat"];
//           // const double placeLng = result["geometry"]["location"]["lng"];
//           // double distance = calculateDistance(currentLat, currentLng, placeLat, placeLng);
//           Serial.print("Place Name: ");
//           Serial.println(name);

//           String encodedName = urlEncode(name);

//           hospitalNames[resultCount] = encodedName;
//           // Serial.print("Distance: ");
//           // Serial.print(distance, 6);  // Print distance with 6 decimal places
//           // Serial.println(" km");
//           resultCount++;
//         }
//       }
//     } else {
//       Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
//     }
//   } else {
//     Serial.printf("[HTTPS] Unable to connect\n");
//   }
// }

// // Haversine formula
// double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
//   double R = 6371.0;  // Radius of the Earth in kilometers
//   double dLat = radians(lat2 - lat1);
//   double dLon = radians(lon2 - lon1);
//   double a = sin(dLat / 2) * sin(dLat / 2) + cos(radians(lat1)) * cos(radians(lat2)) * sin(dLon / 2) * sin(dLon / 2);
//   double c = 2 * atan2(sqrt(a), sqrt(1 - a));
//   double distance = R * c;
//   return distance;
// }

// void searchInSheets() {
//   const char* delimiter = ",";  // Choose a suitable delimiter
//   String combinedHospitalNames = hospitalNames[0] + delimiter + hospitalNames[1] + delimiter + hospitalNames[2];

//   String GOOGLE_SCRIPT_ID = "AKfycbwaYGkJE0O72P0cg2snxOVV1gHpTio0xFbkoKKMlBM6ABoA8vHDPJ38WA8d2GzduOtphg";  //--> spreadsheet script ID
//   networkReferesh();
//   WiFiClientSecure client;
//   HTTPClient https;
//   client.setInsecure();
//   // String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?values=" + combinedHospitalNames;

//   // Serial.println(url);
//   https.begin(client, ("https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?values=" + combinedHospitalNames));
//   https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

//   int httpCode = https.GET();
//   Serial.print("HTTP Status Code: ");
//   Serial.println(httpCode);
//   // //-----------------------------------------------------------------------------------
//   if (httpCode <= 0) {
//     Serial.println("Error on HTTP request");
//     https.end();
//     client.stop();
//     delay(3000);
//     searchInSheets();
//   }
//   https.end();
//   client.stop();
//   //-----------------------------------------------------------------------------------
//   //reading data comming from Google Sheet
//   String payload = https.getString();
//   Serial.println(payload);
//   // parsePayload(payload);
// }


// String urlEncode(String str) {
//   String encodedString = "";
//   char c;
//   for (int i = 0; i < str.length(); i++) {
//     c = str.charAt(i);
//     if (isAlphaNumeric(c) || c == '-' || c == '.' || c == '_' || c == '~') {
//       encodedString += c;
//     } else {
//       // Encode special characters as %XX where XX is the hexadecimal representation of the character
//       char hex[3];
//       sprintf(hex, "%02X", c);
//       encodedString += "%" + String(hex);
//     }
//   }
//   return encodedString;
// }
//==============================================================================



// function doGet(e) {
//   Logger.log(JSON.stringify(e));  // view parameters
//   var result = 'Ok'; // assume success

//   // Check if the 'value' parameter exists in the query parameters
//   if (!e.parameter.value) {
//     result = 'No Parameter named "value"';
//   } else {
//     var sheet_id = '1Ha6ap3USqMDXS67iOrA7RBHCeQkxMkzW3ywBVGEC0F0'; // Spreadsheet ID
//     var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet(); // get Active sheet
//     var newRow = sheet.getLastRow() + 1;
//     var d = new Date();

//     // Timestamp in column A
//     var timestamp = d.toLocaleTimeString();

//     // Value from the 'value' parameter
//     var coordinates = stripQuotes(e.parameter.value);
//     var mapLink = 'https://www.google.com/maps/search/?api=1&query=' + coordinates;
//     var mapLinkText = 'View on Map';

//     // Create a hyperlink formula
//     var hyperlinkFormula = '=HYPERLINK("' + mapLink + '","' + mapLinkText + '")';

//     // Write data to the specific columns
//     sheet.getRange(newRow, 1).setValue(d);            // Timestamp in column A
//     sheet.getRange(newRow, 2).setValue(timestamp);    // Timestamp in column B
//     sheet.getRange(newRow, 3).setFormula(hyperlinkFormula);  // Hyperlink formula in column C

//     var nearbyHospitals = findNearbyHospitals(coordinates);

//     // var hospitals = nearbyHospitals.split(', ');

//     // Write the 3 nearest hospital names and create direction links
//     if (nearbyHospitals.length >= 3) {
//       for (var i = 0; i < 3; i++) {
//         // var hospitalName = hospitals[i];
//         // var directionLink = 'https://www.google.com/maps/dir/' + coordinates + '/' + getCoordinatesForHospital(hospitalName);
//         // var directionLinkText = 'Get Directions to ' + hospitalName;
//         // var directionHyperlinkFormula = '=HYPERLINK("' + directionLink + '","' + directionLinkText + '")';
//         var hospitalName = nearbyHospitals[i].name;
//         var hospitalLocation = nearbyHospitals[i].location;
//         var directionLink = 'https://www.google.com/maps/dir/' + coordinates + '/' + hospitalLocation.lat + ',' + hospitalLocation.lng;
//         var directionLinkText = hospitalName;
//         var directionHyperlinkFormula = '=HYPERLINK("' + directionLink + '","' + directionLinkText + '")';
//         sheet.getRange(newRow, 4 + i).setFormula(directionHyperlinkFormula);  // Hyperlink formula for Directions in columns D, E, F
//       }
//     }

//     // result = 'Nearby Hospitals: ' + nearbyHospitals;
//     result = 'Nearby Hospitals: ' + nearbyHospitals.map(function(hospital) { return hospital.name; }).join(', ');
//   }

//   // Return result of operation

//   return ContentService.createTextOutput(result);
// }

// function stripQuotes(value) {
//   return value.replace(/^["']|['"]$/g, "");
// }

// function findNearbyHospitals(coordinates) {
//   // Your Google Places API request
//   var apiKey = 'AIzaSyBgAC1y6dEPcJkNZJSjy_0Ec2KHEAC9NCs';
//   var radius = 5000; // Define your preferred search radius in meters
//   var url = 'https://maps.googleapis.com/maps/api/place/nearbysearch/json?location=' + coordinates + '&radius=' + radius + '&type=hospital&keyword=emergency&key=' + apiKey;

//   var response = UrlFetchApp.fetch(url);
//   var data = JSON.parse(response.getContentText());

//   if (data.status === 'OK' && data.results.length > 0 ) {
//     // data.results.sort(function(a, b) {
//     //   return a.geometry.location.lat - b.geometry.location.lat +
//     //          a.geometry.location.lng - b.geometry.location.lng;
//     // });

//     // Sort the hospitals by user_ratings_total in descending order
//     data.results.sort(function(a, b) {
//       return b.user_ratings_total - a.user_ratings_total;
//     });

//     var hospitals = data.results.slice(0, 3).map(function(hospital) {
//       return {
//         name: hospital.name,
//         location: hospital.geometry.location
//       };
//     });
//     return hospitals;
//   } else {
//     return 'Error: Unable to find nearby hospitals';
//   }
// }
