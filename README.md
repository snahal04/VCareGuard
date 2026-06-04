# VCareGuard

VCareGuard is an accident-response and emergency-notification system built around ESP8266-based hardware, Google Sheets, Google Maps, and email alerts. It is designed to help detect an emergency situation, capture the location, find nearby hospitals, and notify family members and hospitals quickly.

## What this project does

- Detects an accident or emergency condition through the connected hardware flow.
- Captures the current GPS/Wi-Fi location of the device.
- Logs details to Google Sheets.
- Finds nearby hospitals and creates direction links.
- Sends alert emails to family members and hospitals.

## Project structure

- firmware/ — Arduino sketch files for the ESP8266 device and experiment versions
  - Final.ino and Final_v2.ino — recommended main firmware versions to start with
  - ESP_*.ino and Mail_*.ino — supporting and experimental variants
- scripts/ — Google Apps Script files used by Sheets/Maps integration
  - Maps_script.js — writes accident data and hospital links into Google Sheets
  - Hospital_dataset.js — returns hospital records for the email and mapping flow
- docs/ — project documentation and reference material
  - Smart Accident Response System.docx — project presentation/reference notes

## Recommended starting point

If you are new to this repository, begin with:

1. firmware/Final.ino
2. firmware/Final_v2.ino
3. scripts/Maps_script.js
4. scripts/Hospital_dataset.js

## Hardware requirements

- NodeMCU/ESP8266 development board
- Emergency detection hardware or sensor setup used in your prototype
- Stable Wi-Fi connection
- Optional USB cable and serial monitor for debugging

## Software requirements

- Arduino IDE
- ESP8266 board package installed in Arduino IDE
- Required libraries for the firmware sketches (for example ESP8266WiFi, ESP_Mail_Client, WifiLocation, ESP8266HTTPClient)
- Google Apps Script deployment for the two JavaScript files

## Quick setup guide

1. Open the firmware sketches in Arduino IDE.
2. Install the required libraries for the ESP8266 build.
3. Update Wi-Fi credentials, email credentials, and the Google script IDs in the firmware code.
4. Deploy the Apps Script files from scripts/.
5. Upload the firmware to the ESP8266 board.
6. Test with the serial monitor and confirm the Sheets/email flow works.

## Important configuration notes

Before using this project in real life:

- Replace all hardcoded passwords, API keys, and personal contact details with your own secure values.
- Use Gmail app passwords or secure secrets management instead of storing credentials directly in code.
- Replace placeholder Google Spreadsheet IDs and Apps Script URLs with your own deployed versions.

## How the system works

1. The ESP8266 device connects to Wi-Fi and starts the emergency flow.
2. The firmware gathers emergency and location data.
3. The location is used to search for nearby hospitals.
4. Results are written to Google Sheets and sent through email alerts.
5. Family members and hospitals receive the relevant information for fast response.

## Notes

This repository contains both main firmware and earlier experimental versions. The final production flow should be built from the clearest working sketch and verified with real hardware before deployment.

## Next improvements

- Add a clean hardware wiring diagram
- Add a deployment checklist for Google Apps Script
- Add a safer configuration file for credentials
- Add a testing and debugging guide
