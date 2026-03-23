#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30100_PulseOximeter.h"

#define ONE_WIRE_BUS 5
#define REPORTING_PERIOD_MS 1000
#define FILTER_SIZE 10

// --- DS18B20 ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --- LCD ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- DS18B20 Timing ---
unsigned long startTime = 0;
unsigned long lastSampleTime = 0;

const unsigned long MEASURE_WINDOW = 100000;
const unsigned long SAMPLE_INTERVAL = 800;

float currentTemp = 0.0;
float maxTemp = -100.0;

// --- MAX30100 ---
PulseOximeter pox;

float bpmBuffer[FILTER_SIZE];
uint8_t bufferIndex = 0;
uint32_t tsLastReport = 0;

void onBeatDetected()
{
  Serial.println("Beat detected!");
}

float getFilteredBPM(float newValue)
{
  bpmBuffer[bufferIndex] = newValue;
  bufferIndex++;

  if (bufferIndex >= FILTER_SIZE)
    bufferIndex = 0;

  float sum = 0;
  int count = 0;

  for (int i = 0; i < FILTER_SIZE; i++)
  {
    if (bpmBuffer[i] > 40 && bpmBuffer[i] < 180)
    {
      sum += bpmBuffer[i];
      count++;
    }
  }

  if (count == 0) return 0;

  return sum / count;
}

void setup()
{
  Serial.begin(115200);

  // --- DS18B20 Setup ---
  sensors.begin();
  sensors.setResolution(12);
  sensors.setWaitForConversion(true);

  // --- LCD Setup ---
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("DS18B20 Ready");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);

  lcd.clear();
  startTime = millis();

  // --- MAX30100 Setup ---
  Wire.begin(21, 22);

  Serial.println("MAX30100 Stable Monitor");

  if (!pox.begin())
  {
    Serial.println("MAX30100 not detected");
    while (1);
  }

  Serial.println("Sensor ready");

  pox.setIRLedCurrent(MAX30100_LED_CURR_11MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  unsigned long now = millis();

  // --- DS18B20 Loop ---
  if (now - lastSampleTime >= SAMPLE_INTERVAL)
  {
    lastSampleTime = now;

    sensors.requestTemperatures();
    currentTemp = sensors.getTempCByIndex(0);

    if (currentTemp != DEVICE_DISCONNECTED_C)
    {
      if (currentTemp > maxTemp)
      {
        maxTemp = currentTemp;
      }
    }

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

  if (now - startTime >= MEASURE_WINDOW)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Highest Temp:");
    lcd.setCursor(0, 1);
    lcd.print(maxTemp, 2);
    lcd.print(" C");

    Serial.println("---- FINAL ----");
    Serial.print("Highest Temp = ");
    Serial.print(maxTemp, 2);
    Serial.println(" C");

    delay(5000);

    maxTemp = -100.0;
    startTime = millis();
    lcd.clear();
  }

  // --- MAX30100 Loop ---
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS)
{
  float rawBPM = pox.getHeartRate();
  float filteredBPM = getFilteredBPM(rawBPM);
  float spo2 = pox.getSpO2();

  Serial.println("----------------------");

  if (filteredBPM == 0 || spo2 == 0)
  {
    Serial.println("Place finger properly");

    lcd.setCursor(0, 0);
    lcd.print("BPM: --         ");
    lcd.setCursor(0, 1);
    lcd.print("SpO2: --        ");
  }
  else
  {
    Serial.print("Heart Rate: ");
    Serial.print(filteredBPM);
    Serial.println(" bpm");

    Serial.print("SpO2: ");
    Serial.print(spo2);
    Serial.println(" %");

    lcd.setCursor(0, 0);
    lcd.print("BPM: ");
    lcd.print(filteredBPM, 0);
    lcd.print("       ");

    lcd.setCursor(0, 1);
    lcd.print("SpO2: ");
    lcd.print(spo2, 0);
    lcd.print(" %     ");
  }

  tsLastReport = millis();
}
}
