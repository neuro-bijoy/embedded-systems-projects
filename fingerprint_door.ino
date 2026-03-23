#include <Adafruit_Fingerprint.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

#define RELAY_PIN       7
#define GREEN_LED       8
#define RED_LED         9
#define ACCESS_DELAY    6000

int prevState = -1;

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("    Welcome");
  lcd.setCursor(0, 1);
  lcd.print("FingerPrint Lock");

  finger.begin(57600);
  delay(5);

  if (!finger.verifyPassword())
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    while (1) { delay(1); }
  }

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  delay(2800);
}

void loop()
{
  placeFinger();
  int val = getFingerPrint();

  if (val != -1)
  {
    okFinger();
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RELAY_PIN, LOW);
    delay(2500);
    okFinger_2();
    delay(ACCESS_DELAY);
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }
  else
  {
    if (prevState == 0)
    {
      prevState = -1;
      nokFinger();
      digitalWrite(RED_LED, HIGH);
      digitalWrite(RELAY_PIN, HIGH);
      delay(2000);
      digitalWrite(RED_LED, LOW);
    }
  }
  delay(50);
}

int getFingerPrint()
{
  int p = finger.getImage();
  if (p == 0)
  {
    prevState = 0;
  }
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  prevState = 1;
  return finger.fingerID;
}

void placeFinger()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger...");
  lcd.setCursor(0, 1);
  lcd.print("LOCK:Locked    ");
}

void nokFinger()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Denied  ");
  lcd.setCursor(0, 1);
  lcd.print("LOCK:Locked    ");
}

void okFinger()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Granted ");
  lcd.setCursor(0, 1);
  lcd.print("LOCK:Unlocked  ");
}

void okFinger_2()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Locking in...");
  lcd.setCursor(0, 1);
  lcd.print(String(ACCESS_DELAY / 1000) + " sec");
}