#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Change address if LCD is different
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Timing
unsigned long startTime = 0;
unsigned long lastSampleTime = 0;

const unsigned long MEASURE_WINDOW = 100000;   // 1 minute
const unsigned long SAMPLE_INTERVAL = 800;   // Safe for 12-bit

float currentTemp = 0.0;
float maxTemp = -100.0;

void setup() {
  Serial.begin(9600);

  sensors.begin();
  sensors.setResolution(12);   // Highest accuracy
  sensors.setWaitForConversion(true);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("DS18B20 Ready");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);

  lcd.clear();
  startTime = millis();
}

void loop() {
  unsigned long now = millis();

  // Take temperature sample every 800 ms
  if (now - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = now;

    sensors.requestTemperatures();
    currentTemp = sensors.getTempCByIndex(0);

    if (currentTemp != DEVICE_DISCONNECTED_C) {
      if (currentTemp > maxTemp) {
        maxTemp = currentTemp;
      }
    }

    // LCD update (always visible)
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(currentTemp, 2);
    lcd.print(" C   ");

    lcd.setCursor(0, 1);
    lcd.print("Max:  ");
    lcd.print(maxTemp, 2);
    lcd.print(" C   ");

    Serial.print("Temp: ");
    Serial.print(currentTemp, 2);
    Serial.print("  Max: ");
    Serial.println(maxTemp, 2);
  }

  // After 6 seconds, freeze result
  if (now - startTime >= MEASURE_WINDOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Highest Temp:");
    lcd.setCursor(0, 1);
    lcd.print(maxTemp, 2);
    lcd.print(" C");

    Serial.println("--- FINAL ---");
    Serial.print("Highest Temp = ");
    Serial.print(maxTemp, 2);
    Serial.println(" C");

    delay(5000);   // Hold result

    // Reset for next cycle
    maxTemp = -100.0;
    startTime = millis();
    lcd.clear();
  }
}
