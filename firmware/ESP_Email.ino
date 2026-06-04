/*
Email Id: REPLACE_WITH_YOUR_GMAIL_ADDRESS
App Password: vtfu ukht fbjs ykxx
SMTP Server: smtp.gmail.com
SMTP username: Complete Gmail address
SMTP password: Your Gmail password
SMTP port (TLS): 587
SMTP port (SSL): 465
SMTP TLS/SSL required: yes
*/
#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>

#define WIFI_SSID "REPLACE_WITH_YOUR_WIFI_SSID"
#define WIFI_PASSWORD "REPLACE_WITH_YOUR_WIFI_PASSWORD"
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587

/* The log in credentials */
#define AUTHOR_EMAIL "REPLACE_WITH_YOUR_GMAIL_ADDRESS"
#define AUTHOR_PASSWORD "REPLACE_WITH_YOUR_GMAIL_APP_PASSWORD"

/* Recipient email address */
// #define RECIPIENT_EMAIL "snahal135@gmail.com"

String RECIPIENT_EMAILS[5] = {
  "snahal135@gmail.com",
  "pk529852@gmail.com", // Add more recipient email addresses as needed
  "20bec081@smvdu.ac.in",
  "00",
  "00"
};

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void setup()
{

  Serial.begin(115200);

  Serial.println();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /*  Set the network reconnection option */
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

  /* ###############################################################################################################           Set the message contents */
  message.sender.name = F("NEED ATTENTION");
  message.sender.email = AUTHOR_EMAIL;

  message.subject = F("YOUR FAMILY NEEDS URGENT AMBULANCE");

  for (uint i = 0; i < 5; i++) {
    if(RECIPIENT_EMAILS[i]=="00")break;
    message.addRecipient(("Recipient " + String(i + 1)), RECIPIENT_EMAILS[i]);
  }
  // message.addRecipient(F("Snahal Kumar"), RECIPIENT_EMAIL);

  String textMsg = "<p>This is the <span style=\"color:#ff0000;\">html text</span> message.</p><p>The message was sent via ESP device.</p>";
  message.html.content = textMsg;
  message.html.charSet = F("us-ascii");

  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  message.addHeader(F("Message-ID: snahal135@gmail.com"));
  // ############################################################################################################################################################

  /* Connect to the server */
  if (!smtp.connect(&config))
  {
    MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }
  if (!smtp.isLoggedIn())
  {
    Serial.println("Not yet logged in.");
  }
  else
  {
    if (smtp.isAuthenticated())
      Serial.println("Successfully logged in.");
    else
      Serial.println("Connected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

  // to clear sending result log
  // smtp.sendingResult.clear();
}

void loop()
{
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    // MailClient.printf used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    MailClient.printf("Message sent success: %d\n", status.completedCount());
    MailClient.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
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
