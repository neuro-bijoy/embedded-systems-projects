#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

#define RELAY_PIN 7
#define LED_PIN 8     // Green LED (Access Granted)
#define LED2_PIN 6    // Red LED (Access Denied)

MFRC522 rfid(SS_PIN, RST_PIN);

// 🔐 Replace this UID with your RFID tag UID
byte authorizedUID[4] = {0x33, 0x7F, 0xE7, 0x1B};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH); // Relay OFF (Door locked)
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);

  Serial.println("Place your RFID tag...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  if (checkUID()) {
    Serial.println("Access Granted");
    unlockDoor();
  } else {
    Serial.println("Access Denied");
    denyAccess();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

bool checkUID() {
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != authorizedUID[i]) {
      return false;
    }
  }
  return true;
}

void unlockDoor() {
  digitalWrite(LED_PIN, HIGH);     // Green LED ON
  digitalWrite(LED2_PIN, LOW);     // Red LED OFF
  digitalWrite(RELAY_PIN, LOW);    // Door Unlock

  delay(3000);                     // Door open for 3 seconds

  digitalWrite(RELAY_PIN, HIGH);   // Door Lock again
  digitalWrite(LED_PIN, LOW);      // Green LED OFF
}

void denyAccess() {
  digitalWrite(LED2_PIN, HIGH);    // Red LED ON
  digitalWrite(LED_PIN, LOW);      // Green LED OFF
  digitalWrite(RELAY_PIN, HIGH);   // Ensure door stays locked

  delay(2000);                     // Show denied status for 2 sec
  digitalWrite(LED2_PIN, LOW);     // Red LED OFF
}
