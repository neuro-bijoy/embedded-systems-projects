#include <TinyGPS++.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);  // UART2

#define GPS_RX_PIN  16
#define GPS_TX_PIN  17
#define GPS_BAUD    9600

void setup() {
  Serial.begin(115200);
  delay(1000);

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  Serial.println("==========================");
  Serial.println("     ESP32 GPS TRACKER    ");
  Serial.println("==========================");
  Serial.println("Waiting for GPS fix...");
}

void loop() {
  // Feed GPS parser for 1 second
  unsigned long start = millis();
  bool newData = false;

  while (millis() - start < 1000) {
    while (gpsSerial.available()) {
      char c = gpsSerial.read();
      if (gps.encode(c))
        newData = true;
    }
  }

  // No bytes received at all = wiring problem
  if (gps.charsProcessed() < 10) {
    Serial.println("  No data from GPS! Check wiring:");
    Serial.println("    GPS TX → ESP32 Pin 16");
    Serial.println("    GPS RX → ESP32 Pin 17");
    return;
  }

  if (newData) {
    Serial.println("-----------------------------");

    // --- Location ---
    if (gps.location.isValid()) {
      Serial.print("Latitude   : ");
      Serial.println(gps.location.lat(), 7);
      Serial.print("Longitude  : ");
      Serial.println(gps.location.lng(), 7);
    } else {
      Serial.println("Location   : Waiting for fix...");
    }

    // --- Altitude ---
    if (gps.altitude.isValid()) {
      Serial.print("Altitude   : ");
      Serial.print(gps.altitude.meters(), 2);
      Serial.println(" m");
    } else {
      Serial.println("Altitude   : Not valid");
    }

    // --- Satellites ---
    if (gps.satellites.isValid()) {
      Serial.print("Satellites : ");
      Serial.println(gps.satellites.value());
    } else {
      Serial.println("Satellites : Not valid");
    }

    // --- Speed ---
    if (gps.speed.isValid()) {
      Serial.print("Speed      : ");
      Serial.print(gps.speed.kmph(), 2);
      Serial.println(" km/h");
    } else {
      Serial.println("Speed      : Not valid");
    }

    // --- Date & Time ---
    if (gps.date.isValid() && gps.time.isValid()) {
      Serial.printf("Date/Time  : %04d-%02d-%02d %02d:%02d:%02d UTC\n",
        gps.date.year(), gps.date.month(),  gps.date.day(),
        gps.time.hour(), gps.time.minute(), gps.time.second());
    } else {
      Serial.println("Date/Time  : Not valid");
    }

    Serial.println("-----------------------------");

  } else {
    Serial.println("Waiting for valid GPS data...");
  }
}