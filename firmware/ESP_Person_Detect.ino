#include <ESP8266WiFi.h> // For ESP8266

bool family[3] = {0,0,0};
// {0-SNAHAL, 1-PAPA, 2-MOMA}

void setup() {
  Serial.flush();
  Serial.begin(115200);
  Serial.flush();
  delay(1000);
 
}

void loop() {
  if (Serial.available() > 0) {
    // Read the incoming data
    String command = Serial.readStringUntil('\n'); // Read until newline character

    // Check if the received command is "start"
    if (command == "start") {
      
      Serial.println("Starting execution...");
      snahal(); papa(); moma();
      if(family[0]) Serial.println("HELLO SNAHAL");
      if(family[1]) Serial.println("HELLO PAPA");
      if(family[2]) Serial.println("HELLO MOMA");
      
    } else {
      // Handle unknown commands or do nothing
      Serial.println("Unknown command: " + command);

    }
  }
}

// ############################# EDIT HERE FOR PASSWORD AND SSID OF FAMILY MEMBER ##########################
void snahal(){
  if(connectWiFi("REPLACE_WITH_YOUR_WIFI_SSID", "REPLACE_WITH_YOUR_WIFI_PASSWORD", "SNAHAL")) family[0] = 1;
}

void papa(){
  if(connectWiFi("SNAHAL", "REPLACE_WITH_YOUR_WIFI_PASSWORD", "PAPA")) family[1] = 1;
}

void moma(){
  if(connectWiFi("MOMA", "REPLACE_WITH_YOUR_WIFI_PASSWORD", "MOMA")) family[2] = 1;
}
// ########################################################################################################

// ############################ DO NOT TOUCH WIFI CHECKING ##################################################
bool connectWiFi(String ssid, String password, String name){
  WiFi.begin(ssid,password);
  Serial.printf("Connecting to %s ...\n", name);
  uint8_t count = 10;
  
  while (WiFi.status() != WL_CONNECTED && count--) {
    delay(1000);
    Serial.println("Connecting...");
  }
  bool isConnected = WiFi.status() == WL_CONNECTED;

  if(isConnected){
    Serial.printf("Connected to %s\n", name);
    delay(1000);
    WiFi.disconnect();
    delay(1000);
    Serial.println("Disconnected");
    return 1;
  }
  else Serial.printf("%s is not present \n", name);
  return 0;
}
// ########################################################################################################
