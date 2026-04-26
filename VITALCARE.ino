#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30100_PulseOximeter.h"

#define ONE_WIRE_BUS 4
#define REPORTING_PERIOD_MS 1000
#define FILTER_SIZE 10
#define SAMPLE_INTERVAL 800
#define MEASURE_WINDOW 100000
#define CONVERSION_DELAY_MS 800

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);
PulseOximeter pox;

// --- DS18B20 state ---
unsigned long startTime = 0;
unsigned long lastRequestTime = 0;
bool conversionRequested = false;
float currentTemp = 0.0;


// --- MAX30100 state ---
float bpmBuffer[FILTER_SIZE] = {0};
uint8_t bufferIndex = 0;
uint32_t tsLastReport = 0;

// --- Display state ---
enum DisplayMode { SHOW_TEMP, SHOW_SPO2, SHOW_FINAL };
DisplayMode displayMode = SHOW_TEMP;
unsigned long finalDisplayStart = 0;

void onBeatDetected() {
  Serial.println("Beat detected!");
}

float getFilteredBPM(float newValue) {
  bpmBuffer[bufferIndex] = newValue;
  bufferIndex = (bufferIndex + 1) % FILTER_SIZE;

  float sum = 0;
  int count = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    if (bpmBuffer[i] > 40 && bpmBuffer[i] < 180) {
      sum += bpmBuffer[i];
      count++;
    }
  }
  return (count == 0) ? 0 : sum / count;
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  // --- LCD ---
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);
  lcd.clear();

  // --- DS18B20 ---
  sensors.begin();
  sensors.setResolution(12);
  sensors.setWaitForConversion(false);

  // --- MAX30100 ---
  if (!pox.begin()) {
    Serial.println("MAX30100 not detected");
    lcd.print("MAX30100 Error!");
    while (1);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_11MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);

  startTime = millis();
  lastRequestTime = millis();
  sensors.requestTemperatures();
  conversionRequested = true;
}

void loop() {
  unsigned long now = millis();

  pox.update();

  
  if (conversionRequested && (now - lastRequestTime >= CONVERSION_DELAY_MS)) {
    currentTemp = sensors.getTempCByIndex(0);
    conversionRequested = false;

    
    Serial.print("Body Temp : ");
    if (currentTemp == DEVICE_DISCONNECTED_C || currentTemp < -50.0) {
      Serial.println("SENSOR ERROR");
    } else {
      Serial.print(currentTemp, 2);
      Serial.println(" C");
    }
  }

  if (!conversionRequested && (now - lastRequestTime >= SAMPLE_INTERVAL)) {
    sensors.requestTemperatures();
    lastRequestTime = now;
    conversionRequested = true;
  }

  // --- Final display ---
  if (displayMode == SHOW_FINAL) {
    if (now - finalDisplayStart >= 5000) {
      startTime = millis();        
      displayMode = SHOW_TEMP;
      lcd.clear();
    }
    return;
  }

  if (now - startTime >= MEASURE_WINDOW) {
    displayMode = SHOW_FINAL;
    finalDisplayStart = now;
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Body Temp:");   
    lcd.setCursor(0, 1); lcd.print(currentTemp, 2); lcd.print(" C");
    Serial.print("Body Temp = "); Serial.print(currentTemp, 2);       
    Serial.println(" C");
    return;
  }

  // --- MAX30100 reporting ---
  if (now - tsLastReport > REPORTING_PERIOD_MS) {
    float rawBPM = pox.getHeartRate();
    float filteredBPM = getFilteredBPM(rawBPM);
    float spo2 = pox.getSpO2();
    tsLastReport = now;

    Serial.println("======================");
    Serial.print("Body Temp : ");          
    Serial.print(currentTemp, 2);
    Serial.println(" C");

    if (filteredBPM < 40 || spo2 < 80) {
      displayMode = SHOW_TEMP;
      Serial.println("BPM       : --");
      Serial.println("SpO2      : --");
      Serial.println("Place finger properly");
    } else {
      displayMode = SHOW_SPO2;
      Serial.print("BPM       : "); Serial.println(filteredBPM);
      Serial.print("SpO2      : "); Serial.print(spo2); Serial.println(" %");
    }
    Serial.println("======================");
  }

  // --- LCD rendering ---
  if (displayMode == SHOW_TEMP) {
    lcd.setCursor(0, 0);
    lcd.print("Body Temp:      ");        
    lcd.setCursor(0, 1);
    lcd.print(currentTemp, 2);
    lcd.print(" C          ");
  } else if (displayMode == SHOW_SPO2) {
    float filteredBPM = getFilteredBPM(0);
    float spo2 = pox.getSpO2();
    lcd.setCursor(0, 0);
    lcd.print("BPM:  "); lcd.print(filteredBPM, 0); lcd.print("      ");
    lcd.setCursor(0, 1);
    lcd.print("SpO2: "); lcd.print(spo2, 0); lcd.print(" %    ");
  }
}
