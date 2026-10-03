#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;
int16_t ax, ay, az, gx, gy, gz;

uint8_t receiverAddress[] = {0xAC, 0x0B, 0xFB, 0xD9, 0x1A, 0x77}; 

// Structure to send
typedef struct struct_message {
  char gesture;
} struct_message;

struct_message msg;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 not connected!");
    while (1);
  }

  // Initialize WiFi in station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(receiverAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);


  if (ax > 10000)       msg.gesture = 'F';  // Forward
  else if (ax < -10000) msg.gesture = 'B';  // Backward
  else if (ay > 10000)  msg.gesture = 'L';  // Left
  else if (ay < -10000) msg.gesture = 'R';  // Right
  else                  msg.gesture = 'S';  // Stop

  esp_now_send(receiverAddress, (uint8_t *)&msg, sizeof(msg));

  Serial.print("Gesture sent: ");
  Serial.println(msg.gesture);

  delay(200); // Adjust for responsiveness
}
